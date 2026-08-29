#!/usr/bin/env bash
set -euo pipefail

# Summit Tale / Priestess C01 development tool
#
# Proven scope:
#   Blender Edit Mode, existing topology
#   MshC01 / MsmC01 / MslC01
#   patch only serialized SMD XYZ
#   safe client backup / push / rollback
#
# Commands:
#   status
#   export
#   push
#   restore-previous
#   restore-original
#   history
#
# Optional environment overrides:
#   PT_BLEND=/path/file.blend
#   PT_CLIENT_ROOT=/path/to/client
#   PT_BUILD_DIR=/path/to/build

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

BLENDER_APP="${PT_BLENDER_APP:-org.blender.Blender}"
BLEND="${PT_BLEND:-$ROOT/MfbC01_direct_smd_v3_static_armature_testv2.blend}"
ORIGINAL_SMD="${PT_ORIGINAL_SMD:-$ROOT/MfbC01.smd}"
SKELETON="${PT_SKELETON:-$ROOT/m5.smb}"
PATCHER="${PT_PATCHER:-$ROOT/pt_blender_patch_smd_vertices_v1.py}"

CLIENT_ROOT="${PT_CLIENT_ROOT:-/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
BUILD_DIR="${PT_BUILD_DIR:-$ROOT/pt_build/C01}"
BACKUP_ROOT="${PT_BACKUP_ROOT:-$ROOT/client_backups/C01}"
HISTORY="$BUILD_DIR/history.tsv"

FINAL_SMD="$BUILD_DIR/MfbC01_current.smd"
FINAL_REPORT="$BUILD_DIR/MfbC01_current_manifest.txt"

EXPECTED_ORIGINAL_SHA256="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
EXPECTED_SIZE="290229"

OBJECTS=("MshC01" "MsmC01" "MslC01")

die() {
  printf 'ERROR: %s\n' "$*" >&2
  exit 1
}

sha256_file() {
  sha256sum -- "$1" | awk '{print $1}'
}

file_size() {
  stat -c %s -- "$1"
}

ensure_dirs() {
  mkdir -p "$BUILD_DIR" "$BACKUP_ROOT"
  if [[ ! -f "$HISTORY" ]]; then
    printf 'timestamp\taction\tsha256\tpath_or_note\n' > "$HISTORY"
  fi
}

validate_original() {
  [[ -f "$ORIGINAL_SMD" ]] || die "Original SMD not found: $ORIGINAL_SMD"
  [[ "$(file_size "$ORIGINAL_SMD")" == "$EXPECTED_SIZE" ]] || \
    die "Original SMD size mismatch."
  [[ "$(sha256_file "$ORIGINAL_SMD")" == "$EXPECTED_ORIGINAL_SHA256" ]] || \
    die "Original SMD SHA256 mismatch. Refusing to continue."
}

validate_smd_basic() {
  local f="$1"
  [[ -f "$f" ]] || die "SMD not found: $f"
  [[ "$(file_size "$f")" == "$EXPECTED_SIZE" ]] || \
    die "Unexpected SMD size $(file_size "$f"); expected $EXPECTED_SIZE: $f"

  # 23-byte literal header, no text parser assumptions.
  local hdr
  hdr="$(head -c 23 -- "$f" || true)"
  [[ "$hdr" == "SMD Model data Ver 0.62" ]] || \
    die "Unexpected SMD header: $f"
}

resolve_client_target() {
  [[ -d "$CLIENT_ROOT" ]] || die "Client root not found: $CLIENT_ROOT"
  mapfile -t _targets < <(find "$CLIENT_ROOT" -type f -iname 'MfbC01.smd' -print)
  if (( ${#_targets[@]} != 1 )); then
    printf 'Found %d MfbC01.smd candidate(s):\n' "${#_targets[@]}" >&2
    printf '  %s\n' "${_targets[@]:-}" >&2
    die "Expected exactly one client MfbC01.smd."
  fi
  CLIENT_TARGET="${_targets[0]}"
}

known_managed_hash() {
  local wanted="$1"

  [[ "$wanted" == "$EXPECTED_ORIGINAL_SHA256" ]] && return 0

  if [[ -f "$HISTORY" ]] && awk -F'\t' -v h="$wanted" 'NR>1 && $3==h {found=1} END{exit !found}' "$HISTORY"; then
    return 0
  fi

  # Import compatibility with the earlier safe installer manifests.
  if [[ -d "$ROOT/client_backups" ]] && \
     grep -Rhs --include='manifest.txt' '^replacement_sha256=' "$ROOT/client_backups" 2>/dev/null \
       | cut -d= -f2- | grep -Fxq -- "$wanted"; then
    return 0
  fi

  return 1
}

record_history() {
  local action="$1" hash="$2" note="$3"
  ensure_dirs
  printf '%s\t%s\t%s\t%s\n' "$(date --iso-8601=seconds)" "$action" "$hash" "$note" >> "$HISTORY"
}

run_blender_patch() {
  local blend="$1" input_smd="$2" object="$3" output_smd="$4" report="$5"

  flatpak run --filesystem=home "$BLENDER_APP" \
    --background "$blend" \
    --python "$PATCHER" -- \
    --smd "$input_smd" \
    --skeleton "$SKELETON" \
    --object "$object" \
    --out "$output_smd" \
    --report "$report"
}

export_lods() {
  ensure_dirs
  validate_original
  [[ -f "$BLEND" ]] || die "Blend not found: $BLEND"
  [[ -f "$SKELETON" ]] || die "Skeleton not found: $SKELETON"
  [[ -f "$PATCHER" ]] || die "Patcher not found: $PATCHER"

  local work
  work="$(mktemp -d "$BUILD_DIR/.export.XXXXXX")"

  local input="$ORIGINAL_SMD"
  local i=0
  local last=""
  local object stage report

  echo "PT_C01_EXPORT"
  echo "blend=$BLEND"
  echo "original_smd=$ORIGINAL_SMD"
  echo

  for object in "${OBJECTS[@]}"; do
    i=$((i + 1))
    stage="$work/$(printf '%02d' "$i")_${object}.smd"
    report="$BUILD_DIR/${object}_report.json"

    echo "===== ${object} ====="
    run_blender_patch "$BLEND" "$input" "$object" "$stage" "$report"
    echo

    input="$stage"
    last="$stage"
  done

  [[ -n "$last" ]] || die "Internal error: no export stage produced."

  validate_smd_basic "$last"

  local out_tmp="$BUILD_DIR/.MfbC01_current.smd.tmp.$$"
  cp -- "$last" "$out_tmp"
  validate_smd_basic "$out_tmp"
  mv -f -- "$out_tmp" "$FINAL_SMD"

  local final_sha original_sha
  final_sha="$(sha256_file "$FINAL_SMD")"
  original_sha="$(sha256_file "$ORIGINAL_SMD")"

  [[ "$final_sha" != "$original_sha" ]] || \
    die "Export is byte-identical to original; no C01 torso edit was detected."

  {
    printf 'generated_at=%s\n' "$(date --iso-8601=seconds)"
    printf 'blend=%s\n' "$BLEND"
    printf 'original_smd=%s\n' "$ORIGINAL_SMD"
    printf 'original_sha256=%s\n' "$original_sha"
    printf 'output=%s\n' "$FINAL_SMD"
    printf 'output_size=%s\n' "$(file_size "$FINAL_SMD")"
    printf 'output_sha256=%s\n' "$final_sha"
    printf 'objects=%s\n' "${OBJECTS[*]}"
  } > "$FINAL_REPORT"

  record_history "export" "$final_sha" "$FINAL_SMD"

  echo "===== EXPORT RESULT ====="
  echo "output=$FINAL_SMD"
  echo "size=$(file_size "$FINAL_SMD")"
  echo "sha256=$final_sha"
  echo "manifest=$FINAL_REPORT"
  echo "EXPORT_CHECK=PASS"

  rm -rf -- "$work"
}

backup_current_client() {
  local label="$1"
  resolve_client_target
  validate_smd_basic "$CLIENT_TARGET"

  local stamp current_sha dir backup
  stamp="$(date '+%Y%m%d_%H%M%S')"
  current_sha="$(sha256_file "$CLIENT_TARGET")"
  dir="$BACKUP_ROOT/${label}_${stamp}"
  backup="$dir/MfbC01.smd"

  mkdir -p "$dir"
  cp -a -- "$CLIENT_TARGET" "$backup"

  [[ "$(sha256_file "$backup")" == "$current_sha" ]] || \
    die "Backup hash verification failed."

  {
    printf 'created_at=%s\n' "$(date --iso-8601=seconds)"
    printf 'client_target=%s\n' "$CLIENT_TARGET"
    printf 'backup=%s\n' "$backup"
    printf 'sha256=%s\n' "$current_sha"
  } > "$dir/manifest.txt"

  LAST_BACKUP_DIR="$dir"
  LAST_BACKUP_FILE="$backup"
  LAST_BACKUP_SHA="$current_sha"
}

push_build() {
  ensure_dirs
  export_lods
  resolve_client_target
  validate_smd_basic "$CLIENT_TARGET"
  validate_smd_basic "$FINAL_SMD"

  local client_sha build_sha
  client_sha="$(sha256_file "$CLIENT_TARGET")"
  build_sha="$(sha256_file "$FINAL_SMD")"

  echo
  echo "PT_C01_PUSH"
  echo "client=$CLIENT_TARGET"
  echo "client_sha256=$client_sha"
  echo "build=$FINAL_SMD"
  echo "build_sha256=$build_sha"

  [[ "$client_sha" != "$build_sha" ]] || \
    die "Client already contains this exact build."

  known_managed_hash "$client_sha" || \
    die "Current client SMD is not pristine and is not a recorded managed build. Refusing to overwrite."

  backup_current_client "before_push"

  local tmp
  tmp="$(dirname "$CLIENT_TARGET")/.MfbC01.smd.pt_c01.tmp.$$"
  cp -- "$FINAL_SMD" "$tmp"
  chmod --reference="$CLIENT_TARGET" "$tmp" 2>/dev/null || true

  [[ "$(sha256_file "$tmp")" == "$build_sha" ]] || {
    rm -f -- "$tmp"
    die "Temporary client copy hash verification failed."
  }

  mv -f -- "$tmp" "$CLIENT_TARGET"

  if [[ "$(sha256_file "$CLIENT_TARGET")" != "$build_sha" ]]; then
    echo "Install verification failed; restoring previous client SMD." >&2
    cp -a -- "$LAST_BACKUP_FILE" "$CLIENT_TARGET"
    die "Push failed; previous version restored."
  fi

  ln -sfn -- "$LAST_BACKUP_DIR" "$BACKUP_ROOT/latest"
  record_history "push" "$build_sha" "$CLIENT_TARGET"

  {
    printf 'replacement=%s\n' "$FINAL_SMD"
    printf 'replacement_sha256=%s\n' "$build_sha"
  } >> "$LAST_BACKUP_DIR/manifest.txt"

  echo
  echo "backup=$LAST_BACKUP_DIR"
  echo "backed_up_sha256=$LAST_BACKUP_SHA"
  echo "installed_sha256=$build_sha"
  echo "PUSH_CHECK=PASS"
}

restore_previous() {
  ensure_dirs
  resolve_client_target

  [[ -L "$BACKUP_ROOT/latest" || -d "$BACKUP_ROOT/latest" ]] || \
    die "No latest C01 backup found."

  local dir backup sha current
  dir="$(readlink -f "$BACKUP_ROOT/latest")"
  backup="$dir/MfbC01.smd"
  [[ -f "$backup" ]] || die "Backup file missing: $backup"

  validate_smd_basic "$backup"
  sha="$(sha256_file "$backup")"
  current="$(sha256_file "$CLIENT_TARGET")"

  # Backup the state we are about to replace as well.
  backup_current_client "before_restore_previous"

  cp -a -- "$backup" "$CLIENT_TARGET"
  [[ "$(sha256_file "$CLIENT_TARGET")" == "$sha" ]] || \
    die "Restore verification failed."

  record_history "restore_previous" "$sha" "$dir"

  echo "restored_sha256=$sha"
  echo "replaced_sha256=$current"
  echo "RESTORE_PREVIOUS_CHECK=PASS"
}

find_pristine_backup() {
  local f
  if [[ -d "$ROOT/client_backups" ]]; then
    while IFS= read -r f; do
      [[ -f "$f" ]] || continue
      if [[ "$(file_size "$f")" == "$EXPECTED_SIZE" ]] && \
         [[ "$(sha256_file "$f")" == "$EXPECTED_ORIGINAL_SHA256" ]]; then
        printf '%s\n' "$f"
        return 0
      fi
    done < <(find "$ROOT/client_backups" -type f -name 'MfbC01.smd' -print | sort)
  fi
  return 1
}

restore_original() {
  ensure_dirs
  resolve_client_target

  local current
  current="$(sha256_file "$CLIENT_TARGET")"

  if [[ "$current" == "$EXPECTED_ORIGINAL_SHA256" ]]; then
    echo "Client is already pristine."
    echo "RESTORE_ORIGINAL_CHECK=PASS"
    return 0
  fi

  local pristine
  pristine="$(find_pristine_backup || true)"
  [[ -n "$pristine" ]] || die "No verified pristine MfbC01.smd found in backups."

  backup_current_client "before_restore_original"
  cp -a -- "$pristine" "$CLIENT_TARGET"

  [[ "$(sha256_file "$CLIENT_TARGET")" == "$EXPECTED_ORIGINAL_SHA256" ]] || \
    die "Pristine restore verification failed."

  record_history "restore_original" "$EXPECTED_ORIGINAL_SHA256" "$pristine"

  echo "source=$pristine"
  echo "sha256=$EXPECTED_ORIGINAL_SHA256"
  echo "RESTORE_ORIGINAL_CHECK=PASS"
}

status() {
  ensure_dirs
  echo "PT_C01_STATUS"
  echo "root=$ROOT"
  echo "blend=$BLEND"
  echo "original_smd=$ORIGINAL_SMD"
  echo "build=$FINAL_SMD"
  echo "client_root=$CLIENT_ROOT"
  echo

  if [[ -f "$ORIGINAL_SMD" ]]; then
    echo "original_size=$(file_size "$ORIGINAL_SMD")"
    echo "original_sha256=$(sha256_file "$ORIGINAL_SMD")"
  else
    echo "original=MISSING"
  fi

  if [[ -f "$BLEND" ]]; then
    echo "blend_size=$(file_size "$BLEND")"
  else
    echo "blend=MISSING"
  fi

  if [[ -f "$FINAL_SMD" ]]; then
    echo "build_size=$(file_size "$FINAL_SMD")"
    echo "build_sha256=$(sha256_file "$FINAL_SMD")"
  else
    echo "build=NOT_EXPORTED"
  fi

  if [[ -d "$CLIENT_ROOT" ]]; then
    resolve_client_target
    echo "client_target=$CLIENT_TARGET"
    echo "client_size=$(file_size "$CLIENT_TARGET")"
    echo "client_sha256=$(sha256_file "$CLIENT_TARGET")"
    if known_managed_hash "$(sha256_file "$CLIENT_TARGET")"; then
      echo "client_state=MANAGED"
    else
      echo "client_state=UNKNOWN"
    fi
  else
    echo "client_target=UNAVAILABLE"
  fi

  if [[ -L "$BACKUP_ROOT/latest" || -d "$BACKUP_ROOT/latest" ]]; then
    echo "latest_backup=$(readlink -f "$BACKUP_ROOT/latest")"
  else
    echo "latest_backup=NONE"
  fi
}

history() {
  ensure_dirs
  cat "$HISTORY"
}

usage() {
  cat <<'EOF'
Usage:
  ./pt_c01_tool.sh status
  ./pt_c01_tool.sh export
  ./pt_c01_tool.sh push
  ./pt_c01_tool.sh restore-previous
  ./pt_c01_tool.sh restore-original
  ./pt_c01_tool.sh history

Typical development loop:
  1. Edit and save:
       MfbC01_direct_smd_v3_static_armature_testv2.blend

  2. Export only:
       ./pt_c01_tool.sh export

     or export + backup + install:
       ./pt_c01_tool.sh push

  3. Test client.

  4. Roll back one client version:
       ./pt_c01_tool.sh restore-previous

  5. Return to verified pristine original:
       ./pt_c01_tool.sh restore-original

Environment overrides:
  PT_BLEND=/path/file.blend
  PT_CLIENT_ROOT=/path/client
  PT_BUILD_DIR=/path/build
EOF
}

case "${1:-}" in
  status) status ;;
  export) export_lods ;;
  push) push_build ;;
  restore-previous) restore_previous ;;
  restore-original|restore) restore_original ;;
  history) history ;;
  *) usage; exit 2 ;;
esac

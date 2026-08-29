#!/usr/bin/env bash
set -euo pipefail

# Summit Tale / Priestess C01 Tool V2
#
# Proven editing scope:
#   - existing topology only
#   - MshC01 / MsmC01 / MslC01
#   - serialized SMD XYZ patches via pt_blender_patch_smd_vertices_v1.py
#
# Main commands:
#   doctor
#   status
#   export [--force]
#   push [--force]
#   undo
#   redo
#   restore-original
#   history
#
# Environment overrides:
#   PT_BLEND
#   PT_CLIENT_ROOT
#   PT_BUILD_DIR
#   PT_BACKUP_ROOT

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

BLENDER_APP="${PT_BLENDER_APP:-org.blender.Blender}"
BLEND="${PT_BLEND:-$ROOT/MfbC01_direct_smd_v3_static_armature_testv2.blend}"
ORIGINAL_SMD="${PT_ORIGINAL_SMD:-$ROOT/MfbC01.smd}"
SKELETON="${PT_SKELETON:-$ROOT/m5.smb}"
PATCHER="${PT_PATCHER:-$ROOT/pt_blender_patch_smd_vertices_v1.py}"

CLIENT_ROOT="${PT_CLIENT_ROOT:-/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
BUILD_DIR="${PT_BUILD_DIR:-$ROOT/pt_build/C01}"
BACKUP_ROOT="${PT_BACKUP_ROOT:-$ROOT/client_backups/C01_v2}"

FINAL_SMD="$BUILD_DIR/MfbC01_current.smd"
BUILD_MANIFEST="$BUILD_DIR/MfbC01_current_manifest.txt"
HISTORY="$BUILD_DIR/history_v2.tsv"
UNDO_STACK="$BUILD_DIR/undo_stack.txt"
REDO_STACK="$BUILD_DIR/redo_stack.txt"

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

now_iso() {
  date --iso-8601=seconds
}

ensure_dirs() {
  mkdir -p "$BUILD_DIR" "$BACKUP_ROOT"
  touch "$UNDO_STACK" "$REDO_STACK"
  if [[ ! -f "$HISTORY" ]]; then
    printf 'timestamp\taction\tsha256\tnote\n' > "$HISTORY"
  fi
}

record_history() {
  local action="$1" hash="$2" note="$3"
  ensure_dirs
  printf '%s\t%s\t%s\t%s\n' "$(now_iso)" "$action" "$hash" "$note" >> "$HISTORY"
}

validate_original() {
  [[ -f "$ORIGINAL_SMD" ]] || die "Original SMD not found: $ORIGINAL_SMD"
  [[ "$(file_size "$ORIGINAL_SMD")" == "$EXPECTED_SIZE" ]] || \
    die "Original SMD size mismatch."
  [[ "$(sha256_file "$ORIGINAL_SMD")" == "$EXPECTED_ORIGINAL_SHA256" ]] || \
    die "Original SMD SHA256 mismatch."
}

validate_smd_basic() {
  local f="$1"
  [[ -f "$f" ]] || die "SMD not found: $f"
  [[ "$(file_size "$f")" == "$EXPECTED_SIZE" ]] || \
    die "Unexpected SMD size $(file_size "$f"); expected $EXPECTED_SIZE: $f"

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

input_fingerprint() {
  [[ -f "$BLEND" ]] || die "Blend not found: $BLEND"
  [[ -f "$PATCHER" ]] || die "Patcher not found: $PATCHER"
  [[ -f "$SKELETON" ]] || die "Skeleton not found: $SKELETON"
  validate_original

  printf '%s\n' \
    "blend=$(sha256_file "$BLEND")" \
    "patcher=$(sha256_file "$PATCHER")" \
    "skeleton=$(sha256_file "$SKELETON")" \
    "original=$(sha256_file "$ORIGINAL_SMD")"
}

manifest_value() {
  local key="$1"
  [[ -f "$BUILD_MANIFEST" ]] || return 1
  awk -F= -v k="$key" '$1==k {sub(/^[^=]*=/,""); print; exit}' "$BUILD_MANIFEST"
}

build_cache_valid() {
  [[ -f "$FINAL_SMD" && -f "$BUILD_MANIFEST" ]] || return 1
  validate_smd_basic "$FINAL_SMD" || return 1

  local current_fp saved_fp output_sha actual_sha
  current_fp="$(input_fingerprint)"
  saved_fp="$(
    printf '%s\n' \
      "blend=$(manifest_value blend_sha256 || true)" \
      "patcher=$(manifest_value patcher_sha256 || true)" \
      "skeleton=$(manifest_value skeleton_sha256 || true)" \
      "original=$(manifest_value original_sha256 || true)"
  )"

  [[ "$current_fp" == "$saved_fp" ]] || return 1

  output_sha="$(manifest_value output_sha256 || true)"
  actual_sha="$(sha256_file "$FINAL_SMD")"
  [[ -n "$output_sha" && "$output_sha" == "$actual_sha" ]]
}

run_blender_patch() {
  local input_smd="$1" object="$2" output_smd="$3" report="$4"
  flatpak run --filesystem=home "$BLENDER_APP" \
    --background "$BLEND" \
    --python "$PATCHER" -- \
    --smd "$input_smd" \
    --skeleton "$SKELETON" \
    --object "$object" \
    --out "$output_smd" \
    --report "$report"
}

do_export() {
  local force="${1:-0}"
  ensure_dirs

  if [[ "$force" != "1" ]] && build_cache_valid; then
    echo "EXPORT_CACHE=HIT"
    echo "output=$FINAL_SMD"
    echo "sha256=$(sha256_file "$FINAL_SMD")"
    echo "EXPORT_CHECK=PASS"
    return 0
  fi

  [[ -f "$BLEND" ]] || die "Blend not found: $BLEND"
  [[ -f "$PATCHER" ]] || die "Patcher not found: $PATCHER"
  [[ -f "$SKELETON" ]] || die "Skeleton not found: $SKELETON"
  validate_original

  local work
  work="$(mktemp -d "$BUILD_DIR/.export.XXXXXX")"

  cleanup_export() {
    rm -rf -- "$work"
  }
  trap cleanup_export RETURN

  local input="$ORIGINAL_SMD"
  local last=""
  local i=0 object stage report

  echo "PT_C01_EXPORT_V2"
  echo "blend=$BLEND"
  echo

  for object in "${OBJECTS[@]}"; do
    i=$((i + 1))
    stage="$work/$(printf '%02d' "$i")_${object}.smd"
    report="$BUILD_DIR/${object}_report.json"

    echo "===== ${object} ====="
    run_blender_patch "$input" "$object" "$stage" "$report"
    echo

    input="$stage"
    last="$stage"
  done

  [[ -n "$last" ]] || die "No export stage produced."
  validate_smd_basic "$last"

  local tmp="$BUILD_DIR/.MfbC01_current.smd.tmp.$$"
  cp -- "$last" "$tmp"
  validate_smd_basic "$tmp"
  mv -f -- "$tmp" "$FINAL_SMD"

  local blend_sha patcher_sha skeleton_sha original_sha output_sha
  blend_sha="$(sha256_file "$BLEND")"
  patcher_sha="$(sha256_file "$PATCHER")"
  skeleton_sha="$(sha256_file "$SKELETON")"
  original_sha="$(sha256_file "$ORIGINAL_SMD")"
  output_sha="$(sha256_file "$FINAL_SMD")"

  [[ "$output_sha" != "$original_sha" ]] || \
    die "Export is byte-identical to original; no torso edit detected."

  {
    printf 'generated_at=%s\n' "$(now_iso)"
    printf 'blend=%s\n' "$BLEND"
    printf 'blend_sha256=%s\n' "$blend_sha"
    printf 'patcher=%s\n' "$PATCHER"
    printf 'patcher_sha256=%s\n' "$patcher_sha"
    printf 'skeleton=%s\n' "$SKELETON"
    printf 'skeleton_sha256=%s\n' "$skeleton_sha"
    printf 'original_smd=%s\n' "$ORIGINAL_SMD"
    printf 'original_sha256=%s\n' "$original_sha"
    printf 'output=%s\n' "$FINAL_SMD"
    printf 'output_size=%s\n' "$(file_size "$FINAL_SMD")"
    printf 'output_sha256=%s\n' "$output_sha"
    printf 'objects=%s\n' "${OBJECTS[*]}"
  } > "$BUILD_MANIFEST"

  record_history "export" "$output_sha" "$FINAL_SMD"

  echo "===== EXPORT RESULT ====="
  echo "output=$FINAL_SMD"
  echo "size=$(file_size "$FINAL_SMD")"
  echo "sha256=$output_sha"
  echo "EXPORT_CACHE=MISS"
  echo "EXPORT_CHECK=PASS"

  trap - RETURN
  cleanup_export
}

backup_client_to_dir() {
  local label="$1"
  resolve_client_target
  validate_smd_basic "$CLIENT_TARGET"

  local stamp current_sha dir backup
  stamp="$(date '+%Y%m%d_%H%M%S_%N')"
  current_sha="$(sha256_file "$CLIENT_TARGET")"
  dir="$BACKUP_ROOT/${label}_${stamp}"
  backup="$dir/MfbC01.smd"

  mkdir -p "$dir"
  cp -a -- "$CLIENT_TARGET" "$backup"

  [[ "$(sha256_file "$backup")" == "$current_sha" ]] || \
    die "Backup verification failed."

  {
    printf 'created_at=%s\n' "$(now_iso)"
    printf 'client_target=%s\n' "$CLIENT_TARGET"
    printf 'sha256=%s\n' "$current_sha"
  } > "$dir/manifest.txt"

  BACKUP_DIR_RESULT="$dir"
  BACKUP_FILE_RESULT="$backup"
  BACKUP_SHA_RESULT="$current_sha"
}

stack_push() {
  local stack="$1" value="$2"
  printf '%s\n' "$value" >> "$stack"
}

stack_peek() {
  local stack="$1"
  tail -n 1 "$stack"
}

stack_pop() {
  local stack="$1"
  local n
  n="$(wc -l < "$stack")"
  (( n > 0 )) || return 1
  sed -i '${d;}' "$stack"
}

stack_count() {
  wc -l < "$1" | tr -d ' '
}

push_build() {
  local force="${1:-0}"
  ensure_dirs
  do_export "$force"

  resolve_client_target
  validate_smd_basic "$CLIENT_TARGET"
  validate_smd_basic "$FINAL_SMD"

  local current_sha build_sha
  current_sha="$(sha256_file "$CLIENT_TARGET")"
  build_sha="$(sha256_file "$FINAL_SMD")"

  echo
  echo "PT_C01_PUSH_V2"
  echo "client_sha256=$current_sha"
  echo "build_sha256=$build_sha"

  [[ "$current_sha" != "$build_sha" ]] || \
    die "Client already contains this exact build."

  # Current target must be pristine or a build that has been recorded.
  if [[ "$current_sha" != "$EXPECTED_ORIGINAL_SHA256" ]]; then
    if ! awk -F'\t' -v h="$current_sha" 'NR>1 && $3==h {found=1} END{exit !found}' "$HISTORY"; then
      die "Current client SMD is neither pristine nor recorded by V2."
    fi
  fi

  backup_client_to_dir "undo"
  stack_push "$UNDO_STACK" "$BACKUP_DIR_RESULT"
  : > "$REDO_STACK"

  local tmp
  tmp="$(dirname "$CLIENT_TARGET")/.MfbC01.smd.pt_c01_v2.tmp.$$"
  cp -- "$FINAL_SMD" "$tmp"
  chmod --reference="$CLIENT_TARGET" "$tmp" 2>/dev/null || true

  [[ "$(sha256_file "$tmp")" == "$build_sha" ]] || {
    rm -f -- "$tmp"
    die "Temporary copy verification failed."
  }

  mv -f -- "$tmp" "$CLIENT_TARGET"

  if [[ "$(sha256_file "$CLIENT_TARGET")" != "$build_sha" ]]; then
    cp -a -- "$BACKUP_FILE_RESULT" "$CLIENT_TARGET"
    stack_pop "$UNDO_STACK" || true
    die "Push verification failed; previous client state restored."
  fi

  record_history "push" "$build_sha" "$CLIENT_TARGET"

  echo "undo_depth=$(stack_count "$UNDO_STACK")"
  echo "redo_depth=$(stack_count "$REDO_STACK")"
  echo "backup=$BACKUP_DIR_RESULT"
  echo "PUSH_CHECK=PASS"
}

restore_from_stack() {
  local from_stack="$1" to_stack="$2" action="$3"
  ensure_dirs
  resolve_client_target

  local source_dir source_file source_sha current_sha
  source_dir="$(stack_peek "$from_stack" || true)"
  [[ -n "$source_dir" ]] || die "No state available for $action."

  source_file="$source_dir/MfbC01.smd"
  validate_smd_basic "$source_file"
  source_sha="$(sha256_file "$source_file")"
  current_sha="$(sha256_file "$CLIENT_TARGET")"

  # Save current state into the opposite stack before replacing it.
  backup_client_to_dir "${action}_reverse"
  stack_push "$to_stack" "$BACKUP_DIR_RESULT"

  cp -a -- "$source_file" "$CLIENT_TARGET"
  if [[ "$(sha256_file "$CLIENT_TARGET")" != "$source_sha" ]]; then
    cp -a -- "$BACKUP_FILE_RESULT" "$CLIENT_TARGET"
    stack_pop "$to_stack" || true
    die "$action verification failed; current state restored."
  fi

  stack_pop "$from_stack"
  record_history "$action" "$source_sha" "$source_dir"

  echo "restored_sha256=$source_sha"
  echo "replaced_sha256=$current_sha"
  echo "undo_depth=$(stack_count "$UNDO_STACK")"
  echo "redo_depth=$(stack_count "$REDO_STACK")"
  printf '%s_CHECK=PASS\n' "$(printf '%s' "$action" | tr '[:lower:]-' '[:upper:]_')"
}

undo() {
  restore_from_stack "$UNDO_STACK" "$REDO_STACK" "undo"
}

redo() {
  restore_from_stack "$REDO_STACK" "$UNDO_STACK" "redo"
}

restore_original() {
  ensure_dirs
  validate_original
  resolve_client_target

  local current_sha
  current_sha="$(sha256_file "$CLIENT_TARGET")"

  if [[ "$current_sha" == "$EXPECTED_ORIGINAL_SHA256" ]]; then
    echo "Client is already pristine."
    echo "RESTORE_ORIGINAL_CHECK=PASS"
    return 0
  fi

  # Make this restoration undoable.
  backup_client_to_dir "restore_original_reverse"
  stack_push "$UNDO_STACK" "$BACKUP_DIR_RESULT"
  : > "$REDO_STACK"

  cp -a -- "$ORIGINAL_SMD" "$CLIENT_TARGET"

  if [[ "$(sha256_file "$CLIENT_TARGET")" != "$EXPECTED_ORIGINAL_SHA256" ]]; then
    cp -a -- "$BACKUP_FILE_RESULT" "$CLIENT_TARGET"
    stack_pop "$UNDO_STACK" || true
    die "Original restore verification failed."
  fi

  record_history "restore_original" "$EXPECTED_ORIGINAL_SHA256" "$ORIGINAL_SMD"

  echo "original=$ORIGINAL_SMD"
  echo "undo_depth=$(stack_count "$UNDO_STACK")"
  echo "RESTORE_ORIGINAL_CHECK=PASS"
}

doctor() {
  ensure_dirs
  local failures=0

  check_file() {
    local label="$1" path="$2"
    if [[ -f "$path" ]]; then
      printf 'PASS  %-18s %s\n' "$label" "$path"
    else
      printf 'FAIL  %-18s %s\n' "$label" "$path"
      failures=$((failures + 1))
    fi
  }

  echo "PT_C01_DOCTOR_V2"
  check_file "blend" "$BLEND"
  check_file "original_smd" "$ORIGINAL_SMD"
  check_file "skeleton" "$SKELETON"
  check_file "patcher" "$PATCHER"

  if command -v flatpak >/dev/null 2>&1; then
    printf 'PASS  %-18s %s\n' "flatpak" "$(command -v flatpak)"
  else
    printf 'FAIL  %-18s %s\n' "flatpak" "not found"
    failures=$((failures + 1))
  fi

  if [[ -d "$CLIENT_ROOT" ]]; then
    printf 'PASS  %-18s %s\n' "client_root" "$CLIENT_ROOT"
    if resolve_client_target 2>/dev/null; then
      printf 'PASS  %-18s %s\n' "client_smd" "$CLIENT_TARGET"
    else
      printf 'FAIL  %-18s %s\n' "client_smd" "not uniquely resolved"
      failures=$((failures + 1))
    fi
  else
    printf 'FAIL  %-18s %s\n' "client_root" "$CLIENT_ROOT"
    failures=$((failures + 1))
  fi

  if [[ -f "$ORIGINAL_SMD" ]]; then
    if [[ "$(sha256_file "$ORIGINAL_SMD")" == "$EXPECTED_ORIGINAL_SHA256" ]]; then
      printf 'PASS  %-18s %s\n' "original_hash" "$EXPECTED_ORIGINAL_SHA256"
    else
      printf 'FAIL  %-18s %s\n' "original_hash" "$(sha256_file "$ORIGINAL_SMD")"
      failures=$((failures + 1))
    fi
  fi

  if (( failures == 0 )); then
    echo "DOCTOR_CHECK=PASS"
  else
    echo "DOCTOR_CHECK=FAIL failures=$failures"
    return 1
  fi
}

status() {
  ensure_dirs
  echo "PT_C01_STATUS_V2"
  echo "blend=$BLEND"
  echo "build=$FINAL_SMD"
  echo "client_root=$CLIENT_ROOT"
  echo

  if [[ -f "$BLEND" ]]; then
    echo "blend_sha256=$(sha256_file "$BLEND")"
  else
    echo "blend=MISSING"
  fi

  if [[ -f "$FINAL_SMD" ]]; then
    echo "build_sha256=$(sha256_file "$FINAL_SMD")"
    if build_cache_valid; then
      echo "build_cache=VALID"
    else
      echo "build_cache=STALE"
    fi
  else
    echo "build=NOT_EXPORTED"
  fi

  if [[ -d "$CLIENT_ROOT" ]]; then
    resolve_client_target
    echo "client_target=$CLIENT_TARGET"
    echo "client_sha256=$(sha256_file "$CLIENT_TARGET")"
  else
    echo "client=UNAVAILABLE"
  fi

  echo "undo_depth=$(stack_count "$UNDO_STACK")"
  echo "redo_depth=$(stack_count "$REDO_STACK")"
}

show_history() {
  ensure_dirs
  if command -v column >/dev/null 2>&1; then
    column -t -s $'\t' "$HISTORY"
  else
    cat "$HISTORY"
  fi
}

parse_force() {
  if [[ "${1:-}" == "--force" ]]; then
    printf '1'
  else
    printf '0'
  fi
}

usage() {
  cat <<'EOF'
Usage:
  ./pt_c01_tool_v2.sh doctor
  ./pt_c01_tool_v2.sh status
  ./pt_c01_tool_v2.sh export [--force]
  ./pt_c01_tool_v2.sh push [--force]
  ./pt_c01_tool_v2.sh undo
  ./pt_c01_tool_v2.sh redo
  ./pt_c01_tool_v2.sh restore-original
  ./pt_c01_tool_v2.sh history

Recommended loop:
  edit .blend -> save -> ./pt_c01_tool_v2.sh push -> test client

Improvements over V1:
  - export cache: push does not re-export an unchanged .blend
  - real undo/redo stacks
  - restore-original is itself undoable
  - doctor command
  - formatted history
  - same proven XYZ-only Blender patcher underneath

Use --force to rebuild even when the cache is valid.
EOF
}

case "${1:-}" in
  doctor) doctor ;;
  status) status ;;
  export) do_export "$(parse_force "${2:-}")" ;;
  push) push_build "$(parse_force "${2:-}")" ;;
  undo|restore-previous) undo ;;
  redo) redo ;;
  restore-original|restore) restore_original ;;
  history) show_history ;;
  *) usage; exit 2 ;;
esac

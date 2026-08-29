#!/usr/bin/env bash
set -euo pipefail

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BACKUP_ROOT="$SCRIPT_DIR/client_backups"
EXPECTED_ORIGINAL_SHA256="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
EXPECTED_SIZE="290229"

die() {
  printf 'ERROR: %s\n' "$*" >&2
  exit 1
}

sha256_file() {
  sha256sum -- "$1" | awk '{print $1}'
}

resolve_single_target() {
  [[ -d "$CLIENT" ]] || die "Client directory not found: $CLIENT"
  mapfile -t TARGETS < <(find "$CLIENT" -type f -iname 'MfbC01.smd' -print)
  if (( ${#TARGETS[@]} != 1 )); then
    echo "Found ${#TARGETS[@]} MfbC01.smd candidate(s):" >&2
    printf '  %s\n' "${TARGETS[@]:-}" >&2
    die "Refusing unless exactly one MfbC01.smd exists."
  fi
  TARGET="${TARGETS[0]}"
}

known_test_hash() {
  local wanted="$1"
  [[ -d "$BACKUP_ROOT" ]] || return 1

  # Old/new manifests record replacement_sha256=...
  grep -Rhs --include='manifest.txt' '^replacement_sha256=' "$BACKUP_ROOT" 2>/dev/null \
    | cut -d= -f2- \
    | grep -Fxq -- "$wanted"
}

validate_smd_basic() {
  local file="$1"
  [[ -f "$file" ]] || die "SMD not found: $file"

  local size header
  size="$(stat -c %s "$file")"
  [[ "$size" == "$EXPECTED_SIZE" ]] || \
    die "Unexpected SMD size $size (expected $EXPECTED_SIZE): $file"

  header="$(head -c 23 "$file" || true)"
  [[ "$header" == "SMD Model data Ver 0.62" ]] || \
    die "Unexpected SMD header: $file"
}

backup_current() {
  local label="$1"
  local timestamp current_sha backup_dir backup_file relative

  current_sha="$(sha256_file "$TARGET")"
  timestamp="$(date '+%Y%m%d_%H%M%S')"
  backup_dir="$BACKUP_ROOT/${label}_${timestamp}"
  mkdir -p "$backup_dir/original"

  backup_file="$backup_dir/original/MfbC01.smd"
  cp -a -- "$TARGET" "$backup_file"
  [[ "$(sha256_file "$backup_file")" == "$current_sha" ]] || \
    die "Backup verification failed; target was not modified."

  relative="${TARGET#"$CLIENT"/}"
  {
    printf 'created_at=%s\n' "$(date --iso-8601=seconds)"
    printf 'client_root=%s\n' "$CLIENT"
    printf 'target=%s\n' "$TARGET"
    printf 'target_relative=%s\n' "$relative"
    printf 'original_sha256=%s\n' "$current_sha"
  } > "$backup_dir/manifest.txt"

  printf '%s\n' "$backup_dir"
}

install_or_update() {
  resolve_single_target

  local replacement="${1:-$SCRIPT_DIR/MfbC01_testv2_lods.smd}"
  replacement="$(readlink -f "$replacement")"

  validate_smd_basic "$TARGET"
  validate_smd_basic "$replacement"

  local target_sha replacement_sha
  target_sha="$(sha256_file "$TARGET")"
  replacement_sha="$(sha256_file "$replacement")"

  echo "TARGET=$TARGET"
  echo "TARGET_SHA256=$target_sha"
  echo "REPLACEMENT=$replacement"
  echo "REPLACEMENT_SHA256=$replacement_sha"

  [[ "$target_sha" != "$replacement_sha" ]] || \
    die "Target and replacement are already identical. Re-export the edited .blend first."

  # Accept pristine original OR a version previously installed/recorded by our workflow.
  if [[ "$target_sha" != "$EXPECTED_ORIGINAL_SHA256" ]] && ! known_test_hash "$target_sha"; then
    die "Current client SMD is neither pristine nor a previously recorded test build. Nothing changed."
  fi

  local backup_dir tmp
  backup_dir="$(backup_current MfbC01_update)"
  {
    printf 'replacement=%s\n' "$replacement"
    printf 'replacement_sha256=%s\n' "$replacement_sha"
  } >> "$backup_dir/manifest.txt"

  tmp="$(dirname "$TARGET")/.MfbC01.smd.summit_update.tmp.$$"
  cp -- "$replacement" "$tmp"
  chmod --reference="$TARGET" "$tmp" 2>/dev/null || true

  [[ "$(sha256_file "$tmp")" == "$replacement_sha" ]] || {
    rm -f -- "$tmp"
    die "Temporary replacement verification failed."
  }

  mv -f -- "$tmp" "$TARGET"

  if [[ "$(sha256_file "$TARGET")" != "$replacement_sha" ]]; then
    echo "Install verification failed; restoring immediately." >&2
    cp -a -- "$backup_dir/original/MfbC01.smd" "$TARGET"
    die "Replacement verification failed; previous version restored."
  fi

  ln -sfn -- "$backup_dir" "$BACKUP_ROOT/latest"

  echo
  echo "UPDATE_CHECK=PASS"
  echo "BACKUP_DIR=$backup_dir"
  echo "BACKED_UP_SHA256=$target_sha"
  echo "INSTALLED_SHA256=$replacement_sha"
  echo "Only MfbC01.smd was modified."
}

restore_previous() {
  resolve_single_target
  [[ -L "$BACKUP_ROOT/latest" || -d "$BACKUP_ROOT/latest" ]] || \
    die "No latest backup available."

  local backup_dir backup_file sha
  backup_dir="$(readlink -f "$BACKUP_ROOT/latest")"
  backup_file="$backup_dir/original/MfbC01.smd"
  [[ -f "$backup_file" ]] || die "Backup file missing: $backup_file"

  sha="$(sha256_file "$backup_file")"
  cp -a -- "$backup_file" "$TARGET"
  [[ "$(sha256_file "$TARGET")" == "$sha" ]] || die "Restore verification failed."

  echo "RESTORE_PREVIOUS_CHECK=PASS"
  echo "RESTORED_SHA256=$sha"
  echo "FROM=$backup_dir"
}

restore_original() {
  resolve_single_target

  local candidate=""
  if [[ "$(sha256_file "$TARGET")" == "$EXPECTED_ORIGINAL_SHA256" ]]; then
    echo "RESTORE_ORIGINAL_CHECK=PASS"
    echo "Client is already pristine."
    return 0
  fi

  if [[ -d "$BACKUP_ROOT" ]]; then
    while IFS= read -r f; do
      if [[ "$(sha256_file "$f")" == "$EXPECTED_ORIGINAL_SHA256" ]]; then
        candidate="$f"
        break
      fi
    done < <(find "$BACKUP_ROOT" -type f -path '*/original/MfbC01.smd' -print | sort)
  fi

  [[ -n "$candidate" ]] || die "No pristine original backup found."

  cp -a -- "$candidate" "$TARGET"
  [[ "$(sha256_file "$TARGET")" == "$EXPECTED_ORIGINAL_SHA256" ]] || \
    die "Original restore verification failed."

  echo "RESTORE_ORIGINAL_CHECK=PASS"
  echo "RESTORED=$TARGET"
  echo "SHA256=$EXPECTED_ORIGINAL_SHA256"
  echo "SOURCE_BACKUP=$candidate"
}

scan() {
  resolve_single_target
  echo "TARGET=$TARGET"
  echo "TARGET_SHA256=$(sha256_file "$TARGET")"
  echo "TARGET_SIZE=$(stat -c %s "$TARGET")"
  echo
  echo "Recorded backups:"
  if [[ -d "$BACKUP_ROOT" ]]; then
    find "$BACKUP_ROOT" -name manifest.txt -print | sort
  else
    echo "(none)"
  fi
}

case "${1:-}" in
  scan)
    scan
    ;;
  update|install)
    install_or_update "${2:-}"
    ;;
  restore-previous)
    restore_previous
    ;;
  restore-original|restore)
    restore_original
    ;;
  *)
    cat <<'EOF'
Usage:
  ./pt_client_mfbc01_safe_v2.sh scan
  ./pt_client_mfbc01_safe_v2.sh update [replacement.smd]
  ./pt_client_mfbc01_safe_v2.sh restore-previous
  ./pt_client_mfbc01_safe_v2.sh restore-original

update:
  Accepts the pristine SMD or a test SMD previously recorded by this workflow.
  Always backs up the currently installed MfbC01.smd before replacement.
  Refuses if target == replacement (usually means the edited .blend was not re-exported).

restore-previous:
  Goes back one installed version using client_backups/latest.

restore-original:
  Finds a backup whose SHA256 matches the known pristine MfbC01.smd and restores it.
EOF
    exit 2
    ;;
esac

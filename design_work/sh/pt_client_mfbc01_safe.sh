#!/usr/bin/env bash
set -euo pipefail

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPLACEMENT="$SCRIPT_DIR/MfbC01_testv2.smd"
BACKUP_ROOT="$SCRIPT_DIR/client_backups"

EXPECTED_ORIGINAL_SHA256="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
EXPECTED_TESTV2_SHA256="e9fa4db4afb39849fa9105df5f09aba1c7d4a52cb939bd26a3318608d577955b"

die() {
  printf 'ERROR: %s\n' "$*" >&2
  exit 1
}

sha256_file() {
  sha256sum -- "$1" | awk '{print $1}'
}

find_exact_asset() {
  local name="$1"
  find "$CLIENT" -type f -iname "$name" -print
}

scan_assets() {
  [[ -d "$CLIENT" ]] || die "Client directory not found: $CLIENT"

  echo "CLIENT_ROOT=$CLIENT"
  echo
  echo "Relevant Priestess/base assets found:"
  find "$CLIENT" -type f \( \
      -iname 'MfbC01.smd' -o \
      -iname 'MfbC01.smb' -o \
      -iname 'MfbC01.ASE' -o \
      -iname 'MfbC01.bmp' -o \
      -iname 'MfbC03.bmp' -o \
      -iname 'mo-p-ac.BMP' -o \
      -iname 'm5.smb' -o \
      -iname 'm5.ase' -o \
      -iname 'M5Bip.inx' -o \
      -iname 'mC001.ini' -o \
      -iname 'mC001.inx' \
    \) -print | sort

  echo
  echo "Exact MfbC01.smd candidates:"
  mapfile -t targets < <(find_exact_asset 'MfbC01.smd')
  printf 'count=%d\n' "${#targets[@]}"
  for f in "${targets[@]}"; do
    printf '%s  %s\n' "$(sha256_file "$f")" "$f"
  done
}

resolve_single_target() {
  [[ -d "$CLIENT" ]] || die "Client directory not found: $CLIENT"
  mapfile -t TARGETS < <(find_exact_asset 'MfbC01.smd')
  if (( ${#TARGETS[@]} != 1 )); then
    echo "Found ${#TARGETS[@]} MfbC01.smd candidate(s):" >&2
    printf '  %s\n' "${TARGETS[@]:-}" >&2
    die "Refusing to modify anything unless exactly one MfbC01.smd exists."
  fi
  TARGET="${TARGETS[0]}"
}

install_test() {
  resolve_single_target
  [[ -f "$REPLACEMENT" ]] || die "Replacement not found: $REPLACEMENT"

  local target_sha replacement_sha timestamp backup_dir backup_file relative tmp
  target_sha="$(sha256_file "$TARGET")"
  replacement_sha="$(sha256_file "$REPLACEMENT")"

  echo "TARGET=$TARGET"
  echo "TARGET_SHA256=$target_sha"
  echo "REPLACEMENT=$REPLACEMENT"
  echo "REPLACEMENT_SHA256=$replacement_sha"

  [[ "$replacement_sha" == "$EXPECTED_TESTV2_SHA256" ]] || \
    die "Replacement hash is not the approved MfbC01_testv2.smd hash."

  [[ "$target_sha" == "$EXPECTED_ORIGINAL_SHA256" ]] || \
    die "Client MfbC01.smd is not the expected pristine original. Nothing changed."

  timestamp="$(date '+%Y%m%d_%H%M%S')"
  backup_dir="$BACKUP_ROOT/MfbC01_testv2_$timestamp"
  mkdir -p "$backup_dir/original"

  relative="${TARGET#"$CLIENT"/}"
  backup_file="$backup_dir/original/MfbC01.smd"

  cp -a -- "$TARGET" "$backup_file"
  [[ "$(sha256_file "$backup_file")" == "$target_sha" ]] || \
    die "Backup verification failed. Client has NOT been modified."

  {
    printf 'created_at=%s\n' "$(date --iso-8601=seconds)"
    printf 'client_root=%s\n' "$CLIENT"
    printf 'target=%s\n' "$TARGET"
    printf 'target_relative=%s\n' "$relative"
    printf 'original_sha256=%s\n' "$target_sha"
    printf 'replacement=%s\n' "$REPLACEMENT"
    printf 'replacement_sha256=%s\n' "$replacement_sha"
  } > "$backup_dir/manifest.txt"

  # Inventory related files for later diagnosis. These are NOT modified.
  find "$CLIENT" -type f \( \
      -iname 'MfbC01.smd' -o \
      -iname 'MfbC01.smb' -o \
      -iname 'MfbC01.ASE' -o \
      -iname 'MfbC01.bmp' -o \
      -iname 'MfbC03.bmp' -o \
      -iname 'mo-p-ac.BMP' -o \
      -iname 'm5.smb' -o \
      -iname 'm5.ase' -o \
      -iname 'M5Bip.inx' -o \
      -iname 'mC001.ini' -o \
      -iname 'mC001.inx' \
    \) -print | sort > "$backup_dir/asset_inventory.txt"

  tmp="$(dirname "$TARGET")/.MfbC01.smd.summit_testv2.tmp.$$"
  cp -- "$REPLACEMENT" "$tmp"
  chmod --reference="$TARGET" "$tmp" 2>/dev/null || true
  touch --reference="$TARGET" "$tmp" 2>/dev/null || true

  [[ "$(sha256_file "$tmp")" == "$replacement_sha" ]] || {
    rm -f -- "$tmp"
    die "Temporary replacement verification failed."
  }

  mv -f -- "$tmp" "$TARGET"

  [[ "$(sha256_file "$TARGET")" == "$replacement_sha" ]] || {
    echo "Install verification failed; restoring backup immediately." >&2
    cp -a -- "$backup_file" "$TARGET"
    die "Replacement verification failed; original restored."
  }

  ln -sfn -- "$backup_dir" "$BACKUP_ROOT/latest"

  echo
  echo "INSTALL_CHECK=PASS"
  echo "BACKUP_DIR=$backup_dir"
  echo "BACKUP_SHA256=$(sha256_file "$backup_file")"
  echo "INSTALLED_SHA256=$(sha256_file "$TARGET")"
  echo "Only MfbC01.smd was modified."
}

restore_backup() {
  resolve_single_target

  local backup_dir="${1:-}"
  if [[ -z "$backup_dir" ]]; then
    [[ -L "$BACKUP_ROOT/latest" || -d "$BACKUP_ROOT/latest" ]] || \
      die "No latest backup pointer found under $BACKUP_ROOT"
    backup_dir="$(readlink -f "$BACKUP_ROOT/latest")"
  else
    backup_dir="$(readlink -f "$backup_dir")"
  fi

  local backup_file="$backup_dir/original/MfbC01.smd"
  [[ -f "$backup_file" ]] || die "Backup file not found: $backup_file"

  local backup_sha
  backup_sha="$(sha256_file "$backup_file")"
  [[ "$backup_sha" == "$EXPECTED_ORIGINAL_SHA256" ]] || \
    die "Backup does not match expected pristine original hash."

  cp -a -- "$backup_file" "$TARGET"

  [[ "$(sha256_file "$TARGET")" == "$backup_sha" ]] || \
    die "Restore verification failed."

  echo "RESTORE_CHECK=PASS"
  echo "RESTORED=$TARGET"
  echo "SHA256=$backup_sha"
}

usage() {
  cat <<'EOF'
Usage:
  ./pt_client_mfbc01_safe.sh scan
  ./pt_client_mfbc01_safe.sh install
  ./pt_client_mfbc01_safe.sh restore [backup_dir]

scan:
  Finds relevant Priestess/base assets and prints hashes for MfbC01.smd.
  Makes no changes.

install:
  Requires exactly one MfbC01.smd.
  Requires the client's SMD to match the known pristine SHA256.
  Requires MfbC01_testv2.smd to match the approved test SHA256.
  Creates and verifies a dated backup before replacing the SMD.
  Aborts without changes on any mismatch.

restore:
  Restores the pristine MfbC01.smd from the latest backup by default,
  or from a specific backup directory.
EOF
}

case "${1:-}" in
  scan)
    scan_assets
    ;;
  install)
    install_test
    ;;
  restore)
    restore_backup "${2:-}"
    ;;
  *)
    usage
    exit 2
    ;;
esac

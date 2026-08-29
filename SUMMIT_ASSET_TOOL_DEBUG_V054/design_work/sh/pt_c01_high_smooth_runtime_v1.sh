#!/usr/bin/env bash
set -euo pipefail

# Priestess C01 HIGH smooth topology runtime gate V1
#
# Approved build:
#   MshC01 HIGH smooth patch
#   173 -> 197 vertices
#   331 -> 367 faces
#   size 294021
#   sha256 b1678aced23e84f560e65be56a2c11b5f35980228ee9afe32548c1424197ac23
#
# Separate from SAFE XYZ workflow.
# Every client modification is backed up and verified.

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"

CLIENT_ROOT="${PT_CLIENT_ROOT:-/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
BUILD="${PT_HIGH_SMOOTH_BUILD:-$ROOT/pt_build/C01_topology/MfbC01_high_smooth_patch_v1.smd}"
BACKUP_ROOT="${PT_HIGH_SMOOTH_BACKUP_ROOT:-$ROOT/client_backups/C01_high_smooth_v1}"

PRISTINE_SHA256="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
PRISTINE_SIZE="290229"

APPROVED_SHA256="b1678aced23e84f560e65be56a2c11b5f35980228ee9afe32548c1424197ac23"
APPROVED_SIZE="294021"

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

validate_header() {
  local f="$1"
  [[ -f "$f" ]] || die "File not found: $f"
  local hdr
  hdr="$(head -c 23 -- "$f" || true)"
  [[ "$hdr" == "SMD Model data Ver 0.62" ]] || die "Unexpected SMD header: $f"
}

resolve_target() {
  [[ -d "$CLIENT_ROOT" ]] || die "Client root not found: $CLIENT_ROOT"
  mapfile -t targets < <(find "$CLIENT_ROOT" -type f -iname 'MfbC01.smd' -print)
  if (( ${#targets[@]} != 1 )); then
    printf 'Found %d MfbC01.smd candidate(s):\n' "${#targets[@]}" >&2
    printf '  %s\n' "${targets[@]:-}" >&2
    die "Expected exactly one runtime MfbC01.smd."
  fi
  TARGET="${targets[0]}"
}

known_hash() {
  local h="$1"

  [[ "$h" == "$PRISTINE_SHA256" ]] && return 0
  [[ "$h" == "$APPROVED_SHA256" ]] && return 0

  if [[ -d "$ROOT/client_backups" ]]; then
    if grep -Rhs --include='manifest.txt' \
      -E '^(replacement_sha256|sha256|original_sha256)=' \
      "$ROOT/client_backups" 2>/dev/null \
      | cut -d= -f2- \
      | grep -Fxq -- "$h"; then
      return 0
    fi
  fi

  if [[ -f "$ROOT/pt_build/C01/history_v2.tsv" ]]; then
    if awk -F'\t' -v h="$h" 'NR>1 && $3==h {found=1} END{exit !found}' \
      "$ROOT/pt_build/C01/history_v2.tsv"; then
      return 0
    fi
  fi

  return 1
}

validate_build() {
  validate_header "$BUILD"

  local size hash
  size="$(file_size "$BUILD")"
  hash="$(sha256_file "$BUILD")"

  [[ "$size" == "$APPROVED_SIZE" ]] || \
    die "Approved HIGH smooth build size mismatch: $size != $APPROVED_SIZE"
  [[ "$hash" == "$APPROVED_SHA256" ]] || \
    die "Approved HIGH smooth build SHA256 mismatch: $hash"

  echo "BUILD=$BUILD"
  echo "BUILD_SIZE=$size"
  echo "BUILD_SHA256=$hash"
}

backup_current() {
  resolve_target
  validate_header "$TARGET"

  local stamp dir backup hash size
  stamp="$(date '+%Y%m%d_%H%M%S_%N')"
  dir="$BACKUP_ROOT/before_change_$stamp"
  backup="$dir/MfbC01.smd"
  hash="$(sha256_file "$TARGET")"
  size="$(file_size "$TARGET")"

  mkdir -p "$dir"
  cp -a -- "$TARGET" "$backup"

  [[ "$(sha256_file "$backup")" == "$hash" ]] || die "Backup SHA verification failed."
  [[ "$(file_size "$backup")" == "$size" ]] || die "Backup size verification failed."

  {
    printf 'created_at=%s\n' "$(date --iso-8601=seconds)"
    printf 'client_target=%s\n' "$TARGET"
    printf 'backup=%s\n' "$backup"
    printf 'size=%s\n' "$size"
    printf 'sha256=%s\n' "$hash"
  } > "$dir/manifest.txt"

  mkdir -p "$BACKUP_ROOT"
  ln -sfn -- "$dir" "$BACKUP_ROOT/latest"

  BACKUP_DIR="$dir"
  BACKUP_FILE="$backup"
  BACKUP_SHA="$hash"
  BACKUP_SIZE="$size"
}

status() {
  resolve_target
  validate_header "$TARGET"

  local hash size
  hash="$(sha256_file "$TARGET")"
  size="$(file_size "$TARGET")"

  echo "PT_C01_HIGH_SMOOTH_RUNTIME_V1"
  echo "TARGET=$TARGET"
  echo "TARGET_SIZE=$size"
  echo "TARGET_SHA256=$hash"

  if [[ "$hash" == "$PRISTINE_SHA256" && "$size" == "$PRISTINE_SIZE" ]]; then
    echo "TARGET_STATE=PRISTINE"
  elif [[ "$hash" == "$APPROVED_SHA256" && "$size" == "$APPROVED_SIZE" ]]; then
    echo "TARGET_STATE=HIGH_SMOOTH_V1_INSTALLED"
  elif known_hash "$hash"; then
    echo "TARGET_STATE=KNOWN_MANAGED_BUILD"
  else
    echo "TARGET_STATE=UNKNOWN"
  fi

  if [[ -f "$BUILD" ]]; then
    echo "BUILD_SIZE=$(file_size "$BUILD")"
    echo "BUILD_SHA256=$(sha256_file "$BUILD")"
  else
    echo "BUILD=MISSING"
  fi

  if [[ -L "$BACKUP_ROOT/latest" || -d "$BACKUP_ROOT/latest" ]]; then
    echo "LATEST_BACKUP=$(readlink -f "$BACKUP_ROOT/latest")"
  else
    echo "LATEST_BACKUP=NONE"
  fi
}

install_build() {
  validate_build
  resolve_target
  validate_header "$TARGET"

  local current_hash
  current_hash="$(sha256_file "$TARGET")"

  [[ "$current_hash" != "$APPROVED_SHA256" ]] || \
    die "Approved HIGH smooth build is already installed."

  known_hash "$current_hash" || \
    die "Current client SMD is unknown/unmanaged. Refusing to overwrite."

  backup_current

  local tmp
  tmp="$(dirname "$TARGET")/.MfbC01.smd.high_smooth_v1.tmp.$$"
  cp -- "$BUILD" "$tmp"
  chmod --reference="$TARGET" "$tmp" 2>/dev/null || true

  if [[ "$(sha256_file "$tmp")" != "$APPROVED_SHA256" ]] || \
     [[ "$(file_size "$tmp")" != "$APPROVED_SIZE" ]]; then
    rm -f -- "$tmp"
    die "Temporary replacement verification failed."
  fi

  mv -f -- "$tmp" "$TARGET"

  if [[ "$(sha256_file "$TARGET")" != "$APPROVED_SHA256" ]] || \
     [[ "$(file_size "$TARGET")" != "$APPROVED_SIZE" ]]; then
    cp -a -- "$BACKUP_FILE" "$TARGET"
    die "HIGH smooth install failed; previous client state restored."
  fi

  {
    printf 'replacement=%s\n' "$BUILD"
    printf 'replacement_size=%s\n' "$APPROVED_SIZE"
    printf 'replacement_sha256=%s\n' "$APPROVED_SHA256"
  } >> "$BACKUP_DIR/manifest.txt"

  echo "INSTALL_HIGH_SMOOTH_CHECK=PASS"
  echo "BACKUP_DIR=$BACKUP_DIR"
  echo "BACKED_UP_SIZE=$BACKUP_SIZE"
  echo "BACKED_UP_SHA256=$BACKUP_SHA"
  echo "INSTALLED_SIZE=$APPROVED_SIZE"
  echo "INSTALLED_SHA256=$APPROVED_SHA256"
  echo "Only MfbC01.smd was modified."
}

restore_previous() {
  resolve_target

  [[ -L "$BACKUP_ROOT/latest" || -d "$BACKUP_ROOT/latest" ]] || \
    die "No HIGH smooth backup exists."

  local source_dir source_file source_hash source_size
  source_dir="$(readlink -f "$BACKUP_ROOT/latest")"
  source_file="$source_dir/MfbC01.smd"
  validate_header "$source_file"

  source_hash="$(sha256_file "$source_file")"
  source_size="$(file_size "$source_file")"

  backup_current
  local reverse="$BACKUP_DIR"

  cp -a -- "$source_file" "$TARGET"

  if [[ "$(sha256_file "$TARGET")" != "$source_hash" ]] || \
     [[ "$(file_size "$TARGET")" != "$source_size" ]]; then
    cp -a -- "$reverse/MfbC01.smd" "$TARGET"
    die "Restore verification failed; previous state restored."
  fi

  echo "RESTORE_PREVIOUS_CHECK=PASS"
  echo "RESTORED_FROM=$source_dir"
  echo "RESTORED_SIZE=$source_size"
  echo "RESTORED_SHA256=$source_hash"
  echo "REVERSE_BACKUP=$reverse"
}

restore_original() {
  resolve_target

  local pristine="$ROOT/MfbC01.smd"
  [[ -f "$pristine" ]] || die "Pristine source missing: $pristine"
  [[ "$(file_size "$pristine")" == "$PRISTINE_SIZE" ]] || die "Pristine source size mismatch."
  [[ "$(sha256_file "$pristine")" == "$PRISTINE_SHA256" ]] || die "Pristine source SHA mismatch."

  local current_hash current_size
  current_hash="$(sha256_file "$TARGET")"
  current_size="$(file_size "$TARGET")"

  if [[ "$current_hash" == "$PRISTINE_SHA256" && "$current_size" == "$PRISTINE_SIZE" ]]; then
    echo "Client is already pristine."
    echo "RESTORE_ORIGINAL_CHECK=PASS"
    return 0
  fi

  backup_current
  local reverse="$BACKUP_DIR"

  cp -a -- "$pristine" "$TARGET"

  if [[ "$(sha256_file "$TARGET")" != "$PRISTINE_SHA256" ]] || \
     [[ "$(file_size "$TARGET")" != "$PRISTINE_SIZE" ]]; then
    cp -a -- "$reverse/MfbC01.smd" "$TARGET"
    die "Original restore verification failed; previous state restored."
  fi

  echo "RESTORE_ORIGINAL_CHECK=PASS"
  echo "SIZE=$PRISTINE_SIZE"
  echo "SHA256=$PRISTINE_SHA256"
  echo "REVERSE_BACKUP=$reverse"
}

case "${1:-}" in
  status) status ;;
  install) install_build ;;
  restore-previous) restore_previous ;;
  restore-original|restore) restore_original ;;
  *)
    cat <<'EOF'
Usage:
  ./sh/pt_c01_high_smooth_runtime_v1.sh status
  ./sh/pt_c01_high_smooth_runtime_v1.sh install
  ./sh/pt_c01_high_smooth_runtime_v1.sh restore-previous
  ./sh/pt_c01_high_smooth_runtime_v1.sh restore-original

Approved HIGH smooth build:
  build: pt_build/C01_topology/MfbC01_high_smooth_patch_v1.smd
  size:  294021
  sha:   b1678aced23e84f560e65be56a2c11b5f35980228ee9afe32548c1424197ac23
EOF
    exit 2
    ;;
esac

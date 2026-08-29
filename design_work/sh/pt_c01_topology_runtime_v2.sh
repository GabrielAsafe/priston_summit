#!/usr/bin/env bash
set -euo pipefail

# Summit Tale / Priestess C01 - topology runtime gate V2
#
# Approved experimental build:
#   MsmC01 connected 4-face center subdivision
#   V 125 -> 129
#   F 238 -> 246
#   TexLink 238 -> 246
#   size 290997
#   sha256 30b8352d18074fe82982b67b7e670934505d182751ec7529284e614ae5b132d9
#
# This tool is separate from the SAFE XYZ workflow.
# Every write to the client is preceded by a verified backup.

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"

CLIENT_ROOT="${PT_CLIENT_ROOT:-/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
BUILD="${PT_TOPOLOGY_BUILD:-$ROOT/pt_build/C01_topology/MfbC01_patch_v2.smd}"
BACKUP_ROOT="${PT_TOPOLOGY_BACKUP_ROOT:-$ROOT/client_backups/C01_topology_v2}"

PRISTINE_SHA256="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
PRISTINE_SIZE="290229"

APPROVED_SHA256="30b8352d18074fe82982b67b7e670934505d182751ec7529284e614ae5b132d9"
APPROVED_SIZE="290997"

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
    printf 'Found %d target candidate(s):\n' "${#targets[@]}" >&2
    printf '  %s\n' "${targets[@]:-}" >&2
    die "Expected exactly one client MfbC01.smd."
  fi
  TARGET="${targets[0]}"
}

known_hash() {
  local h="$1"
  [[ "$h" == "$PRISTINE_SHA256" ]] && return 0
  [[ "$h" == "$APPROVED_SHA256" ]] && return 0

  if [[ -d "$ROOT/client_backups" ]]; then
    grep -Rhs --include='manifest.txt' \
      -E '^(replacement_sha256|sha256|original_sha256)=' \
      "$ROOT/client_backups" 2>/dev/null \
      | cut -d= -f2- \
      | grep -Fxq -- "$h" && return 0
  fi

  if [[ -f "$ROOT/pt_build/C01/history_v2.tsv" ]]; then
    awk -F'\t' -v h="$h" 'NR>1 && $3==h {found=1} END{exit !found}' \
      "$ROOT/pt_build/C01/history_v2.tsv" && return 0
  fi

  return 1
}

validate_build() {
  validate_header "$BUILD"
  local s h
  s="$(file_size "$BUILD")"
  h="$(sha256_file "$BUILD")"
  [[ "$s" == "$APPROVED_SIZE" ]] || die "Build size mismatch: $s != $APPROVED_SIZE"
  [[ "$h" == "$APPROVED_SHA256" ]] || die "Build SHA mismatch: $h"
}

backup_current() {
  resolve_target
  validate_header "$TARGET"

  local stamp dir backup sha size
  stamp="$(date '+%Y%m%d_%H%M%S_%N')"
  dir="$BACKUP_ROOT/before_change_$stamp"
  backup="$dir/MfbC01.smd"
  sha="$(sha256_file "$TARGET")"
  size="$(file_size "$TARGET")"

  mkdir -p "$dir"
  cp -a -- "$TARGET" "$backup"

  [[ "$(sha256_file "$backup")" == "$sha" ]] || die "Backup hash verification failed."
  [[ "$(file_size "$backup")" == "$size" ]] || die "Backup size verification failed."

  {
    printf 'created_at=%s\n' "$(date --iso-8601=seconds)"
    printf 'client_target=%s\n' "$TARGET"
    printf 'backup=%s\n' "$backup"
    printf 'size=%s\n' "$size"
    printf 'sha256=%s\n' "$sha"
  } > "$dir/manifest.txt"

  mkdir -p "$BACKUP_ROOT"
  ln -sfn -- "$dir" "$BACKUP_ROOT/latest"

  BACKUP_DIR="$dir"
  BACKUP_FILE="$backup"
  BACKUP_SHA="$sha"
  BACKUP_SIZE="$size"
}

status() {
  resolve_target
  validate_header "$TARGET"

  local h s
  h="$(sha256_file "$TARGET")"
  s="$(file_size "$TARGET")"

  echo "PT_C01_TOPOLOGY_RUNTIME_V2"
  echo "TARGET=$TARGET"
  echo "TARGET_SIZE=$s"
  echo "TARGET_SHA256=$h"

  if [[ "$h" == "$PRISTINE_SHA256" && "$s" == "$PRISTINE_SIZE" ]]; then
    echo "TARGET_STATE=PRISTINE"
  elif [[ "$h" == "$APPROVED_SHA256" && "$s" == "$APPROVED_SIZE" ]]; then
    echo "TARGET_STATE=TOPOLOGY_V2_INSTALLED"
  elif known_hash "$h"; then
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
}

install_build() {
  validate_build
  resolve_target
  validate_header "$TARGET"

  local current_sha
  current_sha="$(sha256_file "$TARGET")"

  [[ "$current_sha" != "$APPROVED_SHA256" ]] || \
    die "Approved topology V2 build is already installed."
  known_hash "$current_sha" || \
    die "Current client SMD is unknown/unmanaged; refusing to overwrite."

  backup_current

  local tmp
  tmp="$(dirname "$TARGET")/.MfbC01.smd.topology_v2.tmp.$$"
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
    die "Install verification failed; previous client SMD restored."
  fi

  {
    printf 'replacement=%s\n' "$BUILD"
    printf 'replacement_size=%s\n' "$APPROVED_SIZE"
    printf 'replacement_sha256=%s\n' "$APPROVED_SHA256"
  } >> "$BACKUP_DIR/manifest.txt"

  echo "INSTALL_TOPOLOGY_V2_CHECK=PASS"
  echo "BACKUP_DIR=$BACKUP_DIR"
  echo "BACKED_UP_SIZE=$BACKUP_SIZE"
  echo "BACKED_UP_SHA256=$BACKUP_SHA"
  echo "INSTALLED_SIZE=$APPROVED_SIZE"
  echo "INSTALLED_SHA256=$APPROVED_SHA256"
}

restore_previous() {
  resolve_target

  [[ -L "$BACKUP_ROOT/latest" || -d "$BACKUP_ROOT/latest" ]] || \
    die "No topology V2 backup exists."

  local source_dir source_file source_sha source_size
  source_dir="$(readlink -f "$BACKUP_ROOT/latest")"
  source_file="$source_dir/MfbC01.smd"
  validate_header "$source_file"
  source_sha="$(sha256_file "$source_file")"
  source_size="$(file_size "$source_file")"

  backup_current
  local reverse="$BACKUP_DIR"

  cp -a -- "$source_file" "$TARGET"

  if [[ "$(sha256_file "$TARGET")" != "$source_sha" ]] || \
     [[ "$(file_size "$TARGET")" != "$source_size" ]]; then
    cp -a -- "$reverse/MfbC01.smd" "$TARGET"
    die "Restore verification failed; prior state restored."
  fi

  echo "RESTORE_PREVIOUS_CHECK=PASS"
  echo "RESTORED_FROM=$source_dir"
  echo "RESTORED_SIZE=$source_size"
  echo "RESTORED_SHA256=$source_sha"
  echo "REVERSE_BACKUP=$reverse"
}

restore_original() {
  resolve_target

  local pristine="$ROOT/MfbC01.smd"
  [[ -f "$pristine" ]] || die "Pristine source not found: $pristine"
  [[ "$(file_size "$pristine")" == "$PRISTINE_SIZE" ]] || die "Pristine source size mismatch."
  [[ "$(sha256_file "$pristine")" == "$PRISTINE_SHA256" ]] || die "Pristine source SHA mismatch."

  local current_sha current_size
  current_sha="$(sha256_file "$TARGET")"
  current_size="$(file_size "$TARGET")"

  if [[ "$current_sha" == "$PRISTINE_SHA256" && "$current_size" == "$PRISTINE_SIZE" ]]; then
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
    die "Original restore verification failed; prior state restored."
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
  ./sh/pt_c01_topology_runtime_v2.sh status
  ./sh/pt_c01_topology_runtime_v2.sh install
  ./sh/pt_c01_topology_runtime_v2.sh restore-previous
  ./sh/pt_c01_topology_runtime_v2.sh restore-original

Approved topology V2:
  build: pt_build/C01_topology/MfbC01_patch_v2.smd
  size:  290997
  sha:   30b8352d18074fe82982b67b7e670934505d182751ec7529284e614ae5b132d9
EOF
    exit 2
    ;;
esac

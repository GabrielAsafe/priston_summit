#!/usr/bin/env bash
set -euo pipefail

# Summit Tale / Priestess C01 - topology runtime gate V1
#
# Installs ONLY the currently approved controlled-topology build:
#   MsmC01 SPLIT_ONE_FACE_1_TO_3
#   126 vertices / 240 faces / 240 texlinks
#   file size 290421
#
# Every client modification is preceded by a verified backup.
#
# Commands:
#   status
#   install
#   restore-previous
#   restore-original
#
# This tool is deliberately separate from pt_c01_tool_v2.sh (SAFE XYZ path).

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

CLIENT_ROOT="${PT_CLIENT_ROOT:-/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
BUILD="${PT_TOPOLOGY_BUILD:-$ROOT/pt_build/C01_topology/MfbC01_split_v1.smd}"
BACKUP_ROOT="${PT_TOPOLOGY_BACKUP_ROOT:-$ROOT/client_backups/C01_topology}"

PRISTINE_SHA256="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
PRISTINE_SIZE="290229"

APPROVED_SHA256="3671e38d6b9ff0478ea8a1c1374ff0858df7f27968e0c6a45609951b83d25e14"
APPROVED_SIZE="290421"

die() {
  printf 'ERROR: %s\n' "$*" >&2
  exit 1
}

sha256_file() {
  sha256sum -- "$1" | awk '{print $1}'
}

size_file() {
  stat -c %s -- "$1"
}

resolve_target() {
  [[ -d "$CLIENT_ROOT" ]] || die "Client root not found: $CLIENT_ROOT"

  mapfile -t TARGETS < <(find "$CLIENT_ROOT" -type f -iname 'MfbC01.smd' -print)
  if (( ${#TARGETS[@]} != 1 )); then
    printf 'Found %d MfbC01.smd candidate(s):\n' "${#TARGETS[@]}" >&2
    printf '  %s\n' "${TARGETS[@]:-}" >&2
    die "Expected exactly one runtime MfbC01.smd."
  fi
  TARGET="${TARGETS[0]}"
}

validate_header() {
  local f="$1"
  [[ -f "$f" ]] || die "File not found: $f"
  local hdr
  hdr="$(head -c 23 -- "$f" || true)"
  [[ "$hdr" == "SMD Model data Ver 0.62" ]] || die "Unexpected SMD header: $f"
}

is_known_managed_hash() {
  local h="$1"

  [[ "$h" == "$PRISTINE_SHA256" ]] && return 0
  [[ "$h" == "$APPROVED_SHA256" ]] && return 0

  # Compatibility with previous SAFE installers.
  if [[ -d "$ROOT/client_backups" ]]; then
    if grep -Rhs --include='manifest.txt' \
      -E '^(replacement_sha256|sha256|original_sha256)=' \
      "$ROOT/client_backups" 2>/dev/null \
      | cut -d= -f2- \
      | grep -Fxq -- "$h"; then
      return 0
    fi
  fi

  # Compatibility with pt_c01_tool_v2 history.
  if [[ -f "$ROOT/pt_build/C01/history_v2.tsv" ]]; then
    if awk -F'\t' -v h="$h" 'NR>1 && $3==h {found=1} END{exit !found}' \
      "$ROOT/pt_build/C01/history_v2.tsv"; then
      return 0
    fi
  fi

  return 1
}

validate_approved_build() {
  validate_header "$BUILD"

  local size sha
  size="$(size_file "$BUILD")"
  sha="$(sha256_file "$BUILD")"

  [[ "$size" == "$APPROVED_SIZE" ]] || \
    die "Topology build size mismatch: $size != $APPROVED_SIZE"
  [[ "$sha" == "$APPROVED_SHA256" ]] || \
    die "Topology build SHA256 mismatch: $sha"

  echo "BUILD=$BUILD"
  echo "BUILD_SIZE=$size"
  echo "BUILD_SHA256=$sha"
}

backup_current() {
  resolve_target
  validate_header "$TARGET"

  local stamp sha size dir backup
  stamp="$(date '+%Y%m%d_%H%M%S_%N')"
  sha="$(sha256_file "$TARGET")"
  size="$(size_file "$TARGET")"
  dir="$BACKUP_ROOT/before_change_$stamp"
  backup="$dir/MfbC01.smd"

  mkdir -p "$dir"
  cp -a -- "$TARGET" "$backup"

  [[ "$(sha256_file "$backup")" == "$sha" ]] || \
    die "Backup SHA verification failed."
  [[ "$(size_file "$backup")" == "$size" ]] || \
    die "Backup size verification failed."

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

  local sha size
  sha="$(sha256_file "$TARGET")"
  size="$(size_file "$TARGET")"

  echo "PT_C01_TOPOLOGY_RUNTIME_V1"
  echo "TARGET=$TARGET"
  echo "TARGET_SIZE=$size"
  echo "TARGET_SHA256=$sha"

  if [[ "$sha" == "$PRISTINE_SHA256" && "$size" == "$PRISTINE_SIZE" ]]; then
    echo "TARGET_STATE=PRISTINE"
  elif [[ "$sha" == "$APPROVED_SHA256" && "$size" == "$APPROVED_SIZE" ]]; then
    echo "TARGET_STATE=TOPOLOGY_V1_INSTALLED"
  elif is_known_managed_hash "$sha"; then
    echo "TARGET_STATE=KNOWN_MANAGED_BUILD"
  else
    echo "TARGET_STATE=UNKNOWN"
  fi

  if [[ -f "$BUILD" ]]; then
    echo "BUILD_SIZE=$(size_file "$BUILD")"
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
  validate_approved_build
  resolve_target
  validate_header "$TARGET"

  local current_sha
  current_sha="$(sha256_file "$TARGET")"

  echo "TARGET=$TARGET"
  echo "TARGET_SIZE=$(size_file "$TARGET")"
  echo "TARGET_SHA256=$current_sha"

  [[ "$current_sha" != "$APPROVED_SHA256" ]] || \
    die "Approved topology build is already installed."

  is_known_managed_hash "$current_sha" || \
    die "Current client SMD is unknown/unmanaged. Refusing to overwrite."

  backup_current

  local tmp
  tmp="$(dirname "$TARGET")/.MfbC01.smd.topology_v1.tmp.$$"
  cp -- "$BUILD" "$tmp"
  chmod --reference="$TARGET" "$tmp" 2>/dev/null || true

  [[ "$(sha256_file "$tmp")" == "$APPROVED_SHA256" ]] || {
    rm -f -- "$tmp"
    die "Temporary replacement SHA verification failed."
  }
  [[ "$(size_file "$tmp")" == "$APPROVED_SIZE" ]] || {
    rm -f -- "$tmp"
    die "Temporary replacement size verification failed."
  }

  mv -f -- "$tmp" "$TARGET"

  if [[ "$(sha256_file "$TARGET")" != "$APPROVED_SHA256" ]] || \
     [[ "$(size_file "$TARGET")" != "$APPROVED_SIZE" ]]; then
    echo "Install verification failed; restoring previous file immediately." >&2
    cp -a -- "$BACKUP_FILE" "$TARGET"
    die "Topology install failed; previous client state restored."
  fi

  {
    printf 'replacement=%s\n' "$BUILD"
    printf 'replacement_size=%s\n' "$APPROVED_SIZE"
    printf 'replacement_sha256=%s\n' "$APPROVED_SHA256"
  } >> "$BACKUP_DIR/manifest.txt"

  echo
  echo "INSTALL_TOPOLOGY_CHECK=PASS"
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
    die "No topology runtime backup exists."

  local dir src sha size
  dir="$(readlink -f "$BACKUP_ROOT/latest")"
  src="$dir/MfbC01.smd"
  [[ -f "$src" ]] || die "Latest backup SMD missing: $src"
  validate_header "$src"

  sha="$(sha256_file "$src")"
  size="$(size_file "$src")"

  # Back up whatever is currently installed before restoring.
  backup_current
  local reverse_backup="$BACKUP_DIR"

  cp -a -- "$src" "$TARGET"

  if [[ "$(sha256_file "$TARGET")" != "$sha" ]] || \
     [[ "$(size_file "$TARGET")" != "$size" ]]; then
    cp -a -- "$reverse_backup/MfbC01.smd" "$TARGET"
    die "Restore verification failed; previous state put back."
  fi

  echo "RESTORE_PREVIOUS_CHECK=PASS"
  echo "RESTORED_FROM=$dir"
  echo "RESTORED_SIZE=$size"
  echo "RESTORED_SHA256=$sha"
  echo "REVERSE_BACKUP=$reverse_backup"
}

find_pristine_backup() {
  local f

  # Prefer the canonical original in design_work if present and exact.
  if [[ -f "$ROOT/MfbC01.smd" ]] && \
     [[ "$(size_file "$ROOT/MfbC01.smd")" == "$PRISTINE_SIZE" ]] && \
     [[ "$(sha256_file "$ROOT/MfbC01.smd")" == "$PRISTINE_SHA256" ]]; then
    printf '%s\n' "$ROOT/MfbC01.smd"
    return 0
  fi

  if [[ -d "$ROOT/client_backups" ]]; then
    while IFS= read -r f; do
      [[ -f "$f" ]] || continue
      if [[ "$(size_file "$f")" == "$PRISTINE_SIZE" ]] && \
         [[ "$(sha256_file "$f")" == "$PRISTINE_SHA256" ]]; then
        printf '%s\n' "$f"
        return 0
      fi
    done < <(find "$ROOT/client_backups" -type f -name 'MfbC01.smd' -print | sort)
  fi

  return 1
}

restore_original() {
  resolve_target

  local current_sha current_size
  current_sha="$(sha256_file "$TARGET")"
  current_size="$(size_file "$TARGET")"

  if [[ "$current_sha" == "$PRISTINE_SHA256" && "$current_size" == "$PRISTINE_SIZE" ]]; then
    echo "Client is already pristine."
    echo "RESTORE_ORIGINAL_CHECK=PASS"
    return 0
  fi

  local pristine
  pristine="$(find_pristine_backup || true)"
  [[ -n "$pristine" ]] || die "Verified pristine MfbC01.smd not found."

  # Every modification receives a backup.
  backup_current
  local reverse_backup="$BACKUP_DIR"

  cp -a -- "$pristine" "$TARGET"

  if [[ "$(sha256_file "$TARGET")" != "$PRISTINE_SHA256" ]] || \
     [[ "$(size_file "$TARGET")" != "$PRISTINE_SIZE" ]]; then
    cp -a -- "$reverse_backup/MfbC01.smd" "$TARGET"
    die "Original restore verification failed; previous state restored."
  fi

  echo "RESTORE_ORIGINAL_CHECK=PASS"
  echo "SOURCE=$pristine"
  echo "SHA256=$PRISTINE_SHA256"
  echo "SIZE=$PRISTINE_SIZE"
  echo "REVERSE_BACKUP=$reverse_backup"
}

case "${1:-}" in
  status)
    status
    ;;
  install)
    install_build
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
  ./pt_c01_topology_runtime_v1.sh status
  ./pt_c01_topology_runtime_v1.sh install
  ./pt_c01_topology_runtime_v1.sh restore-previous
  ./pt_c01_topology_runtime_v1.sh restore-original

Approved build:
  pt_build/C01_topology/MfbC01_split_v1.smd
  size   290421
  sha256 3671e38d6b9ff0478ea8a1c1374ff0858df7f27968e0c6a45609951b83d25e14

This is the experimental topology runtime gate.
It does not alter the SAFE XYZ tool.
EOF
    exit 2
    ;;
esac

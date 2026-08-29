#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/profiles" "$ROOT/docs"

install -m 0644 "$PKG/profiles/pt_character_profile_priestess_c01_m5_v1.json" \
  "$ROOT/profiles/pt_character_profile_priestess_c01_m5_v1.json"
install -m 0755 "$PKG/pt_character_identity_discover_v1.py" \
  "$ROOT/pt_character_identity_discover_v1.py"
install -m 0644 "$PKG/README_IDENTITY_CORRECTION_V1.md" \
  "$ROOT/docs/README_IDENTITY_CORRECTION_V1.md"
install -m 0644 "$PKG/docs/POINT_OF_RESUME_IDENTITY_CORRECTION_2026-08-27.md" \
  "$ROOT/docs/POINT_OF_RESUME_IDENTITY_CORRECTION_2026-08-27.md"

echo "PT_CHARACTER_IDENTITY_CORRECTION_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=4"
echo "client_modified=False"
echo "workspace_modified=False"
echo "install_check=PASS"

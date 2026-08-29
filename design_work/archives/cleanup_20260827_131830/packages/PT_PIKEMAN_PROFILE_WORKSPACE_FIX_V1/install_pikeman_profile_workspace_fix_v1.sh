#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/docs"

install -m 0755 "$PKG/pt_pikeman_profile_fix_v1.py" \
  "$ROOT/pt_pikeman_profile_fix_v1.py"
install -m 0755 "$PKG/pt_character_workspace_init_v1.py" \
  "$ROOT/pt_character_workspace_init_v1.py"
install -m 0644 "$PKG/README_PIKEMAN_PROFILE_WORKSPACE_FIX_V1.md" \
  "$ROOT/docs/README_PIKEMAN_PROFILE_WORKSPACE_FIX_V1.md"
install -m 0644 "$PKG/docs/POINT_OF_RESUME_REAL_PIKEMAN_2026-08-27.md" \
  "$ROOT/docs/POINT_OF_RESUME_REAL_PIKEMAN_2026-08-27.md"

echo "PT_PIKEMAN_PROFILE_WORKSPACE_FIX_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=4"
echo "client_modified=False"
echo "install_check=PASS"

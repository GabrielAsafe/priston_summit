#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/profiles" "$ROOT/docs/history"

if [[ -f "$ROOT/pt_character_asset_v1.py" ]]; then
  cp -a "$ROOT/pt_character_asset_v1.py" \
    "$ROOT/history_py/pt_character_asset_v1.py" 2>/dev/null || true
fi

install -m 0755 "$PKG/pt_character_asset_v1_1.py" \
  "$ROOT/pt_character_asset_v1_1.py"
install -m 0644 "$PKG/profiles/pt_character_profile_pikeman_c01_m5_v2.json" \
  "$ROOT/profiles/pt_character_profile_pikeman_c01_m5_v2.json"
install -m 0644 "$PKG/docs/Summit_Tale_Blender_Tools_Source_of_Truth_v13_1_2026-08-27.md" \
  "$ROOT/docs/Summit_Tale_Blender_Tools_Source_of_Truth_v13_1_2026-08-27.md"
install -m 0644 "$PKG/docs/POINT_OF_RESUME_2026-08-27.md" \
  "$ROOT/docs/POINT_OF_RESUME_2026-08-27.md"
install -m 0644 "$PKG/README_CHARACTER_ASSET_V1_1.md" \
  "$ROOT/docs/README_CHARACTER_ASSET_V1_1.md"

echo "PT_CHARACTER_ASSET_V1_1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=5"
echo "install_check=PASS"

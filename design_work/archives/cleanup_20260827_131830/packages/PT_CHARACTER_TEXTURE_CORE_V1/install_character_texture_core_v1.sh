#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/profiles" "$ROOT/docs" "$ROOT/history_py"

for f in \
  pt_character_asset_v1_1.py \
  summit_workspace_asset_bridge_v1_1.py
do
  if [[ -f "$ROOT/$f" ]]; then
    cp -a "$ROOT/$f" "$ROOT/history_py/$f" 2>/dev/null || true
  fi
done

install -m 0755 "$PKG/pt_character_texture_v1.py" \
  "$ROOT/pt_character_texture_v1.py"
install -m 0644 "$PKG/pt_blender_bind_editable_texture_v1.py" \
  "$ROOT/pt_blender_bind_editable_texture_v1.py"
install -m 0755 "$PKG/summit_workspace_asset_bridge_v1_2.py" \
  "$ROOT/summit_workspace_asset_bridge_v1_2.py"
install -m 0755 "$PKG/pt_character_asset_v1_2.py" \
  "$ROOT/pt_character_asset_v1_2.py"
install -m 0644 "$PKG/profiles/pt_character_profile_pikeman_c01_m5_v2.json" \
  "$ROOT/profiles/pt_character_profile_pikeman_c01_m5_v2.json"
install -m 0644 "$PKG/docs/TEXTURE_WORKFLOW_ADDENDUM_V1_2026-08-27.md" \
  "$ROOT/docs/TEXTURE_WORKFLOW_ADDENDUM_V1_2026-08-27.md"
install -m 0644 "$PKG/docs/POINT_OF_RESUME_2026-08-27.md" \
  "$ROOT/docs/POINT_OF_RESUME_2026-08-27.md"
install -m 0644 "$PKG/README_CHARACTER_TEXTURE_CORE_V1.md" \
  "$ROOT/docs/README_CHARACTER_TEXTURE_CORE_V1.md"

echo "PT_CHARACTER_TEXTURE_CORE_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=8"
echo "install_check=PASS"

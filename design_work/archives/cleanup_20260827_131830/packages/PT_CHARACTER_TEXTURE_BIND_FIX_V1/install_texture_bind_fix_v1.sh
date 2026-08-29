#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/history_py" "$ROOT/docs"

for f in \
  pt_blender_bind_editable_texture_v1.py \
  pt_character_texture_v1.py \
  pt_character_asset_v1_2.py
do
  if [[ -f "$ROOT/$f" ]]; then
    cp -a "$ROOT/$f" "$ROOT/history_py/$f" 2>/dev/null || true
  fi
done

install -m 0644 "$PKG/pt_blender_bind_editable_texture_v2.py" \
  "$ROOT/pt_blender_bind_editable_texture_v2.py"
install -m 0644 "$PKG/pt_blender_material_texture_probe_v1.py" \
  "$ROOT/pt_blender_material_texture_probe_v1.py"
install -m 0755 "$PKG/pt_character_texture_v1_1.py" \
  "$ROOT/pt_character_texture_v1_1.py"
install -m 0755 "$PKG/pt_character_asset_v1_3.py" \
  "$ROOT/pt_character_asset_v1_3.py"
install -m 0644 "$PKG/TEXTURE_BIND_FIX_V1.md" \
  "$ROOT/docs/TEXTURE_BIND_FIX_V1.md"

echo "PT_CHARACTER_TEXTURE_BIND_FIX_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=5"
echo "install_check=PASS"

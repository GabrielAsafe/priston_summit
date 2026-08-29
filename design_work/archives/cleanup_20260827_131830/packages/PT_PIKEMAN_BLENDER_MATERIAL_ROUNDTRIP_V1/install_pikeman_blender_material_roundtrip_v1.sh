#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "$ROOT/docs"
install -m 0644 "$PKG/pt_blender_summit_material_bind_v1.py" "$ROOT/pt_blender_summit_material_bind_v1.py"
install -m 0644 "$PKG/pt_blender_summit_material_export_v1.py" "$ROOT/pt_blender_summit_material_export_v1.py"
install -m 0755 "$PKG/pt_smd_material_build_from_blender_v1.py" "$ROOT/pt_smd_material_build_from_blender_v1.py"
install -m 0644 "$PKG/docs/PIKEMAN_BLENDER_MATERIAL_ROUNDTRIP_V1_2026-08-27.md" "$ROOT/docs/PIKEMAN_BLENDER_MATERIAL_ROUNDTRIP_V1_2026-08-27.md"
echo "PT_PIKEMAN_BLENDER_MATERIAL_ROUNDTRIP_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=4"
echo "client_modified=False"
echo "install_check=PASS"

#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

install -m 0644 "$PKG/pt_free_mesh_validate_multi_v1.py" \
  "$ROOT/pt_free_mesh_validate_multi_v1.py"
install -m 0644 "$PKG/pt_blender_to_smd_free_mesh_v4_multi.py" \
  "$ROOT/pt_blender_to_smd_free_mesh_v4_multi.py"
install -m 0755 "$PKG/pt_character_multi_object_lod_pipeline_v1.py" \
  "$ROOT/pt_character_multi_object_lod_pipeline_v1.py"

echo "PT_MULTI_OBJECT_LOD_CORE_INSTALL_V1"
echo "ROOT=$ROOT"
echo "installed=3"
echo "install_check=PASS"

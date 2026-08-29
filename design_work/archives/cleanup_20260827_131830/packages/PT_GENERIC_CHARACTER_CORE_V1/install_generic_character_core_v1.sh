#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/profiles" "$ROOT/history_py" "$ROOT/sh/history"

install -m 0644 "$PKG/pt_rigid_skin_normalize_copy_v2.py" "$ROOT/pt_rigid_skin_normalize_copy_v2.py"
install -m 0644 "$PKG/pt_rigid_skin_force_resolve_test_v2.py" "$ROOT/pt_rigid_skin_force_resolve_test_v2.py"
install -m 0644 "$PKG/pt_free_mesh_validate_v2.py" "$ROOT/pt_free_mesh_validate_v2.py"
install -m 0644 "$PKG/pt_blender_to_smd_free_mesh_v3.py" "$ROOT/pt_blender_to_smd_free_mesh_v3.py"
install -m 0755 "$PKG/pt_character_artist_pipeline_v2.py" "$ROOT/pt_character_artist_pipeline_v2.py"
install -m 0755 "$PKG/pt_smd_object_capability_audit_v1.py" "$ROOT/pt_smd_object_capability_audit_v1.py"
install -m 0644 "$PKG/pt_character_profile_pikeman_c01_m5_v1.json" "$ROOT/profiles/pt_character_profile_pikeman_c01_m5_v1.json"

echo "PT_GENERIC_CHARACTER_CORE_INSTALL_V1"
echo "ROOT=$ROOT"
echo "installed=7"
echo "install_check=PASS"

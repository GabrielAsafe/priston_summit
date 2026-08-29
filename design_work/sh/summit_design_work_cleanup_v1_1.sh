#!/usr/bin/env bash
set -euo pipefail

ROOT="${ROOT:-$HOME/Área de Trabalho/summit/design_work}"
MODE="${1:---dry-run}"

if [[ "$MODE" != "--dry-run" && "$MODE" != "--apply" ]]; then
  echo "usage: $0 [--dry-run|--apply]"
  exit 2
fi

cd "$ROOT"

STAMP="$(date +%Y%m%d_%H%M%S)"
ARCHIVE_ROOT="archives/cleanup_${STAMP}"
HISTORY="history_py"
AUDITS="logs/audits"
PKGS="${ARCHIVE_ROOT}/packages"
LEGACY="${ARCHIVE_ROOT}/legacy"
CACHE="${ARCHIVE_ROOT}/cache"
MOVELOG="logs/cleanup_${STAMP}.tsv"

declare -a MOVES=()

unique_destination() {
  local dst="$1"
  if [[ ! -e "$dst" ]]; then
    printf '%s' "$dst"
    return
  fi
  local dir base stem ext candidate n
  dir="$(dirname "$dst")"
  base="$(basename "$dst")"
  if [[ "$base" == *.* ]]; then
    stem="${base%.*}"
    ext=".${base##*.}"
  else
    stem="$base"
    ext=""
  fi
  candidate="${dir}/${stem}__cleanup_${STAMP}${ext}"
  n=1
  while [[ -e "$candidate" ]]; do
    candidate="${dir}/${stem}__cleanup_${STAMP}_${n}${ext}"
    ((n++))
  done
  printf '%s' "$candidate"
}

plan_move() {
  local src="$1"
  local dst="$2"
  [[ -e "$src" ]] || return 0
  dst="$(unique_destination "$dst")"
  MOVES+=("$src"$'\t'"$dst")
}

# Old Python generations -> history_py.
for f in \
  pt_blender_bind_editable_texture_v1.py \
  pt_blender_material_texture_probe_v1.py \
  pt_blender_selected_material_usage_probe_v1.py \
  pt_blender_summit_material_bind_v1.py \
  pt_blender_summit_material_bind_v2.py \
  pt_blender_summit_material_bind_v3.py \
  pt_blender_summit_material_export_v1.py \
  pt_blender_summit_material_export_v2.py \
  pt_blender_summit_material_export_v3.py \
  pt_blender_summit_material_set_v3.py \
  pt_blender_to_smd_free_mesh_v2_3.py \
  pt_blender_to_smd_free_mesh_v3.py \
  pt_character_artist_pipeline_v2.py \
  pt_character_asset_v1.py \
  pt_character_asset_v1_1.py \
  pt_character_asset_v1_2.py \
  pt_character_asset_v1_3.py \
  pt_character_multi_object_lod_pipeline_v1.py \
  pt_character_texture_v1.py \
  pt_character_texture_v1_1.py \
  pt_free_mesh_validate_v1_4.py \
  pt_free_mesh_validate_multi_v1.py \
  pt_pikeman_armor_family_v1.py \
  pt_pikeman_c03_inx_selector_audit_v1.py \
  pt_pikeman_geometry_gate_edit_v1.py \
  pt_pikeman_geometry_gate_edit_v1_1.py \
  pt_pikeman_geometry_reimport_compare_v1_1.py \
  pt_pikeman_geometry_twoside_ab_build_v1.py \
  pt_pikeman_geometry_vertex_build_v1.py \
  pt_pikeman_geometry_vertex_build_v1_1.py \
  pt_pikeman_high_witness_edit_v1.py \
  pt_pikeman_highlod_twoside_witness_build_v1.py \
  pt_pikeman_profile_fix_v1.py \
  pt_pikeman_selfillum_restore_build_v1.py \
  pt_pikeman_smd_selfillum_build_v1.py \
  pt_rigid_skin_force_resolve_test_v1.py \
  pt_rigid_skin_normalize_copy_v1.py \
  pt_smd_blink_color_control_source_audit_v1.py \
  pt_smd_blink_color_control_source_audit_v1_1.py \
  pt_smd_diffuse_color_path_audit_v2.py \
  pt_smd_diffuse_render_source_audit_v1.py \
  pt_smd_material_build_from_blender_v1.py \
  pt_smd_material_build_from_blender_v2.py \
  pt_smd_material_build_from_blender_v3.py \
  pt_smd_material_render_source_audit_v1.py \
  summit_roundtrip_restore_v2.py \
  summit_roundtrip_restore_v3.py \
  summit_workspace_asset_bridge_v1.py \
  summit_workspace_asset_bridge_v1_1.py
do
  plan_move "$f" "$HISTORY/$f"
done

# Completed package dirs/zips -> timestamped archive.
shopt -s nullglob
for f in PT_*; do
  plan_move "$f" "$PKGS/$f"
done
shopt -u nullglob

# Known misnamed legacy workspace (Priestess history) -> archive.
plan_move "blender_workspace_pikeman" "$LEGACY/blender_workspace_pikeman"

# Large audit outputs -> logs.
plan_move "auditoria_v2.txt" "$AUDITS/auditoria_v2.txt"
plan_move "saida_auditoria_equipamento.txt" "$AUDITS/saida_auditoria_equipamento.txt"

# Root caches -> archive, not delete.
plan_move "__pycache__" "$CACHE/__pycache__"
plan_move "_pt_smd_texture_cache" "$CACHE/_pt_smd_texture_cache"

echo "PT_DESIGN_WORK_CLEANUP_V1_1"
echo "ROOT=$ROOT"
echo "MODE=$MODE"
echo "planned_moves=${#MOVES[@]}"
echo
printf "%-70s -> %s\n" "SOURCE" "DESTINATION"
printf "%-70s    %s\n" "------" "-----------"
for row in "${MOVES[@]}"; do
  printf "%-70s -> %s\n" "${row%%$'\t'*}" "${row#*$'\t'}"
done

if [[ "$MODE" == "--dry-run" ]]; then
  echo
  echo "filesystem_modified=False"
  echo "cleanup_check=DRY_RUN_PASS"
  exit 0
fi

# Preflight every source/destination before moving anything.
for row in "${MOVES[@]}"; do
  src="${row%%$'\t'*}"
  dst="${row#*$'\t'}"
  [[ -e "$src" ]] || { echo "cleanup_check=FAIL: source disappeared: $src" >&2; exit 1; }
  [[ ! -e "$dst" ]] || { echo "cleanup_check=FAIL: destination appeared: $dst" >&2; exit 1; }
done

mkdir -p "$HISTORY" "$AUDITS" "$PKGS" "$LEGACY" "$CACHE" logs
: > "$MOVELOG"
printf "source\tdestination\n" >> "$MOVELOG"

for row in "${MOVES[@]}"; do
  src="${row%%$'\t'*}"
  dst="${row#*$'\t'}"
  mkdir -p "$(dirname "$dst")"
  mv -- "$src" "$dst"
  printf "%s\t%s\n" "$src" "$dst" >> "$MOVELOG"
done

echo
echo "move_log=$ROOT/$MOVELOG"
echo "deleted_files=0"
echo "cleanup_check=PASS"

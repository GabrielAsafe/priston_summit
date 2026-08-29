#!/usr/bin/env bash
set -euo pipefail

# Summit Tale design_work cleanup V1
#
# DRY-RUN by default.
# No project file is deleted.
# Known temporary/legacy material is MOVED into history/archive locations.
# Every moved file is recorded with size and SHA256.
#
# Usage:
#   ./sh/pt_workspace_cleanup_v1.sh
#   ./sh/pt_workspace_cleanup_v1.sh --apply
#   ./sh/pt_workspace_cleanup_v1.sh --apply --full-legacy
#
# --full-legacy also archives older experimental directories whose contents
# are not needed by the current artist/free-mesh/roundtrip core.

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
MODE="DRY_RUN"
FULL_LEGACY=0

PRISTINE_SHA="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"

while (( $# )); do
  case "$1" in
    --apply) MODE="APPLY"; shift ;;
    --dry-run) MODE="DRY_RUN"; shift ;;
    --full-legacy) FULL_LEGACY=1; shift ;;
    -h|--help)
      sed -n '1,24p' "$0"
      exit 0
      ;;
    *) echo "ERROR: unknown argument: $1" >&2; exit 2 ;;
  esac
done

die() { echo "ERROR: $*" >&2; exit 1; }
sha() { sha256sum -- "$1" | awk '{print $1}'; }
size() { stat -c %s -- "$1"; }

[[ -f "$ROOT/MfbC01.smd" ]] || die "Not a design_work root: MfbC01.smd missing"
[[ -f "$ROOT/m5.smb" ]] || die "Not a design_work root: m5.smb missing"
[[ -f "$ROOT/M5Bip.inx" ]] || die "Not a design_work root: M5Bip.inx missing"

ROOT_SHA="$(sha "$ROOT/MfbC01.smd")"
echo "PT_WORKSPACE_CLEANUP_V1"
echo "ROOT=$ROOT"
echo "MODE=$MODE"
echo "FULL_LEGACY=$FULL_LEGACY"
echo "ROOT_MfbC01_SHA256=$ROOT_SHA"
if [[ "$ROOT_SHA" == "$PRISTINE_SHA" ]]; then
  echo "ROOT_BASELINE=PRISTINE"
else
  echo "ROOT_BASELINE=NON_PRISTINE_WARNING"
fi

STAMP="$(date '+%Y%m%d_%H%M%S_%N')"
MANIFEST="$ROOT/logs/workspace_cleanup_${STAMP}.tsv"

declare -a OPS=()

queue_move() {
  local src="$1" dst="$2"
  [[ -e "$ROOT/$src" || -L "$ROOT/$src" ]] || return 0
  OPS+=("MOVE"$'\t'"$src"$'\t'"$dst")
}

queue_copy() {
  local src="$1" dst="$2"
  [[ -f "$ROOT/$src" ]] || return 0
  [[ ! -e "$ROOT/$dst" ]] || return 0
  OPS+=("COPY"$'\t'"$src"$'\t'"$dst")
}

# Documentation supplied with this cleanup package.
# README.md intentionally remains at project root.
queue_move "Summit_Tale_Blender_Tools_Source_of_Truth_v12_0_2026-08-26.md" \
           "docs/Summit_Tale_Blender_Tools_Source_of_Truth_v12_0_2026-08-26.md"
queue_move "TOOLS_AND_FILES_INDEX_v1.md" "docs/TOOLS_AND_FILES_INDEX_v1.md"
queue_move "PROJECT_LAYOUT_AFTER_CLEANUP.md" "docs/PROJECT_LAYOUT_AFTER_CLEANUP.md"

# Previous documentation.
queue_move "Summit_Tale_Blender_Tools_Source_of_Truth_v11_6_2026-08-26.md" \
           "docs/history/Summit_Tale_Blender_Tools_Source_of_Truth_v11_6_2026-08-26.md"
queue_move "Summit_Tale_Design_3D_Texturas_Blender_Source_of_Truth_v11_5_2026-08-23.md" \
           "docs/history/Summit_Tale_Design_3D_Texturas_Blender_Source_of_Truth_v11_5_2026-08-23.md"
queue_move "priston_tale_3d_source_unica.md" \
           "docs/history/priston_tale_3d_source_unica.md"

# Active pieces still trapped in the original roundtrip package.
# Copy them out before archiving the original package.
queue_copy "ROUNDTRIPAUTOMATICA/summit_prepare_blender_workspace.py" \
           "summit_prepare_blender_workspace.py"
queue_copy "ROUNDTRIPAUTOMATICA/summit_asset_manifest.json" \
           "summit_asset_manifest.json"
queue_copy "ROUNDTRIPAUTOMATICA/Summit_Tale_Asset_Catalog.xlsx" \
           "docs/reference/Summit_Tale_Asset_Catalog.xlsx"

# Old probes/research scripts superseded by current core.
for f in \
  pt_artist_triangulation_probe_v1.py \
  pt_blender_uv_storage_probe_v1.py \
  pt_blender_uv_storage_probe_v2.py \
  pt_rigid_skin_weight_probe_v1.py \
  pt_blender_to_smd_high_remodel_v2.py
do
  queue_move "$f" "history_py/$f"
done

# Root Blender backup files.
for f in "$ROOT"/*.blend1; do
  [[ -e "$f" ]] || continue
  bn="$(basename "$f")"
  queue_move "$bn" "blender/history/$bn"
done

# Old root SMD test builds; pristine MfbC01.smd is deliberately excluded.
for f in MfbC01_clean_noop.smd MfbC01_testv2.smd MfbC01_testv2_lods.smd; do
  queue_move "$f" "pt_build/archive/root_legacy/$f"
done

# Superseded pipeline wrapper.
queue_move "sh/pt_character_artist_pipeline_v1.sh" \
           "sh/history/pt_character_artist_pipeline_v1.sh"

# Package/extracted sources now superseded by promoted active scripts.
queue_move "ROUNDTRIPAUTOMATICA" \
           "archives/roundtrip_original/ROUNDTRIPAUTOMATICA"
queue_move "ROUNDTRIP_INTEGRATION_PATCH_V1" \
           "archives/integration_patch_v1/ROUNDTRIP_INTEGRATION_PATCH_V1"
queue_move "ROUNDTRIP_INTEGRATION_PATCH_V1.zip" \
           "archives/integration_patch_v1/ROUNDTRIP_INTEGRATION_PATCH_V1.zip"

# Cache only; preserved rather than deleted.
queue_move "__pycache__" "archives/cache/__pycache___${STAMP}"

if (( FULL_LEGACY )); then
  LEGACY_ROOT="archives/legacy_workspace_${STAMP}"
  for d in blender_pipeline out oldMD pike_c01 pike_tmbc01 pike_tmbc01_uvborder pike_tmbc01_uvmap pike_tmbc03; do
    queue_move "$d" "$LEGACY_ROOT/$d"
  done
fi

echo
echo "PLANNED_OPERATIONS=${#OPS[@]}"
for op in "${OPS[@]}"; do
  IFS=$'\t' read -r kind src dst <<<"$op"
  echo "$kind: $src -> $dst"
done

if [[ "$MODE" == "DRY_RUN" ]]; then
  echo
  echo "No files changed."
  echo "Run with --apply after reviewing the plan."
  echo "workspace_cleanup_check=DRY_RUN_PASS"
  exit 0
fi

mkdir -p "$ROOT/logs"
{
  printf 'kind\tsource\tdestination\tsize\tsha256\n'
} > "$MANIFEST"

record_file() {
  local kind="$1" src="$2" dst="$3"
  printf '%s\t%s\t%s\t%s\t%s\n' \
    "$kind" "$src" "$dst" "$(size "$src")" "$(sha "$src")" >> "$MANIFEST"
}

record_tree() {
  local kind="$1" src="$2" dst="$3"
  if [[ -f "$src" ]]; then
    record_file "$kind" "$src" "$dst"
    return
  fi
  while IFS= read -r -d '' f; do
    rel="${f#"$src"/}"
    record_file "$kind" "$f" "$dst/$rel"
  done < <(find "$src" -type f -print0)
}

verify_manifest_destinations() {
  local failed=0
  tail -n +2 "$MANIFEST" | while IFS=$'\t' read -r kind src dst expected_size expected_sha; do
    if [[ ! -f "$dst" ]]; then
      echo "VERIFY_MISSING: $dst" >&2
      failed=1
      continue
    fi
    got_size="$(size "$dst")"
    got_sha="$(sha "$dst")"
    if [[ "$got_size" != "$expected_size" || "$got_sha" != "$expected_sha" ]]; then
      echo "VERIFY_FAIL: $dst" >&2
      failed=1
    fi
  done
  return "$failed"
}

for op in "${OPS[@]}"; do
  IFS=$'\t' read -r kind relsrc reldst <<<"$op"
  src="$ROOT/$relsrc"
  dst="$ROOT/$reldst"

  [[ -e "$src" || -L "$src" ]] || continue

  if [[ "$kind" == "COPY" && -e "$dst" ]]; then
    echo "SKIP_COPY_ALREADY_EXISTS: $reldst"
    continue
  fi

  if [[ "$kind" == "MOVE" && -e "$dst" ]]; then
    die "destination already exists: $dst"
  fi

  mkdir -p "$(dirname "$dst")"
  record_tree "$kind" "$src" "$dst"

  if [[ "$kind" == "COPY" ]]; then
    cp -a -- "$src" "$dst"
  else
    mv -- "$src" "$dst"
  fi

  echo "$kind PASS: $relsrc -> $reldst"
done

# Verify every file recorded in manifest.
FAIL=0
while IFS=$'\t' read -r kind src dst expected_size expected_sha; do
  [[ "$kind" == "kind" ]] && continue
  if [[ ! -f "$dst" ]]; then
    echo "VERIFY_MISSING: $dst" >&2
    FAIL=1
    continue
  fi
  got_size="$(size "$dst")"
  got_sha="$(sha "$dst")"
  if [[ "$got_size" != "$expected_size" || "$got_sha" != "$expected_sha" ]]; then
    echo "VERIFY_FAIL: $dst" >&2
    FAIL=1
  fi
done < "$MANIFEST"

(( FAIL == 0 )) || die "cleanup verification failed; inspect $MANIFEST"

echo
echo "MANIFEST=$MANIFEST"
echo "No project file was deleted."
echo "workspace_cleanup_check=PASS"

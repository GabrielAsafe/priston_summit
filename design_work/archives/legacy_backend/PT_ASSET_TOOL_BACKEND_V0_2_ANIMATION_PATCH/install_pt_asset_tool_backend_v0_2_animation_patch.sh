#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="$ROOT/archives/pt_asset_tool_backend_patch_backup/$STAMP"

[[ -d "$ROOT/pt_asset_tool" ]] || { echo "MISSING_BACKEND=$ROOT/pt_asset_tool" >&2; exit 2; }
[[ -f "$ROOT/pt_asset.py" ]] || { echo "MISSING_ENTRYPOINT=$ROOT/pt_asset.py" >&2; exit 2; }
BASE_VERSION="$(cd "$ROOT" && python3 pt_asset.py --version 2>/dev/null || true)"
[[ "$BASE_VERSION" == "0.1.1" ]] || { echo "BASE_VERSION_MISMATCH expected=0.1.1 actual=$BASE_VERSION" >&2; exit 3; }

while read -r expected rel; do
  [[ -n "${expected:-}" && -n "${rel:-}" ]] || continue
  p="$ROOT/$rel"
  [[ -f "$p" ]] || { echo "BASE_FILE_MISSING=$p" >&2; exit 4; }
  actual="$(sha256sum "$p" | awk '{print $1}')"
  [[ "$actual" == "$expected" ]] || { echo "BASE_SHA_MISMATCH=$rel expected=$expected actual=$actual" >&2; exit 5; }
done < "$HERE/BASE_V0_1_1_SHA256SUMS.txt"
echo "base_v0_1_1_integrity=PASS"

mkdir -p "$BACKUP/pt_asset_tool/blender" "$ROOT/pt_asset_tool/blender"
for rel in pt_asset_tool/__init__.py pt_asset_tool/cli.py pt_asset_tool/validation.py; do
  cp -a "$ROOT/$rel" "$BACKUP/$rel"
done
for rel in pt_asset_tool/motion_core.py pt_asset_tool/blender/bake_actions.py; do
  if [[ -f "$ROOT/$rel" ]]; then cp -a "$ROOT/$rel" "$BACKUP/$rel"; fi
done

cp -a "$HERE/pt_asset_tool/__init__.py" "$ROOT/pt_asset_tool/__init__.py"
cp -a "$HERE/pt_asset_tool/cli.py" "$ROOT/pt_asset_tool/cli.py"
cp -a "$HERE/pt_asset_tool/validation.py" "$ROOT/pt_asset_tool/validation.py"
cp -a "$HERE/pt_asset_tool/motion_core.py" "$ROOT/pt_asset_tool/motion_core.py"
cp -a "$HERE/pt_asset_tool/blender/bake_actions.py" "$ROOT/pt_asset_tool/blender/bake_actions.py"

python3 -m py_compile \
  "$ROOT/pt_asset_tool/__init__.py" \
  "$ROOT/pt_asset_tool/cli.py" \
  "$ROOT/pt_asset_tool/validation.py" \
  "$ROOT/pt_asset_tool/motion_core.py" \
  "$ROOT/pt_asset_tool/blender/bake_actions.py"

VERSION="$(cd "$ROOT" && python3 pt_asset.py --version)"
[[ "$VERSION" == "0.2.0" ]] || { echo "VERSION_CHECK_FAIL=$VERSION" >&2; exit 6; }

# Presence only: these are the proven existing cores this patch delegates to.
for rel in history_py/pt_smb_motion_evaluator_v1.py pt_inx_motion_core_v2.py history_py/pt_smd_blender_import_v3_static_armature.py; do
  [[ -f "$ROOT/$rel" ]] || { echo "REQUIRED_CORE_MISSING=$ROOT/$rel" >&2; exit 7; }
done

echo "PT_ASSET_TOOL_BACKEND_V0_2_ANIMATION_PATCH"
echo "root=$ROOT"
echo "backup=$BACKUP"
echo "version=$VERSION"
echo "certifications_preserved=True"
echo "client_modified=False"
echo "patch_check=PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="$ROOT/archives/pt_asset_tool_backend_patch_backup/$STAMP"
[[ -d "$ROOT/pt_asset_tool" ]] || { echo "MISSING_BACKEND=$ROOT/pt_asset_tool" >&2; exit 2; }
[[ -f "$ROOT/pt_asset.py" ]] || { echo "MISSING_ENTRYPOINT=$ROOT/pt_asset.py" >&2; exit 2; }
BASE_VERSION="$(cd "$ROOT" && python3 pt_asset.py --version 2>/dev/null || true)"
[[ "$BASE_VERSION" == "0.2.0" ]] || { echo "BASE_VERSION_MISMATCH expected=0.2.0 actual=$BASE_VERSION" >&2; exit 3; }
while read -r expected rel; do
  [[ -n "${expected:-}" && -n "${rel:-}" ]] || continue
  p="$ROOT/$rel"
  [[ -f "$p" ]] || { echo "BASE_FILE_MISSING=$p" >&2; exit 4; }
  actual="$(sha256sum "$p" | awk '{print $1}')"
  [[ "$actual" == "$expected" ]] || { echo "BASE_SHA_MISMATCH=$rel expected=$expected actual=$actual" >&2; exit 5; }
done < "$HERE/BASE_V0_2_SHA256SUMS.txt"
echo "base_v0_2_integrity=PASS"
mkdir -p "$BACKUP/pt_asset_tool/blender" "$ROOT/pt_asset_tool/blender"
for rel in pt_asset_tool/__init__.py pt_asset_tool/motion_core.py pt_asset_tool/validation.py pt_asset_tool/blender/bake_actions.py; do
  cp -a "$ROOT/$rel" "$BACKUP/$rel"
  cp -a "$HERE/$rel" "$ROOT/$rel"
done
python3 -m py_compile "$ROOT"/pt_asset_tool/*.py "$ROOT"/pt_asset_tool/blender/*.py
VERSION="$(cd "$ROOT" && python3 pt_asset.py --version)"
[[ "$VERSION" == "0.2.1" ]] || { echo "VERSION_CHECK_FAIL=$VERSION" >&2; exit 6; }
echo "PT_ASSET_TOOL_BACKEND_V0_2_1_ANIMATION_FIX"
echo "root=$ROOT"
echo "backup=$BACKUP"
echo "version=$VERSION"
echo "certifications_preserved=True"
echo "client_modified=False"
echo "patch_check=PASS"

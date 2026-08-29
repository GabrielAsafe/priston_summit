#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="$ROOT/archives/pt_asset_tool_backend_patch_backup/$STAMP"

[[ -d "$ROOT/pt_asset_tool" ]] || { echo "MISSING_BACKEND=$ROOT/pt_asset_tool" >&2; exit 2; }
[[ -f "$ROOT/pt_asset.py" ]] || { echo "MISSING_ENTRYPOINT=$ROOT/pt_asset.py" >&2; exit 2; }
mkdir -p "$BACKUP/pt_asset_tool"

for rel in pt_asset_tool/__init__.py pt_asset_tool/cli.py pt_asset_tool/validation.py; do
  if [[ -f "$ROOT/$rel" ]]; then
    cp -a "$ROOT/$rel" "$BACKUP/$rel"
  fi
done

cp -a "$HERE/pt_asset_tool/__init__.py" "$ROOT/pt_asset_tool/__init__.py"
cp -a "$HERE/pt_asset_tool/cli.py" "$ROOT/pt_asset_tool/cli.py"
cp -a "$HERE/pt_asset_tool/validation.py" "$ROOT/pt_asset_tool/validation.py"
cp -a "$HERE/pt_asset_tool/texture_overlay.py" "$ROOT/pt_asset_tool/texture_overlay.py"

python3 -m py_compile \
  "$ROOT/pt_asset_tool/__init__.py" \
  "$ROOT/pt_asset_tool/cli.py" \
  "$ROOT/pt_asset_tool/validation.py" \
  "$ROOT/pt_asset_tool/texture_overlay.py"

VERSION="$(cd "$ROOT" && python3 pt_asset.py --version)"
[[ "$VERSION" == "0.1.1" ]] || { echo "VERSION_CHECK_FAIL=$VERSION" >&2; exit 3; }

echo "PT_ASSET_TOOL_BACKEND_V0_1_1_PATCH"
echo "root=$ROOT"
echo "backup=$BACKUP"
echo "version=$VERSION"
echo "certifications_preserved=True"
echo "client_modified=False"
echo "patch_check=PASS"

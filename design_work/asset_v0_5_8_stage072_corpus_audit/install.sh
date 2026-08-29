#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
TARGET="${1:-$PWD}"

if [[ ! -f "$TARGET/pt_asset.py" ]]; then
  echo "ERRO: pt_asset.py não encontrado em: $TARGET" >&2
  echo "Execute este instalador apontando para a pasta design_work." >&2
  exit 2
fi
if [[ ! -d "$TARGET/history_py" ]]; then
  echo "ERRO: history_py não encontrado em: $TARGET" >&2
  exit 2
fi

STAMP="$(date +%Y%m%d_%H%M%S)"
if [[ -d "$TARGET/pt_asset_tool" ]]; then
  cp -a "$TARGET/pt_asset_tool" "$TARGET/pt_asset_tool.backup_$STAMP"
  echo "backup=$TARGET/pt_asset_tool.backup_$STAMP"
fi
IMPORTER="$TARGET/history_py/pt_smd_blender_import_v3_static_armature.py"
if [[ -f "$IMPORTER" ]]; then
  cp -a "$IMPORTER" "$IMPORTER.backup_$STAMP"
  echo "backup=$IMPORTER.backup_$STAMP"
fi

rm -rf "$TARGET/pt_asset_tool"
cp -a "$HERE/pt_asset_tool" "$TARGET/pt_asset_tool"
cp -a "$HERE/history_py/pt_smd_blender_import_v3_static_armature.py" "$IMPORTER"

echo "installed=$TARGET/pt_asset_tool"
echo "installed=$IMPORTER"
cd "$TARGET"
python3 -m compileall -q pt_asset_tool history_py/pt_smd_blender_import_v3_static_armature.py
python3 pt_asset.py --version

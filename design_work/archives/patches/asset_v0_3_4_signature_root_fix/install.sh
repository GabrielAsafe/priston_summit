#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
TARGET="${1:-$PWD}"
if [[ ! -f "$TARGET/pt_asset.py" ]]; then
  echo "ERRO: pt_asset.py não encontrado em: $TARGET" >&2
  echo "Execute este instalador apontando para a pasta design_work." >&2
  exit 2
fi
STAMP="$(date +%Y%m%d_%H%M%S)"
if [[ -d "$TARGET/pt_asset_tool" ]]; then
  cp -a "$TARGET/pt_asset_tool" "$TARGET/pt_asset_tool.backup_$STAMP"
  echo "backup=$TARGET/pt_asset_tool.backup_$STAMP"
fi
rm -rf "$TARGET/pt_asset_tool"
cp -a "$HERE/pt_asset_tool" "$TARGET/pt_asset_tool"
echo "installed=$TARGET/pt_asset_tool"
cd "$TARGET"
python3 pt_asset.py --version

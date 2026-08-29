#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

install -m 0755 "$PKG/summit_workspace_asset_bridge_v1_1.py" \
  "$ROOT/summit_workspace_asset_bridge_v1_1.py"
install -m 0755 "$PKG/summit_roundtrip_restore_v3.py" \
  "$ROOT/summit_roundtrip_restore_v3.py"
install -m 0755 "$PKG/pt_character_artist_pipeline_v2_1.py" \
  "$ROOT/pt_character_artist_pipeline_v2_1.py"

echo "PT_ITERATIVE_ROUNDTRIP_UPDATE_V1"
echo "ROOT=$ROOT"
echo "installed=3"
echo "install_check=PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

install -m 0644 "$PKG/pt_pikeman_character_asset_auto_witness_g7_v1.py" \
  "$ROOT/pt_pikeman_character_asset_auto_witness_g7_v1.py"
install -m 0755 "$PKG/run_pikeman_character_asset_auto_witness_g7_v1.sh" \
  "$ROOT/run_pikeman_character_asset_auto_witness_g7_v1.sh"

echo "PT_PIKEMAN_CHARACTER_ASSET_AUTO_WITNESS_G7_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=2"
echo "client_modified=False"
echo "install_check=PASS"

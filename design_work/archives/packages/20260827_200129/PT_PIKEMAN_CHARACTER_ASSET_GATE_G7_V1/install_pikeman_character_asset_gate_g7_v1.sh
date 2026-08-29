#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

install -m 0644 "$PKG/pt_pikeman_character_asset_inspect_g7_v1.py" \
  "$ROOT/pt_pikeman_character_asset_inspect_g7_v1.py"
install -m 0755 "$PKG/pt_pikeman_character_asset_gate_g7_v1.py" \
  "$ROOT/pt_pikeman_character_asset_gate_g7_v1.py"
install -m 0755 "$PKG/prepare_pikeman_character_asset_g7_v1.sh" \
  "$ROOT/prepare_pikeman_character_asset_g7_v1.sh"

mkdir -p "$ROOT/docs"
install -m 0644 "$PKG/docs/PT_PIKEMAN_CHARACTER_ASSET_GATE_G7_V1_2026-08-27.md" \
  "$ROOT/docs/PT_PIKEMAN_CHARACTER_ASSET_GATE_G7_V1_2026-08-27.md"

echo "PT_PIKEMAN_CHARACTER_ASSET_GATE_G7_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=4"
echo "client_modified=False"
echo "install_check=PASS"

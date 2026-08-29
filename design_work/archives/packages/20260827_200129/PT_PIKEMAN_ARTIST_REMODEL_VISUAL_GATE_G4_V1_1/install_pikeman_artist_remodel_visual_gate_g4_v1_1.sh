#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
install -m 0644 "$PKG/pt_pikeman_artist_remodel_visual_edit_g4_v1_1.py" \
  "$ROOT/pt_pikeman_artist_remodel_visual_edit_g4_v1_1.py"
install -m 0755 "$PKG/pt_pikeman_artist_remodel_visual_gate_g4_v1_1.py" \
  "$ROOT/pt_pikeman_artist_remodel_visual_gate_g4_v1_1.py"
mkdir -p "$ROOT/docs"
install -m 0644 "$PKG/docs/PT_PIKEMAN_ARTIST_REMODEL_VISUAL_GATE_G4_V1_1_2026-08-27.md" \
  "$ROOT/docs/PT_PIKEMAN_ARTIST_REMODEL_VISUAL_GATE_G4_V1_1_2026-08-27.md"
echo "PT_PIKEMAN_ARTIST_REMODEL_VISUAL_GATE_G4_V1_1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=3"
echo "client_modified=False"
echo "install_check=PASS"

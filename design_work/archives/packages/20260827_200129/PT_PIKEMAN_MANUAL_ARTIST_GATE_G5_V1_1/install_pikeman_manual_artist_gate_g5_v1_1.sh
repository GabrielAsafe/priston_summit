#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
install -m 0644 "$PKG/pt_pikeman_manual_artist_inspect_g5_v1_1.py" \
  "$ROOT/pt_pikeman_manual_artist_inspect_g5_v1_1.py"
install -m 0755 "$PKG/pt_pikeman_manual_artist_gate_g5_v1_1.py" \
  "$ROOT/pt_pikeman_manual_artist_gate_g5_v1_1.py"
mkdir -p "$ROOT/docs"
install -m 0644 "$PKG/docs/PT_PIKEMAN_MANUAL_ARTIST_GATE_G5_V1_1_2026-08-27.md" \
  "$ROOT/docs/PT_PIKEMAN_MANUAL_ARTIST_GATE_G5_V1_1_2026-08-27.md"
echo "PT_PIKEMAN_MANUAL_ARTIST_GATE_G5_V1_1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=3"
echo "client_modified=False"
echo "install_check=PASS"

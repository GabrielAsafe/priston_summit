#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/history_py" "$ROOT/docs"

install -m 0755 "$PKG/pt_pikeman_smd_selfillum_build_v1.py" \
  "$ROOT/pt_pikeman_smd_selfillum_build_v1.py"
install -m 0644 "$PKG/docs/PIKEMAN_SELFILLUM_RUNTIME_GATE_V1_2026-08-27.md" \
  "$ROOT/docs/PIKEMAN_SELFILLUM_RUNTIME_GATE_V1_2026-08-27.md"

echo "PT_PIKEMAN_SELFILLUM_RUNTIME_GATE_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=2"
echo "client_modified=False"
echo "install_check=PASS"

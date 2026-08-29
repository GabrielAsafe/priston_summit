#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

install -m 0755 "$PKG/pt_bmp_visible_test_patch_v1.py" \
  "$ROOT/pt_bmp_visible_test_patch_v1.py"
install -m 0644 "$PKG/README_TEXTURE_RUNTIME_GATE_V1.md" \
  "$ROOT/docs/README_TEXTURE_RUNTIME_GATE_V1.md"

echo "PT_TEXTURE_RUNTIME_GATE_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=2"
echo "install_check=PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/docs"

install -m 0755 "$PKG/pt_pikeman_bootstrap_v1.py" \
  "$ROOT/pt_pikeman_bootstrap_v1.py"
install -m 0644 "$PKG/README_PIKEMAN_BOOTSTRAP_V1.md" \
  "$ROOT/docs/README_PIKEMAN_BOOTSTRAP_V1.md"

echo "PT_PIKEMAN_BOOTSTRAP_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=2"
echo "client_modified=False"
echo "install_check=PASS"

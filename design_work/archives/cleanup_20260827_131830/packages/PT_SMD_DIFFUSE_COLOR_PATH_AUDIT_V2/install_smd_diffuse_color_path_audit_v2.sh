#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "$ROOT/docs"
install -m 0755 "$PKG/pt_smd_diffuse_color_path_audit_v2.py" \
  "$ROOT/pt_smd_diffuse_color_path_audit_v2.py"
install -m 0644 "$PKG/docs/PT_SMD_DIFFUSE_COLOR_PATH_AUDIT_V2_2026-08-27.md" \
  "$ROOT/docs/PT_SMD_DIFFUSE_COLOR_PATH_AUDIT_V2_2026-08-27.md"
echo "PT_SMD_DIFFUSE_COLOR_PATH_AUDIT_V2_INSTALL"
echo "ROOT=$ROOT"
echo "installed=2"
echo "client_modified=False"
echo "install_check=PASS"

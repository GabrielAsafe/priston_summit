#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/docs"

install -m 0755 "$PKG/pt_smd_material_render_source_audit_v1.py" \
  "$ROOT/pt_smd_material_render_source_audit_v1.py"
install -m 0644 "$PKG/docs/SMD_MATERIAL_RENDER_SOURCE_AUDIT_V1_2026-08-27.md" \
  "$ROOT/docs/SMD_MATERIAL_RENDER_SOURCE_AUDIT_V1_2026-08-27.md"

echo "PT_SMD_MATERIAL_RENDER_SOURCE_AUDIT_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=2"
echo "source_modified=False"
echo "client_modified=False"
echo "install_check=PASS"

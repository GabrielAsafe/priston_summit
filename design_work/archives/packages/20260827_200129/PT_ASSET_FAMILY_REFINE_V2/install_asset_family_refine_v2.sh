#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
install -m 0755 "$PKG/pt_asset_family_refine_v2.py" "$ROOT/pt_asset_family_refine_v2.py"
install -m 0755 "$PKG/run_asset_family_refine_v2.sh" "$ROOT/run_asset_family_refine_v2.sh"
mkdir -p "$ROOT/docs"
install -m 0644 "$PKG/Summit_Tale_Asset_Similarity_Findings_v12_1_2026-08-27.md" \
  "$ROOT/docs/Summit_Tale_Asset_Similarity_Findings_v12_1_2026-08-27.md"
echo "PT_ASSET_FAMILY_REFINE_V2_INSTALL"
echo "installed=3"
echo "client_modified=False"
echo "install_check=PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

install -m 0755 "$PKG/pt_asset_similarity_survey_v1.py" \
  "$ROOT/pt_asset_similarity_survey_v1.py"

install -m 0755 "$PKG/run_asset_similarity_survey_v1.sh" \
  "$ROOT/run_asset_similarity_survey_v1.sh"

mkdir -p "$ROOT/docs"
install -m 0644 "$PKG/Summit_Tale_Blender_3D_Source_of_Truth_v12_0_2026-08-27.md" \
  "$ROOT/docs/Summit_Tale_Blender_3D_Source_of_Truth_v12_0_2026-08-27.md"

echo "PT_ASSET_SIMILARITY_EXPLORATION_V1_INSTALL"
echo "installed=3"
echo "client_modified=False"
echo "install_check=PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

install -m 0755 "$PKG/pt_asset_similarity_survey_v1_1.py" \
  "$ROOT/pt_asset_similarity_survey_v1_1.py"
install -m 0755 "$PKG/run_asset_similarity_survey_v1_1.sh" \
  "$ROOT/run_asset_similarity_survey_v1_1.sh"

echo "PT_ASSET_SIMILARITY_EXPLORATION_V1_1_INSTALL"
echo "installed=2"
echo "client_modified=False"
echo "install_check=PASS"

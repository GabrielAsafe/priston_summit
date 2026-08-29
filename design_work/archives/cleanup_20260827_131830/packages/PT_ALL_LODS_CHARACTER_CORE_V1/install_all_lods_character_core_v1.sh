#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

install -m 0755 "$PKG/pt_character_multi_object_lod_pipeline_v2.py" \
  "$ROOT/pt_character_multi_object_lod_pipeline_v2.py"

echo "PT_ALL_LODS_CHARACTER_CORE_INSTALL_V1"
echo "ROOT=$ROOT"
echo "installed=1"
echo "install_check=PASS"

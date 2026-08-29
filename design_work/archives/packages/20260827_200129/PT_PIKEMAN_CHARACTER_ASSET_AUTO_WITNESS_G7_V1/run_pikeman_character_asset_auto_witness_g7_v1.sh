#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
SRC="$ROOT/blender/pikeman_c01_m4/artist/C001_C002_character_asset_master_g7_v1.blend"
OUT="$ROOT/blender/pikeman_c01_m4/artist/C001_C002_character_asset_g7_auto_witness_v1.blend"
REPORT="$ROOT/logs/characters/pikeman_c01_m4/geometry/character_asset_g7_v1/auto_witness.json"
SCRIPT="$ROOT/pt_pikeman_character_asset_auto_witness_g7_v1.py"

if [[ ! -f "$SRC" ]]; then
  echo "auto_witness_runner_check=FAIL: missing source: $SRC" >&2
  exit 2
fi
if [[ ! -f "$SCRIPT" ]]; then
  echo "auto_witness_runner_check=FAIL: missing script: $SCRIPT" >&2
  exit 3
fi
if [[ -e "$OUT" ]]; then
  echo "auto_witness_runner_check=FAIL: output already exists: $OUT" >&2
  exit 4
fi

mkdir -p "$(dirname "$OUT")" "$(dirname "$REPORT")"

SRC_SHA="$(sha256sum "$SRC" | awk '{print $1}')"
echo "PT_PIKEMAN_CHARACTER_ASSET_AUTO_WITNESS_G7_V1_RUNNER"
echo "source=$SRC"
echo "source_sha256=$SRC_SHA"
echo "output=$OUT"
echo "client_modified=False"

flatpak run --filesystem=home org.blender.Blender \
  --background "$SRC" \
  --python-exit-code 1 \
  --python "$SCRIPT" \
  -- "$OUT" "$REPORT"

OUT_SHA="$(sha256sum "$OUT" | awk '{print $1}')"
echo "output_sha256=$OUT_SHA"
echo "report=$REPORT"
echo "client_modified=False"
echo "auto_witness_runner_check=PASS"

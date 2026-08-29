#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
CLIENT="${2:-$HOME/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
OUT="${3:-$ROOT/logs/asset_similarity_v1_1}"

cd "$ROOT"

python3 "$ROOT/pt_asset_similarity_survey_v1_1.py" \
  --root "$CLIENT" \
  --root "$ROOT" \
  --out-dir "$OUT" \
  --hash-mode smart

echo
echo "REPORT=$OUT/asset_similarity_report.md"
echo "FAMILIES=$OUT/compatibility_families.csv"
echo "REUSE_QUEUE=$OUT/reuse_queue.csv"
echo "ASSET_SETS=$OUT/asset_sets_from_inx.csv"
echo "CHECKPOINT=$OUT/scan_checkpoint.ndjson"
echo "run_asset_similarity_survey_check=PASS"

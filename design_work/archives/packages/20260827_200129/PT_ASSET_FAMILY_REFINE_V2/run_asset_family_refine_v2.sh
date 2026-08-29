#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
CLIENT="${2:-$HOME/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
INV="${3:-$ROOT/logs/asset_similarity_v1_1/asset_inventory.json}"
OUT="${4:-$ROOT/logs/asset_family_refine_v2}"

cd "$ROOT"
python3 "$ROOT/pt_asset_family_refine_v2.py" \
  --inventory "$INV" \
  --client-root "$CLIENT" \
  --out-dir "$OUT" \
  --raw-signatures

echo "REPORT=$OUT/refinement_report.md"
echo "REPRESENTATIVES=$OUT/runtime_representative_queue.csv"
echo "INX_PAIRING=$OUT/inx_physical_pairing.csv"
echo "SMD_SUBFAMILIES=$OUT/smd_semantic_subfamilies.csv"
echo "OUTLIERS=$OUT/outliers.csv"
echo "run_asset_family_refine_check=PASS"

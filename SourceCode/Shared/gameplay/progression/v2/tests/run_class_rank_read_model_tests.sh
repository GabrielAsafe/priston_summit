#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../" && pwd)"
OUT="${TMPDIR:-/tmp}/summit_class_rank_read_model_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/gameplay/progression/v2/tests/class_rank_read_model_smoke.cpp" -o "$OUT"
"$OUT"
echo "class rank read model smoke: PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../" && pwd)"
OUT="${TMPDIR:-/tmp}/summit_restoration_transaction_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/gameplay/commerce/v2/tests/restoration_transaction_smoke.cpp" -o "$OUT"
"$OUT"
echo "restoration transaction smoke: PASS"

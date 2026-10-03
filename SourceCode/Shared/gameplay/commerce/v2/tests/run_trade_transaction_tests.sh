#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_trade_transaction_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"${CXX:-g++}" -std=c++17 -Wall -Wextra -I"$ROOT" \
  "$ROOT/gameplay/commerce/v2/tests/trade_transaction_smoke.cpp" -o "$OUT"
"$OUT"
echo "trade transaction smoke: PASS"

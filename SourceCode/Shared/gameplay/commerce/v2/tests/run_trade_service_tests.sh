#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_trade_service_smoke"
"${CXX:-g++}" -std=c++17 -I"$ROOT" \
  "$ROOT/gameplay/commerce/v2/tests/trade_service_smoke.cpp" \
  -o "$OUT"
"$OUT"

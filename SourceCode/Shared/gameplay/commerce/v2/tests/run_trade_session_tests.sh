#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/trade_session_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/gameplay/commerce/v2/tests/trade_session_smoke.cpp" -o "$OUT"
"$OUT"
echo "trade session smoke: PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_premium_entitlement_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/gameplay/commerce/v2/tests/premium_entitlement_service_smoke.cpp" -o "$OUT"
"$OUT"
echo "premium entitlement service smoke: PASS"

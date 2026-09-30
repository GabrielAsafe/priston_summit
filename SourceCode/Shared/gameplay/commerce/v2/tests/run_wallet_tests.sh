#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_wallet_contract_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/commerce/v2/tests/wallet_contract_smoke.cpp" -o "$OUT"
"$OUT"
echo "wallet contract smoke: PASS"

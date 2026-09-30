#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/coin_wallet_contract_smoke"
"$CXX" -std=c++17 -Wall -Wextra -Werror -I"$ROOT/Shared" "$ROOT/Shared/gameplay/commerce/v2/tests/coin_wallet_contract_smoke.cpp" -o "$OUT"
"$OUT"

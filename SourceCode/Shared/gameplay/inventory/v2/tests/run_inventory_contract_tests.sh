#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_inventory_contract_smoke"
"$CXX" -std=c++17 -Wall -Wextra -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/inventory/v2/tests/inventory_contract_smoke.cpp" -o "$OUT"
"$OUT"

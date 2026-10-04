#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/inventory_use_delay_policy_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/gameplay/inventory/v2/tests/inventory_use_delay_policy_smoke.cpp" -o "$OUT"
"$OUT"
echo "inventory use delay policy smoke: PASS"

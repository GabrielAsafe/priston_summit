#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_personal_shop_transaction_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"${CXX:-g++}" -std=c++17 -Wall -Wextra -I"$ROOT" "$ROOT/gameplay/commerce/v2/tests/personal_shop_transaction_smoke.cpp" -o "$OUT"
"$OUT"
echo "personal shop transaction smoke: PASS"

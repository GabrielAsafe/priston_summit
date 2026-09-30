#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_shop_service_smoke"
"${CXX:-g++}" -std=c++17 -I"$ROOT" \
  "$ROOT/gameplay/commerce/v2/tests/shop_service_smoke.cpp" \
  -o "$OUT"
"$OUT"

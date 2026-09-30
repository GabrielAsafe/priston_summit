#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_personal_shop_service_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$ROOT" \
    "$ROOT/gameplay/commerce/v2/tests/personal_shop_service_smoke.cpp" \
    -o "$OUT"
"$OUT"

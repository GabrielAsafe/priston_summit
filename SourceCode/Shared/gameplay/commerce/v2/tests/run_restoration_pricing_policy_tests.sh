#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_restoration_pricing_policy_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$(cd "$ROOT/.." && pwd)" \
  "$ROOT/gameplay/commerce/v2/tests/restoration_pricing_policy_smoke.cpp" -o "$BIN"
"$BIN"
rm -f "$BIN"

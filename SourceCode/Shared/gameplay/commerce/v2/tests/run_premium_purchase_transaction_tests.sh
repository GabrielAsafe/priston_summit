#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_premium_purchase_tx_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$ROOT/Shared" "$ROOT/Shared/gameplay/commerce/v2/tests/premium_purchase_transaction_smoke.cpp" -o "$BIN"
"$BIN"
rm -f "$BIN"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_premium_purchase_replay_store_contract_$$"
trap 'rm -f "$BIN"' EXIT
c++ -std=c++17 -I"$ROOT" "$ROOT/gameplay/commerce/v2/tests/premium_purchase_replay_store_contract_smoke.cpp" -o "$BIN"
"$BIN"

#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_battle_pass_claim_policy_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$(cd "$ROOT/.." && pwd)" "$HERE/battle_pass_claim_policy_smoke.cpp" -o "$BIN"
"$BIN"
rm -f "$BIN"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../" && pwd)"
OUT="${TMPDIR:-/tmp}/summit_battle_pass_progress_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/gameplay/progression/v2/tests/battle_pass_progress_smoke.cpp" -o "$OUT"
"$OUT"
echo "battle pass progress smoke: PASS"

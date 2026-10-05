#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../" && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_player_experience_service_smoke"
"$CXX" -std=c++17 -Wall -Wextra -Werror -I"$ROOT" \
  "$ROOT/gameplay/progression/v2/tests/player_experience_service_smoke.cpp" -o "$OUT"
"$OUT"
echo "player experience service smoke: PASS"

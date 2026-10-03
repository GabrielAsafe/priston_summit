#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_summoned_actor_commands.$$"
trap 'rm -f "$OUT"' EXIT
"${CXX:-g++}" -std=c++17 -Wall -Wextra -I"$ROOT" "$ROOT/gameplay/monsters/v2/tests/summoned_actor_commands_smoke.cpp" -o "$OUT"
"$OUT"
echo "summoned actor commands smoke: PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_monster_query_smoke"
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/monsters/v2/tests/monster_query_smoke.cpp" -o "$OUT"
"$OUT"
echo "MonsterQuery smoke: PASS"

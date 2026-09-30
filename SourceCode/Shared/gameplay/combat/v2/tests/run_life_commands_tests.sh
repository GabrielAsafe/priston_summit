#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_life_commands_smoke"
g++ -std=c++14 -Wall -Wextra -Werror -I"$ROOT/Shared" \
    "$ROOT/Shared/gameplay/combat/v2/tests/life_commands_smoke.cpp" -o "$OUT"
"$OUT"
echo "life commands smoke: PASS"

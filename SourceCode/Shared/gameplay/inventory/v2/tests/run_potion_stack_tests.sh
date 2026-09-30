#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_potion_stack_smoke"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -I"$ROOT" "$ROOT/gameplay/inventory/v2/tests/potion_stack_smoke.cpp" -o "$OUT"
"$OUT"

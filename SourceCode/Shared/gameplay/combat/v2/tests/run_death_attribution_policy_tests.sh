#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_death_attribution_policy_smoke.$$"
trap 'rm -f "$BIN"' EXIT INT TERM
"${CXX:-g++}" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/combat/v2/tests/death_attribution_policy_smoke.cpp" -o "$BIN"
"$BIN"
echo "death attribution policy smoke: PASS"

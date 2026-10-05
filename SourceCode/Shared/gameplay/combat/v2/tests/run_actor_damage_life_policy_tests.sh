#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_actor_damage_life_policy_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"$CXX" -std=c++17 -I"$ROOT" "$ROOT/gameplay/combat/v2/tests/actor_damage_life_policy_smoke.cpp" -o "$OUT"
"$OUT"
echo "actor damage life policy smoke: PASS"

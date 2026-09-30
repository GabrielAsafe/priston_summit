#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/skill_runtime_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" "$ROOT/gameplay/skills/v2/tests/skill_runtime_smoke.cpp" -o "$OUT"
"$OUT"
echo "skill runtime smoke: PASS"

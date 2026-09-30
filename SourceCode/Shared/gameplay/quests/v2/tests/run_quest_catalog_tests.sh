#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/quest_catalog_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" "$ROOT/gameplay/quests/v2/tests/quest_catalog_smoke.cpp" -o "$OUT"
"$OUT"
echo "quest catalog smoke: PASS"

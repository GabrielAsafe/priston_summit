#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/quest_repository_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" "$ROOT/gameplay/quests/v2/tests/quest_repository_smoke.cpp" -o "$OUT"
"$OUT"
echo "quest repository smoke: PASS"

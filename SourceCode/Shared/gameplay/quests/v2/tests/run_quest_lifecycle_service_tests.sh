#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/quest_lifecycle_service_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/gameplay/quests/v2/tests/quest_lifecycle_service_smoke.cpp" -o "$OUT"
"$OUT"
echo "quest lifecycle service smoke: PASS"

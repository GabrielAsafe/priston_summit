#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/skill_animation_commands_smoke.$$"
trap 'rm -f "$OUT"' EXIT
${CXX:-g++} -std=c++11 -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/skills/v2/tests/skill_animation_commands_smoke.cpp" \
  -o "$OUT"
"$OUT"
echo "skill animation commands smoke: PASS"

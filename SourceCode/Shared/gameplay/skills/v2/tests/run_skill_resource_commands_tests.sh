#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
"$CXX" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/skills/v2/tests/skill_resource_commands_smoke.cpp" \
  -o "$TMP/skill_resource_commands_smoke"
"$TMP/skill_resource_commands_smoke"
echo "Skill resource commands smoke: PASS"

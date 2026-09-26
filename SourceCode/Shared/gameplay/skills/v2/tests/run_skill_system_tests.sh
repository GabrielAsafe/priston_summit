#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_skill_registry_smoke"

"$CXX" -std=c++14 -Wall -Wextra -Werror \
    -I"$ROOT/Shared" \
    "$ROOT/Shared/gameplay/skills/v2/SkillRegistry.cpp" \
    "$ROOT/Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp" \
    "$ROOT/Shared/gameplay/skills/v2/services/SkillService.cpp" \
    "$ROOT/Shared/gameplay/skills/v2/tests/skill_registry_smoke.cpp" \
    -o "$OUT"

"$OUT"

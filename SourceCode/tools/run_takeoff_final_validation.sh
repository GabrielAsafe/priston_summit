#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX="${CXX:-g++}"
TMP_ROOT="${TMPDIR:-/tmp}/takeoff_final_validation"
mkdir -p "$TMP_ROOT"

python3 "$ROOT/tools/validate_progression_v2_migration.py"
python3 "$ROOT/tools/test_progression_repository_contracts.py"
python3 "$ROOT/tools/validate_server_shared_gameplay_manifest.py"
bash "$ROOT/tools/validate_progression_sqlite_translation_units.sh"
python3 "$ROOT/tools/validate_legacy_quest_contract.py"
python3 "$ROOT/tools/validate_legacy_profession_contract.py"
python3 "$ROOT/tools/validate_progression_trigger_hooks.py"
bash "$ROOT/Shared/gameplay/tests/run_v2_domain_tests.sh"

"$CXX" -std=gnu++14 -Wall -Wextra -Werror -pedantic \
    -I"$ROOT/Shared" -I"$ROOT/SrcServer/src" \
    -c "$ROOT/SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp" \
    -o "$TMP_ROOT/ProgressionRuntime.o"

"$CXX" -std=gnu++14 -Wall -Wextra -Werror -pedantic \
    -I"$ROOT/Shared" -I"$ROOT/SrcServer/src" \
    -c "$ROOT/SrcServer/src/systems/gameplay/progression/v2/ServerProgressionConditionContext.cpp" \
    -o "$TMP_ROOT/ServerProgressionConditionContext.o"

echo "TAKEOFF final validation: PASS"

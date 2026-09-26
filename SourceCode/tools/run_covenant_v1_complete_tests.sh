#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX="${CXX:-g++}"
COMMON=(-std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" -I"$ROOT/SrcServer/src")

bash "$ROOT/tools/run_covenant_v0_1_portable_tests.sh"
python3 "$ROOT/tools/validate_covenant_v1_complete.py"
python3 "$ROOT/tools/validate_covenant_modifier_coverage.py"
python3 "$ROOT/tools/test_covenant_sqlite_restart_e2e.py"

"$CXX" "${COMMON[@]}" -fsyntax-only \
  "$ROOT/SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp"
echo "ProgressionRuntime Covenant V1 syntax: PASS"

"$CXX" "${COMMON[@]}" -fsyntax-only \
  "$ROOT/SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp"
echo "Mining/Gathering Covenant hooks syntax: PASS"

"$CXX" "${COMMON[@]}" -fsyntax-only \
  "$ROOT/SrcServer/src/systems/gameplay/mining/v2/MiningRuntime.cpp"
echo "Mining Covenant bridge syntax: PASS"

echo "Covenant V1 complete portable/static validation: PASS"

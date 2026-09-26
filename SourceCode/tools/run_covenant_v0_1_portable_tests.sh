#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX="${CXX:-g++}"
BUILD_DIR="${TMPDIR:-/tmp}/summit_covenant_v0_1_tests"
mkdir -p "$BUILD_DIR"
COMMON=(-std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" -I"$ROOT/SrcServer/src")

"$CXX" "${COMMON[@]}" \
  "$ROOT/Shared/gameplay/covenants/v2/CovenantService.cpp" \
  "$ROOT/Shared/gameplay/covenants/v2/tests/covenant_service_smoke.cpp" \
  -o "$BUILD_DIR/covenant_service_smoke"
"$BUILD_DIR/covenant_service_smoke"
echo "Covenant lifecycle/content smoke: PASS"

"$CXX" "${COMMON[@]}" \
  "$ROOT/Shared/gameplay/covenants/v2/CovenantService.cpp" \
  "$ROOT/Shared/gameplay/covenants/v2/CovenantRewardProcessor.cpp" \
  "$ROOT/Shared/gameplay/covenants/v2/tests/covenant_reward_processor_smoke.cpp" \
  -o "$BUILD_DIR/covenant_reward_processor_smoke"
"$BUILD_DIR/covenant_reward_processor_smoke"
echo "Covenant rewards smoke: PASS"

"$CXX" "${COMMON[@]}" \
  "$ROOT/Shared/gameplay/covenants/v2/tests/covenant_modifier_evaluation_smoke.cpp" \
  -o "$BUILD_DIR/covenant_modifier_evaluation_smoke"
"$BUILD_DIR/covenant_modifier_evaluation_smoke"
echo "Covenant modifier evaluation smoke: PASS"

"$CXX" "${COMMON[@]}" \
  -DSUMMIT_COVENANT_CHAT_SMOKE_TEST=1 \
  "$ROOT/SrcServer/src/systems/gameplay/covenants/v2/CovenantChatCommand.cpp" \
  "$ROOT/SrcServer/src/systems/gameplay/covenants/v2/tests/covenant_chat_command_smoke.cpp" \
  -o "$BUILD_DIR/covenant_chat_command_smoke"
"$BUILD_DIR/covenant_chat_command_smoke"
echo "Covenant chat command smoke: PASS"

python3 "$ROOT/tools/validate_progression_v2_migration.py"
python3 "$ROOT/tools/test_progression_repository_contracts.py"
bash "$ROOT/tools/validate_progression_sqlite_translation_units.sh"

echo "Covenant V0.1 portable validation: PASS"

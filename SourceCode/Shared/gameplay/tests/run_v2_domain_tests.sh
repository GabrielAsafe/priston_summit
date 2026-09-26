#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
CXX="${CXX:-g++}"
BUILD_DIR="${TMPDIR:-/tmp}/priston_gameplay_v2_tests"
mkdir -p "$BUILD_DIR"

COMMON=(-std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" -I"$ROOT/SrcServer/src")

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/events/v2/tests/gameplay_event_hub_smoke.cpp" \
    -o "$BUILD_DIR/gameplay_event_hub_smoke"
"$BUILD_DIR/gameplay_event_hub_smoke"

echo "GameplayEventHub smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/quests/v2/tests/quest_objective_smoke.cpp" \
    -o "$BUILD_DIR/quest_objective_smoke"
"$BUILD_DIR/quest_objective_smoke"

echo "QuestObjective smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/SrcServer/src/systems/gameplay/hunting/v2/HuntingService.cpp" \
    "$ROOT/Shared/gameplay/hunting/v2/tests/hunting_service_smoke.cpp" \
    -o "$BUILD_DIR/hunting_service_smoke"
"$BUILD_DIR/hunting_service_smoke"

echo "HuntingService smoke: PASS"


"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/professions/v2/tests/profession_service_smoke.cpp" \
    -o "$BUILD_DIR/profession_service_smoke"
"$BUILD_DIR/profession_service_smoke"

echo "ProfessionService smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/SrcServer/src/systems/gameplay/gathering/v2/GatheringService.cpp" \
    "$ROOT/Shared/gameplay/gathering/v2/tests/gathering_service_smoke.cpp" \
    -o "$BUILD_DIR/gathering_service_smoke"
"$BUILD_DIR/gathering_service_smoke"

echo "GatheringService smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/SrcServer/src/systems/gameplay/gathering/v2/GatheringService.cpp" \
    "$ROOT/SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp" \
    "$ROOT/Shared/gameplay/mining/v2/tests/mining_service_smoke.cpp" \
    -o "$BUILD_DIR/mining_service_smoke"
"$BUILD_DIR/mining_service_smoke"

echo "MiningService smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/SrcServer/src/systems/gameplay/gathering/v2/GatheringService.cpp" \
    "$ROOT/SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp" \
    "$ROOT/Shared/gameplay/mining/v2/tests/mining_interaction_smoke.cpp" \
    -o "$BUILD_DIR/mining_interaction_smoke"
"$BUILD_DIR/mining_interaction_smoke"

echo "MiningInteractionService smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/conditions/v2/tests/condition_service_smoke.cpp" \
    -o "$BUILD_DIR/condition_service_smoke"
"$BUILD_DIR/condition_service_smoke"

echo "Condition/Reward/Modifier smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/covenants/v2/CovenantService.cpp" \
    "$ROOT/Shared/gameplay/covenants/v2/tests/covenant_service_smoke.cpp" \
    -o "$BUILD_DIR/covenant_service_smoke"
"$BUILD_DIR/covenant_service_smoke"

echo "CovenantService smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/covenants/v2/CovenantService.cpp" \
    "$ROOT/Shared/gameplay/covenants/v2/CovenantRewardProcessor.cpp" \
    "$ROOT/Shared/gameplay/covenants/v2/tests/covenant_reward_processor_smoke.cpp" \
    -o "$BUILD_DIR/covenant_reward_processor_smoke"
"$BUILD_DIR/covenant_reward_processor_smoke"

echo "Covenant reward processor smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/covenants/v2/tests/covenant_modifier_evaluation_smoke.cpp" \
    -o "$BUILD_DIR/covenant_modifier_evaluation_smoke"
"$BUILD_DIR/covenant_modifier_evaluation_smoke"

echo "Covenant modifier evaluation smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/reputation/v2/ReputationService.cpp" \
    "$ROOT/Shared/gameplay/reputation/v2/tests/reputation_service_smoke.cpp" \
    -o "$BUILD_DIR/reputation_service_smoke"
"$BUILD_DIR/reputation_service_smoke"

echo "ReputationService smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/story/v2/StoryService.cpp" \
    "$ROOT/Shared/gameplay/story/v2/tests/story_state_smoke.cpp" \
    -o "$BUILD_DIR/story_state_smoke"
"$BUILD_DIR/story_state_smoke"

echo "StoryState smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/classes/v2/PlayableClassRegistry.cpp" \
    "$ROOT/Shared/gameplay/classes/v2/content/DefaultClassContent.cpp" \
    "$ROOT/Shared/gameplay/classes/v2/content/LegacyClassCatalog.cpp" \
    "$ROOT/Shared/gameplay/classes/v2/content/ReservedClassCatalog.cpp" \
    "$ROOT/Shared/gameplay/classes/v2/services/ClassService.cpp" \
    "$ROOT/Shared/gameplay/classes/v2/tests/playable_class_registry_smoke.cpp" \
    -o "$BUILD_DIR/playable_class_registry_smoke"
"$BUILD_DIR/playable_class_registry_smoke"

echo "PlayableClassRegistry parity smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/covenants/v2/CovenantService.cpp" \
    "$ROOT/Shared/gameplay/reputation/v2/ReputationService.cpp" \
    "$ROOT/Shared/gameplay/story/v2/StoryService.cpp" \
    "$ROOT/Shared/gameplay/progression/v2/ProgressionGateways.cpp" \
    "$ROOT/Shared/gameplay/progression/v2/tests/progression_gateways_smoke.cpp" \
    -o "$BUILD_DIR/progression_gateways_smoke"
"$BUILD_DIR/progression_gateways_smoke"

echo "Progression gateways smoke: PASS"
"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/covenants/v2/CovenantService.cpp" \
    "$ROOT/Shared/gameplay/reputation/v2/ReputationService.cpp" \
    "$ROOT/SrcServer/src/systems/gameplay/progression/v2/ServerProgressionConditionContext.cpp" \
    "$ROOT/Shared/gameplay/progression/v2/tests/server_condition_context_smoke.cpp" \
    -o "$BUILD_DIR/server_condition_context_smoke"
"$BUILD_DIR/server_condition_context_smoke"

echo "Server progression condition context smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/covenants/v2/CovenantService.cpp" \
    "$ROOT/Shared/gameplay/story/v2/StoryService.cpp" \
    "$ROOT/Shared/gameplay/progression/v2/ProgressionContent.cpp" \
    "$ROOT/Shared/gameplay/progression/v2/tests/progression_content_smoke.cpp" \
    -o "$BUILD_DIR/progression_content_smoke"
"$BUILD_DIR/progression_content_smoke"

echo "Progression content registrar smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/progression/v2/tests/custom_handlers_smoke.cpp" \
    -o "$BUILD_DIR/custom_handlers_smoke"
"$BUILD_DIR/custom_handlers_smoke"

echo "Progression custom handlers smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/story/v2/StoryService.cpp" \
    "$ROOT/Shared/gameplay/progression/v2/tests/progression_player_runtime_smoke.cpp" \
    -o "$BUILD_DIR/progression_player_runtime_smoke"
"$BUILD_DIR/progression_player_runtime_smoke"

echo "Progression player runtime smoke: PASS"

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/progression/v2/tests/progression_trigger_coordinator_smoke.cpp" \
    -o "$BUILD_DIR/progression_trigger_coordinator_smoke"
"$BUILD_DIR/progression_trigger_coordinator_smoke"

echo "Progression trigger coordinator smoke: PASS"

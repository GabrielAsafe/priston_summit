#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
run() { echo ">>> $*"; "$@"; }

# Quest domain: catalog, commands, state, lifecycle, progress, persistence and admin.
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_catalog_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_commands_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_lifecycle_service_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_objective_service_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_progress_service_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_repository_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_service_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_state_query_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_view_query_tests.sh"
run bash "$ROOT/Shared/gameplay/quests/v2/tests/run_quest_admin_service_tests.sh"

# Player XP authority used by combat and progression rewards.
run bash "$ROOT/Shared/gameplay/progression/v2/tests/run_player_experience_service_tests.sh"
run python3 "$ROOT/tools/validate_player_experience_ownership_v1.py"

# Progression consumers.
run bash "$ROOT/Shared/gameplay/progression/v2/tests/run_battle_pass_claim_policy_tests.sh"
run bash "$ROOT/Shared/gameplay/progression/v2/tests/run_battle_pass_progress_application_tests.sh"
run bash "$ROOT/Shared/gameplay/progression/v2/tests/run_battle_pass_progress_tests.sh"
run bash "$ROOT/Shared/gameplay/progression/v2/tests/run_class_rank_query_tests.sh"
run bash "$ROOT/Shared/gameplay/progression/v2/tests/run_class_rank_read_model_tests.sh"

# Profession contracts/persistence plus the V2-only fishing domain.
run bash "$ROOT/Shared/gameplay/professions/v2/tests/run_profession_event_tests.sh"
run bash "$ROOT/Shared/gameplay/professions/v2/tests/run_profession_persistence_tests.sh"
run bash "$ROOT/Shared/gameplay/professions/v2/tests/run_profession_port_tests.sh"
run bash "$ROOT/tools/run_fishing_v2_portable_tests.sh"

echo "QUEST_PROGRESSION_PROFESSIONS_PORTABLE=PASS"

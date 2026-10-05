#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
python3 tools/validate_monster_npc_authority_v1.py
python3 tools/validate_monster_query_boundary_v1.py
python3 tools/validate_npc_query_boundary_v1.py
python3 tools/validate_monster_follower_release_v2.py
python3 tools/validate_companion_monster_follower_boundary_v1.py
bash Shared/gameplay/monsters/v2/tests/run_monster_commands_tests.sh
bash Shared/gameplay/monsters/v2/tests/run_monster_query_tests.sh
bash Shared/gameplay/monsters/v2/tests/run_summoned_actor_commands_tests.sh
echo "monster/npc v2 portable: PASS"

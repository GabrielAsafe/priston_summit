#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_world_query_smoke"
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/world/v2/tests/world_query_smoke.cpp" -o "$OUT"
"$OUT"
echo "WorldQuery smoke: PASS"

STAGE_OUT="${TMPDIR:-/tmp}/summit_stage_query_smoke"
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/world/v2/tests/stage_query_smoke.cpp" -o "$STAGE_OUT"
"$STAGE_OUT"
echo "StageQuery smoke: PASS"

TELEPORT_OUT="${TMPDIR:-/tmp}/summit_teleport_service_smoke"
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/world/v2/tests/teleport_service_smoke.cpp" -o "$TELEPORT_OUT"
"$TELEPORT_OUT"
echo "TeleportService smoke: PASS"

SPAWN_OUT="${TMPDIR:-/tmp}/summit_spawn_service_smoke"
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/world/v2/tests/spawn_service_smoke.cpp" -o "$SPAWN_OUT"
"$SPAWN_OUT"
echo "SpawnService smoke: PASS"

NPC_OUT="${TMPDIR:-/tmp}/summit_npc_query_smoke"
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/world/v2/tests/npc_query_smoke.cpp" -o "$NPC_OUT"
"$NPC_OUT"
echo "NpcQuery smoke: PASS"

INTERACTION_OUT="${TMPDIR:-/tmp}/summit_interaction_service_smoke"
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/world/v2/tests/interaction_service_smoke.cpp" -o "$INTERACTION_OUT"
"$INTERACTION_OUT"
echo "InteractionService smoke: PASS"

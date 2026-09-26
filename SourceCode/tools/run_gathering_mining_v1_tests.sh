#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX="${CXX:-g++}"
COMMON=(-std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" -I"$ROOT/SrcServer/src")

python3 "$ROOT/tools/validate_gathering_mining_v1.py"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

"$CXX" "${COMMON[@]}" \
  "$ROOT/SrcServer/src/systems/gameplay/gathering/v2/GatheringService.cpp" \
  "$ROOT/Shared/gameplay/gathering/v2/tests/gathering_service_smoke.cpp" \
  -o "$TMP/gathering_smoke"
"$TMP/gathering_smoke"
echo "Gathering domain smoke: PASS"

"$CXX" "${COMMON[@]}" \
  "$ROOT/SrcServer/src/systems/gameplay/gathering/v2/GatheringService.cpp" \
  "$ROOT/SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp" \
  "$ROOT/Shared/gameplay/mining/v2/tests/mining_service_smoke.cpp" \
  -o "$TMP/mining_smoke"
"$TMP/mining_smoke"
echo "Mining + Covenant bridge smoke: PASS"

bash "$ROOT/tools/run_covenant_ui_tests.sh"
echo "Gathering/Mining V1 portable/static validation: PASS"

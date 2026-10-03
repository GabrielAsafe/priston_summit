#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX="${CXX:-g++}"
COMMON=(-std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" -I"$ROOT/SrcServer/src")
python3 "$ROOT/tools/validate_fishing_v2.py"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
"$CXX" "${COMMON[@]}" \
  "$ROOT/SrcServer/src/systems/gameplay/gathering/v2/GatheringService.cpp" \
  "$ROOT/SrcServer/src/systems/gameplay/fishing/v2/FishingService.cpp" \
  "$ROOT/Shared/gameplay/fishing/v2/tests/fishing_service_smoke.cpp" \
  -o "$TMP/fishing_smoke"
"$TMP/fishing_smoke"
echo "Fishing V2 domain/profession/event smoke: PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
BIN="${TMPDIR:-/tmp}/summit_harvest_service_ports_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT/Shared" -I"$ROOT/SrcServer/src" \
  "$ROOT/Shared/gameplay/gathering/v2/tests/harvest_service_ports_smoke.cpp" \
  "$ROOT/SrcServer/src/systems/gameplay/gathering/v2/GatheringService.cpp" -o "$BIN"
"$BIN"
rm -f "$BIN"

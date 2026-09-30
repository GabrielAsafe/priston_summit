#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/combat_buff_service_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" "$ROOT/gameplay/combat/v2/tests/combat_buff_service_smoke.cpp" -o "$OUT"
"$OUT"
echo "combat buff service smoke: PASS"

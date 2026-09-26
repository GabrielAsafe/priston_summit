#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_companion_v1_1_tests"
mkdir -p "$OUT"

python3 "$ROOT/tools/validate_companion_ui_v1_1.py"
python3 "$ROOT/tools/validate_companion_equipment_contract.py"
python3 "$ROOT/tools/validate_companion_visual_runtime_v2.py"
python3 "$ROOT/tools/validate_companion_skill_executor_v1.py"

"$CXX" -std=c++17 -Wall -Wextra -pedantic \
  -I "$ROOT/Shared" \
  "$ROOT/tests/companions/companion_protocol_smoke.cpp" \
  -o "$OUT/companion_protocol_smoke"
"$OUT/companion_protocol_smoke"
echo "Companion protocol smoke: PASS"

bash "$ROOT/SrcGame/src/systems/ui/v2/tests/run_portable_tests.sh"

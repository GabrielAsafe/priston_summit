#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$ROOT/tools/validate_companion_resurrection_authority_v2.py"
python3 "$ROOT/tools/validate_companion_life_boundary_v1.py"
bash "$ROOT/Shared/gameplay/combat/v2/tests/run_life_commands_tests.sh"
echo "Companion resurrection authority v2: PASS"

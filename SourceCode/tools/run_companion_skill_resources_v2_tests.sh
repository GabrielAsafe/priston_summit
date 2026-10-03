#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$ROOT/tools/validate_companion_skill_resources_v2.py"
bash "$ROOT/Shared/gameplay/skills/v2/tests/run_skill_resource_commands_tests.sh"
echo "Companion skill resource authority v2: PASS"

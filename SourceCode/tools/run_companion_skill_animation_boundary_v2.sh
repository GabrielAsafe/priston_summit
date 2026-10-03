#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$ROOT/tools/validate_companion_skill_animation_boundary_v2.py"
"$ROOT/Shared/gameplay/skills/v2/tests/run_skill_animation_commands_tests.sh"

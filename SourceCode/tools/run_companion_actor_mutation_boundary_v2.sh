#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$ROOT/tools/validate_companion_actor_mutation_boundary_v2.py"
"$ROOT/Shared/gameplay/combat/v2/tests/run_targeting_service_tests.sh"

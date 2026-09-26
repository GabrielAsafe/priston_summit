#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$ROOT/tools/validate_covenant_ui.py"
bash "$ROOT/SourceCode/SrcGame/src/systems/ui/v2/tests/run_covenant_ui_tests.sh"
echo "Covenant UI complete portable/static validation: PASS"

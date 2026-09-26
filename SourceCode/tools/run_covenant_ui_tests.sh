#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Também aceita instalação onde este runner seja chamado a partir da raiz summit.
if [[ -d "$ROOT/SourceCode/SrcGame/src" && -d "$ROOT/SourceCode/Shared" ]]; then
  ROOT="$ROOT/SourceCode"
fi

if [[ ! -d "$ROOT/SrcGame/src" || ! -d "$ROOT/Shared" ]]; then
  echo "ERROR: nao foi possivel localizar a raiz SourceCode a partir de $SCRIPT_DIR" >&2
  exit 2
fi

python3 "$ROOT/tools/validate_covenant_ui.py"
bash "$ROOT/SrcGame/src/systems/ui/v2/tests/run_covenant_ui_tests.sh"
echo "Covenant UI complete portable/static validation: PASS"

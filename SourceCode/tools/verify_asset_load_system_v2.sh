#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$(pwd)}"
cd "$ROOT"
python3 tools/validate_asset_load_system_v2.py
bash tools/run_asset_load_v2_portable_tests.sh .
bash -n tools/verify_asset_load_system_v2.sh tools/run_asset_load_v2_portable_tests.sh link_client_mingw_i686.sh
printf '%s\n' 'ASSET_LOAD_SYSTEM_V2=PASS'

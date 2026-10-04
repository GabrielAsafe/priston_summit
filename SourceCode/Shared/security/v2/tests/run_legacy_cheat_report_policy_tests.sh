#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_legacy_cheat_report_policy_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/security/v2/tests/legacy_cheat_report_policy_smoke.cpp" -o "$OUT"
"$OUT"
echo "legacy cheat report policy smoke: PASS"

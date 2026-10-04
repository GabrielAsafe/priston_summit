#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_duplicate_login_policy_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/authentication/v2/tests/duplicate_login_policy_smoke.cpp" -o "$OUT"
"$OUT"
echo "duplicate login policy smoke: PASS"

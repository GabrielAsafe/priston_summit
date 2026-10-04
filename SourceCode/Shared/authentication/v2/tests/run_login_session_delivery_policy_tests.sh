#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_login_session_delivery_policy_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/authentication/v2/tests/login_session_delivery_policy_smoke.cpp" -o "$OUT"
"$OUT"
echo "login session delivery policy smoke: PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_login_request_validation_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/authentication/v2/tests/login_request_validation_smoke.cpp" -o "$OUT"
"$OUT"
echo "login request validation smoke: PASS"

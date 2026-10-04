#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_security_policy_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/security/v2/tests/security_policy_smoke.cpp" -o "$OUT"
"$OUT"
echo "security finding/policy smoke: PASS"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_admin_audit_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$ROOT" \
  "$ROOT/admin/v2/tests/admin_audit_smoke.cpp" -o "$OUT"
"$OUT"
echo "admin audit smoke: PASS"

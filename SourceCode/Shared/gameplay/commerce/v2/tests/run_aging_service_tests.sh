#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
BIN="${TMPDIR:-/tmp}/summit_aging_service_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT/Shared" "$ROOT/Shared/gameplay/commerce/v2/tests/aging_service_smoke.cpp" -o "$BIN"
"$BIN"
rm -f "$BIN"

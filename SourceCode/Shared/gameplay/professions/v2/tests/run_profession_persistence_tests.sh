#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
BIN="${TMPDIR:-/tmp}/summit_profession_persistence_smoke"
"$CXX" -std=c++11 -Wall -Wextra -pedantic -I"$ROOT/Shared" "$ROOT/Shared/gameplay/professions/v2/tests/profession_persistence_smoke.cpp" -o "$BIN"
"$BIN"
rm -f "$BIN"

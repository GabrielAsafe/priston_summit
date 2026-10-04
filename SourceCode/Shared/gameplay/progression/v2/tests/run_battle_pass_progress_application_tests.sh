#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_battle_pass_progress_application_smoke_$$"
trap 'rm -f "$BIN"' EXIT
c++ -std=c++17 -I"$ROOT" "$ROOT/gameplay/progression/v2/tests/battle_pass_progress_application_smoke.cpp" -o "$BIN"
"$BIN"

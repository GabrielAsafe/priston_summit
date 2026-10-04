#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_class_rank_query_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$(cd "$ROOT/.." && pwd)" "$HERE/class_rank_query_smoke.cpp" -o "$BIN"
"$BIN"
rm -f "$BIN"

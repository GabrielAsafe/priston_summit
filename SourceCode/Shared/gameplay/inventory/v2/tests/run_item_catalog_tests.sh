#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/item_catalog_smoke.$$"
trap 'rm -f "$OUT"' EXIT
"$CXX" -std=c++17 -I"$ROOT" "$ROOT/gameplay/inventory/v2/tests/item_catalog_smoke.cpp" -o "$OUT"
"$OUT"

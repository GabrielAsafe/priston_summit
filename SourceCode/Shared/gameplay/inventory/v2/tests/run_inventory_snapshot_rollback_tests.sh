#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/inventory_snapshot_rollback_smoke"
g++ -std=c++17 -I"$ROOT" "$ROOT/gameplay/inventory/v2/tests/inventory_snapshot_rollback_smoke.cpp" -o "$OUT"
"$OUT"

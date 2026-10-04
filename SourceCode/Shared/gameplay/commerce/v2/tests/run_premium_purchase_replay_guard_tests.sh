#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/premium_purchase_replay_guard_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -pthread -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/commerce/v2/tests/premium_purchase_replay_guard_smoke.cpp" -o "$OUT"
"$OUT"

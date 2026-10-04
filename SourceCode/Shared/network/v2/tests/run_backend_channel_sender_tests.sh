#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_backend_channel_sender_smoke"
"$CXX" -std=c++17 -I"$ROOT/Shared" \
  "$ROOT/Shared/network/v2/tests/backend_channel_sender_smoke.cpp" \
  -o "$OUT"
"$OUT"
echo "backend channel sender smoke: PASS"

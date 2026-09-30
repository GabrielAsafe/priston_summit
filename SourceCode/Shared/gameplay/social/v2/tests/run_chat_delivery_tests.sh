#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_chat_delivery_smoke"
"${CXX:-g++}" -std=c++17 -I"$ROOT" \
  "$ROOT/gameplay/social/v2/tests/chat_delivery_smoke.cpp" \
  -o "$OUT"
"$OUT"

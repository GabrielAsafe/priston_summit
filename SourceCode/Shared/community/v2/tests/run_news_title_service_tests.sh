#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_news_title_service_smoke_$$"
trap 'rm -f "$BIN"' EXIT
c++ -std=c++17 -I"$ROOT" "$ROOT/Shared/community/v2/tests/news_title_service_smoke.cpp" -o "$BIN"
"$BIN"

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/summit_news_feed_service_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$(dirname "$ROOT")" \
  "$ROOT/community/v2/tests/news_feed_service_smoke.cpp" -o "$OUT"
"$OUT"
echo "news feed service smoke: PASS"

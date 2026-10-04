#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
BIN="${TMPDIR:-/tmp}/summit_admin_authorization_smoke"
g++ -std=c++17 -Wall -Wextra -pedantic -I"$(cd "$ROOT/.." && pwd)" "$HERE/admin_authorization_service_smoke.cpp" -o "$BIN"
"$BIN"
rm -f "$BIN"

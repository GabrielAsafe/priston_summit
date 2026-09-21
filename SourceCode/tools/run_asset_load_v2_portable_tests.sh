#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$(pwd)}"
cd "$ROOT"
CXX_BIN="${CXX:-g++}"
if ! command -v "$CXX_BIN" >/dev/null 2>&1; then
  echo "Asset Load System V2 portable tests: FAIL (compiler not found: $CXX_BIN)" >&2
  exit 1
fi
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
"$CXX_BIN" -std=c++17 -Wall -Wextra -Werror -IShared \
  tests/assets/asset_load_registry_v2_smoke.cpp \
  -o "$TMP/asset_load_registry_v2_smoke"
"$TMP/asset_load_registry_v2_smoke"
echo 'Asset Load System V2 portable tests: PASS'

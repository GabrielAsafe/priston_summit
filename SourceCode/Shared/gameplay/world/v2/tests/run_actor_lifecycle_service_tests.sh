#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/actor_lifecycle_service_smoke.$$"
trap 'rm -f "$OUT"' EXIT INT TERM
${CXX:-g++} -std=c++11 -I"$ROOT" "$ROOT/gameplay/world/v2/tests/actor_lifecycle_service_smoke.cpp" -o "$OUT"
"$OUT"
echo "actor lifecycle service smoke: PASS"

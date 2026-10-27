#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT HUP INT TERM
c++ -std=gnu++14 -O1 -g -pthread -Wall -Wextra -Werror \
  -I "$ROOT/Shared" \
  "$ROOT/tools/companion_trade_350a_native_test.cpp" \
  -o "$DIR/summit350a_native"
"$DIR/summit350a_native"

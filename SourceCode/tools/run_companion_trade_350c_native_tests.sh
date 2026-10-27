#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
TMPDIR_350C=$(mktemp -d)
trap 'rm -rf "$TMPDIR_350C"' EXIT HUP INT TERM
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
  tools/companion_trade_350c_native_test.cpp -o "$TMPDIR_350C/test"
"$TMPDIR_350C/test"

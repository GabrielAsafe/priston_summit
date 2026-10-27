#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
TMPDIR_350D=$(mktemp -d)
trap 'rm -rf "$TMPDIR_350D"' EXIT HUP INT TERM
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
  tools/companion_trade_350d_native_test.cpp -o "$TMPDIR_350D/test"
"$TMPDIR_350D/test"

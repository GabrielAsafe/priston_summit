#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
  tools/companion_trade_350e_native_test.cpp -o "$T/test_350e"
"$T/test_350e"

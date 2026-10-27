#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
: "${CXX:=g++}"
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT HUP INT TERM
"$CXX" -std=c++14 -pthread -O2 -Wall -Wextra -IShared tools/companion_trade_350q_live_handoff_test.cpp -o "$out/test"
"$out/test"

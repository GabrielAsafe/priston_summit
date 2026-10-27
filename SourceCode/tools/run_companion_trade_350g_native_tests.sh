#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
: "${CXX:=g++}"
D="$(mktemp -d)"
trap 'rm -rf "$D"' EXIT HUP INT TERM
"$CXX" -std=c++17 -O2 -Wall -Wextra -pthread -I Shared tools/companion_trade_350g_external_writer_test.cpp -o "$D/guard"
"$D/guard"

#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
TMPDIR_350F=$(mktemp -d)
trap 'rm -rf "$TMPDIR_350F"' EXIT HUP INT TERM
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
  tools/companion_trade_350f_writer_gate_test.cpp -o "$TMPDIR_350F/test"
"$TMPDIR_350F/test"

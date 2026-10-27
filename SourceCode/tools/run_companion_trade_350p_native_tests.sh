#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
BIN="$(mktemp "${TMPDIR:-/tmp}/summit350p.XXXXXXXX")"
trap 'rm -f "$BIN"' EXIT HUP INT TERM
${CXX:-g++} -std=gnu++14 -Wall -Wextra -Werror -IShared \
    tools/companion_trade_350p_preview_test.cpp -o "$BIN"
"$BIN"

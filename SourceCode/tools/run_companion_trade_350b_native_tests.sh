#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
TMPDIR_350B=$(mktemp -d)
trap 'rm -rf "$TMPDIR_350B"' EXIT HUP INT TERM
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
  tools/companion_trade_350b_native_test.cpp -o "$TMPDIR_350B/test"
"$TMPDIR_350B/test"
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
  tools/companion_trade_350b_gate_test.cpp -o "$TMPDIR_350B/gate"
"$TMPDIR_350B/gate"
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
  tools/companion_trade_350b_journal_test.cpp -o "$TMPDIR_350B/journal"
"$TMPDIR_350B/journal"

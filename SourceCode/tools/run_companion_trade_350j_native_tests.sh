#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
sh tools/run_companion_trade_350i_native_tests.sh
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
  tools/companion_trade_350j_end_to_end_test.cpp \
  Shared/gameplay/companions/v2/CompanionTradeExactRecordStore.cpp \
  Shared/gameplay/companions/v2/CompanionTradeExactPairBackend.cpp \
  Shared/gameplay/companions/v2/CompanionTradeDiskJournal.cpp \
  -o "$T/trade_350j"
mkdir -m 700 "$T/disposable"
"$T/trade_350j" "$T/disposable"

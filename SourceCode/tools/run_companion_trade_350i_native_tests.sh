#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
sh tools/run_companion_trade_350h_native_tests.sh
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
    tools/companion_trade_350i_witness_test.cpp -o "$T/350i_witness"
"$T/350i_witness"
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
    tools/companion_trade_350i_disk_recovery_test.cpp \
    Shared/gameplay/companions/v2/CompanionTradeExactRecordStore.cpp \
    Shared/gameplay/companions/v2/CompanionTradeExactPairBackend.cpp \
    Shared/gameplay/companions/v2/CompanionTradeDiskJournal.cpp \
    -o "$T/350i_real_disk"
mkdir -m 700 "$T/disposable"
"$T/350i_real_disk" "$T/disposable"

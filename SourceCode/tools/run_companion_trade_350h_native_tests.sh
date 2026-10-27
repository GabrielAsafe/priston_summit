#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
sh tools/run_companion_trade_350a_native_tests.sh
sh tools/run_companion_trade_350b_native_tests.sh
sh tools/run_companion_trade_350c_native_tests.sh
sh tools/run_companion_trade_350d_native_tests.sh
sh tools/run_companion_trade_350e_native_tests.sh
sh tools/run_companion_trade_350f_native_tests.sh
sh tools/run_companion_trade_350g_native_tests.sh
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
"${CXX:-g++}" -std=c++14 -pthread -Wall -Wextra -Werror -I Shared \
    tools/companion_trade_350h_lifetime_test.cpp -o "$T/350h_lifetime"
"$T/350h_lifetime"
python3 tools/audit_companion_trade_350h.py
printf '%s\n' 'SUMMIT350H_NATIVE=PASS (journal nonce cross-intent refusal; previous 350A-G tests; static checks)'

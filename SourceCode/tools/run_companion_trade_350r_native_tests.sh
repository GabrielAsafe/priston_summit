#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
: "${CXX:=g++}"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT HUP INT TERM
"$CXX" -std=c++17 -O2 -Wall -Wextra -ISrcGame/src -IShared \
  SrcGame/src/systems/ui/v2/core/Widget.cpp \
  SrcGame/src/systems/ui/v2/widgets/Button.cpp \
  tools/companion_trade_350r_ui_test.cpp -o "$TMPDIR/350r_ui"
"$TMPDIR/350r_ui"
"$CXX" -std=c++17 -fsyntax-only -Wall -Wextra -ISrcGame/src -IShared \
  SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp
python3 tools/audit_companion_trade_350r.py
sh tools/run_companion_trade_350q_native_tests.sh
printf '%s\n' 'SUMMIT350R_NATIVE=PASS (real icons, disabled item visible, UI syntax, live save barrier)'

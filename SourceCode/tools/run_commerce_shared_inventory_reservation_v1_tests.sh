#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$ROOT/tools/validate_commerce_shared_inventory_reservation_v1.py"
python3 "$ROOT/tools/validate_trade_inventory_reservation_v1.py"
python3 "$ROOT/tools/validate_personal_shop_atomic_inventory_v1.py"
python3 "$ROOT/tools/validate_personal_shop_transaction_boundary_v1.py"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/inventory/v2/tests/inventory_reservation_smoke.cpp" \
  -o "$TMP/inventory_reservation_smoke"
"$TMP/inventory_reservation_smoke"
echo "Commerce shared inventory reservation v1: PASS"

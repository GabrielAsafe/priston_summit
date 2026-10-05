#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

run() {
    echo ">>> $*"
    "$@"
}

# Wallet authority is a release-level contract, not just a transaction detail.
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_wallet_tests.sh"
run python3 "$ROOT/tools/validate_wallet_boundary_v1.py"
run python3 "$ROOT/tools/validate_wallet_mutation_ownership_v2.py"
run python3 "$ROOT/tools/validate_coin_wallet_mutation_ownership_v1.py"

# Core transaction/session services.
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_trade_service_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_trade_session_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_trade_transaction_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_shop_service_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_shop_transaction_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_personal_shop_service_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_personal_shop_transaction_tests.sh"

# Premium/restoration consistency and replay handling.
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_premium_entitlement_service_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_premium_purchase_transaction_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_premium_purchase_replay_guard_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_premium_purchase_replay_store_contract_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_restoration_pricing_policy_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_restoration_transaction_tests.sh"

# Crafting-family domain services/policies.
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_crafting_service_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_mixing_service_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_aging_service_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_aging_quest_mode_policy_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_aging_result_log_policy_tests.sh"
run bash "$ROOT/Shared/gameplay/commerce/v2/tests/run_aging_stone_kind_policy_tests.sh"

# Cross-domain reservation/session authority.
run bash "$ROOT/tools/run_commerce_shared_inventory_reservation_v1_tests.sh"
run bash "$ROOT/tools/run_npc_shop_inventory_reservation_v1_tests.sh"
run bash "$ROOT/tools/run_trade_inventory_reservation_v1_tests.sh"
run bash "$ROOT/tools/run_personal_shop_session_v2_authority_tests.sh"

echo "COMMERCE_V2_PORTABLE=PASS"

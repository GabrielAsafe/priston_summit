#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX="${CXX:-g++}"
TMP="${TMPDIR:-/tmp}"
run() { echo ">>> $*"; "$@"; }
compile_run() {
    local source="$1"
    local name="$2"
    local out="$TMP/summit_${name}_$$"
    "$CXX" -std=c++17 -Wall -Wextra -Werror -I"$ROOT/Shared" "$source" -o "$out"
    "$out"
    rm -f "$out"
}

compile_run "$ROOT/Shared/gameplay/social/clan/v2/tests/clan_membership_service_smoke.cpp" clan_membership_service_smoke
compile_run "$ROOT/Shared/gameplay/social/clan/v2/tests/clan_query_smoke.cpp" clan_query_smoke
compile_run "$ROOT/Shared/gameplay/social/clan/v2/tests/clan_chat_service_smoke.cpp" clan_chat_service_smoke
compile_run "$ROOT/Shared/gameplay/social/clan/v2/tests/clan_update_notifier_smoke.cpp" clan_update_notifier_smoke

run python3 "$ROOT/tools/validate_clan_membership_mutation_ownership_v1.py"
run python3 "$ROOT/tools/validate_clan_query_boundary_v1.py"
run python3 "$ROOT/tools/validate_clan_query_ownership_v2.py"
run python3 "$ROOT/tools/validate_clan_chat_boundary_v1.py"
run python3 "$ROOT/tools/validate_clan_update_notifier_boundary_v1.py"
run python3 "$ROOT/tools/validate_clan_service_packet_sender_v1.py"
run python3 "$ROOT/tools/validate_clan_custom_query_boundary_v1.py"
run python3 "$ROOT/tools/validate_clan_custom_packet_sender_v2.py"

echo "CLAN_V2_PORTABLE=PASS"

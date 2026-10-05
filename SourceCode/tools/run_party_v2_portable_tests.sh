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

compile_run "$ROOT/Shared/gameplay/social/party/v2/tests/party_commands_smoke.cpp" party_commands_smoke
compile_run "$ROOT/Shared/gameplay/social/party/v2/tests/party_query_smoke.cpp" party_query_smoke
compile_run "$ROOT/Shared/gameplay/social/party/v2/tests/party_relay_service_smoke.cpp" party_relay_service_smoke

run python3 "$ROOT/tools/validate_party_membership_mutation_ownership_v2.py"
run python3 "$ROOT/tools/validate_party_commands_join_boundary_v1.py"
run python3 "$ROOT/tools/validate_party_control_commands_boundary_v1.py"
run python3 "$ROOT/tools/validate_party_membership_sync_boundary_v1.py"
run python3 "$ROOT/tools/validate_party_release_sync_boundary_v1.py"
run python3 "$ROOT/tools/validate_party_potion_relay_boundary_v1.py"
run python3 "$ROOT/tools/validate_party_packet_sender_v2.py"
run python3 "$ROOT/tools/validate_client_party_transport_retired_v3.py"

echo "PARTY_V2_PORTABLE=PASS"

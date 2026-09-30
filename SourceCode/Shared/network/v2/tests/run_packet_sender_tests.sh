#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/summit_packet_sender_smoke"
"$CXX" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
    "$ROOT/Shared/network/v2/tests/packet_sender_smoke.cpp" -o "$OUT"
"$OUT"
echo "PacketSender smoke: PASS"

CLIENT_COMMAND_OUT="${TMPDIR:-/tmp}/summit_client_command_sender_smoke"
"$CXX" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
    "$ROOT/Shared/network/v2/tests/client_command_sender_smoke.cpp" -o "$CLIENT_COMMAND_OUT"
"$CLIENT_COMMAND_OUT"
echo "ClientCommandSender smoke: PASS"

ROUTER_OUT="${TMPDIR:-/tmp}/summit_packet_router_smoke"
"$CXX" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
    "$ROOT/Shared/network/v2/tests/packet_router_smoke.cpp" -o "$ROUTER_OUT"
"$ROUTER_OUT"
echo "PacketRouter smoke: PASS"

CONNECTION_OUT="${TMPDIR:-/tmp}/summit_connection_query_smoke"
"$CXX" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
    "$ROOT/Shared/network/v2/tests/connection_query_smoke.cpp" -o "$CONNECTION_OUT"
"$CONNECTION_OUT"
echo "ConnectionQuery smoke: PASS"

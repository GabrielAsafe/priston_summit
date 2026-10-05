#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CASES = (
    (ROOT / "SrcGame/src/systems/network/client/netplay.cpp", "SendClientLegacySocket", "ClientLegacyCommandSender().SendToLegacySocket"),
    (ROOT / "SrcServer/src/systems/network/server_gameplay/netplay.cpp", "SendServerLegacySocket", "ServerLegacyPacketSender().SendToLegacySocket"),
)
DIRECT = re.compile(r"->\s*Send\s*\(")

failures = []
for path, helper, delegation in CASES:
    text = path.read_text(encoding="utf-8", errors="replace")
    if DIRECT.search(text):
        failures.append(f"{path.relative_to(ROOT)} still performs direct socket sends")
    if f"int {helper}(smWINSOCK* socket, const void* data, int size)" not in text:
        failures.append(f"{path.relative_to(ROOT)} missing frozen-legacy transport helper")
    if delegation not in text:
        failures.append(f"{path.relative_to(ROOT)} does not delegate arbitrary legacy sockets to AMBER transport")

client_header = (ROOT / "SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.h").read_text(errors="replace")
client_adapter = (ROOT / "SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp").read_text(errors="replace")
server_adapter = (ROOT / "SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp").read_text(errors="replace")
if "SendToLegacySocket(smWINSOCK* socket" not in client_header:
    failures.append("client AMBER transport does not expose the frozen legacy-socket bridge")
if "LegacyClientCommandSender::SendToLegacySocket" not in client_adapter or "socket->Send" not in client_adapter:
    failures.append("client AMBER transport does not own the actual socket Send")
if "LegacyPacketSender::SendToLegacySocket" not in server_adapter or "socket->Send" not in server_adapter:
    failures.append("server AMBER transport lost arbitrary legacy-socket ownership")

if failures:
    for failure in failures:
        print("FAIL: " + failure, file=sys.stderr)
    raise SystemExit(1)
print("netplay socket transport retired v3: PASS (client/server RED direct ->Send=0)")

#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CASES = (
    (
        ROOT / "SrcGame/src/systems/network/client/netplay.cpp",
        "ClientLegacyBackendChannelSender",
        "SendClientBackendPacket",
    ),
    (
        ROOT / "SrcServer/src/systems/network/server_gameplay/netplay.cpp",
        "ServerLegacyBackendChannelSender",
        "SendServerBackendPacket",
    ),
)
DIRECT = re.compile(r"\b(smWsockDataServer|smWsockUserServer|smWsockExtendServer)\s*->\s*Send\s*\(")

failures = []
for path, accessor, helper in CASES:
    text = path.read_text(encoding="utf-8", errors="replace")
    direct = DIRECT.findall(text)
    if direct:
        failures.append(f"{path.relative_to(ROOT)} still owns backend socket sends: {sorted(set(direct))}")
    if f"int {helper}(gameplay::network::BackendChannel channel, const void* data, int size)" not in text:
        failures.append(f"{path.relative_to(ROOT)} missing typed backend compatibility helper")
    if f"{accessor}().Send(" not in text:
        failures.append(f"{path.relative_to(ROOT)} does not delegate backend transport to AMBER sender")
    for channel in ("Data", "User", "Extend"):
        if f"BackendChannel::{channel}" not in text:
            failures.append(f"{path.relative_to(ROOT)} lost {channel} channel routing")

if failures:
    for failure in failures:
        print("FAIL: " + failure, file=sys.stderr)
    raise SystemExit(1)

print("netplay backend transport retired v3: PASS (Data/User/Extend direct sends=0 on client+server)")

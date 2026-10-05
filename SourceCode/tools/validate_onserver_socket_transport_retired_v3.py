#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CASES = (
    (ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "ServerLegacyPacketSender().SendToLegacySocket"),
    (ROOT / "Shared/server/legacy_runtime/OnSever_legacy_client.inl", "ClientLegacyCommandSender().SendToLegacySocket"),
)
DIRECT = re.compile(r"->\s*Send\s*\(")
failures = []
for path, delegation in CASES:
    text = path.read_text(encoding="utf-8", errors="replace")
    if DIRECT.search(text):
        failures.append(f"{path.relative_to(ROOT)} still performs direct socket sends")
    if "SendOnServerLegacySocket" not in text or delegation not in text:
        failures.append(f"{path.relative_to(ROOT)} does not delegate frozen socket transport to AMBER")
if failures:
    for failure in failures:
        print("FAIL: " + failure, file=sys.stderr)
    raise SystemExit(1)
print("OnServer socket transport retired v3: PASS (active client/server legacy mirrors direct ->Send=0)")

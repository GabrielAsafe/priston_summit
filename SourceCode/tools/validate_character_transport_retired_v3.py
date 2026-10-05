#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
p=ROOT/"Shared/gameplay/character/character_legacy_client.inl"
t=p.read_text(errors="replace")
pat=re.compile(r"\b(?:lpsmSock|smWsock[A-Za-z0-9_]*)\s*->\s*Send\s*\(")
fail=[]
if pat.search(t): fail.append("tracked direct legacy socket send remains in client character kernel")
for token in ("SendCharacterLegacySocket", "ClientLegacyCommandSender().SendToLegacySocket", "SendCharacterDataPacket", "ClientLegacyBackendChannelSender().Send"):
    if token not in t: fail.append("missing delegation token: "+token)
if fail:
    print("\n".join("FAIL: "+x for x in fail), file=sys.stderr); raise SystemExit(1)
print("character transport retired v3: PASS (tracked direct socket sends=0)")

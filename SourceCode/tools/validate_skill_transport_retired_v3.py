#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
cases=(
 (ROOT/"Shared/gameplay/skills/SkillSub_legacy_client.inl", "ClientLegacyCommandSender().SendToLegacySocket", "ClientLegacyBackendChannelSender().Send"),
 (ROOT/"Shared/gameplay/skills/SkillSub_legacy_server.inl", "ServerLegacyPacketSender().SendToLegacySocket", "ServerLegacyBackendChannelSender().Send"),
)
pat=re.compile(r"\b(?:lpsmSock|smWsockDataServer)\s*->\s*Send\s*\(")
fail=[]
for path,socket_owner,data_owner in cases:
    text=path.read_text(errors="replace")
    if pat.search(text): fail.append(path.name+": direct tracked socket send remains")
    if socket_owner not in text: fail.append(path.name+": socket AMBER delegation missing")
    if data_owner not in text or "BackendChannel::Data" not in text: fail.append(path.name+": data-channel AMBER delegation missing")
if fail:
    print("\n".join("FAIL: "+x for x in fail),file=sys.stderr); raise SystemExit(1)
print("skill transport retired v3: PASS (active client/server SkillSub direct tracked sends=0)")

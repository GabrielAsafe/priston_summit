#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
cases=(
 (ROOT/"Shared/persistence/character_record/record_legacy_client.inl", "ClientLegacyCommandSender().SendToLegacySocket"),
 (ROOT/"Shared/persistence/character_record/record_legacy_server.inl", "ServerLegacyPacketSender().SendToLegacySocket"),
)
fail=[]
for path,owner in cases:
    text=path.read_text(errors="replace")
    if re.search(r"\blpsmSock\s*->\s*Send\s*\(",text): fail.append(path.name+": direct lpsmSock send remains")
    if "SummitRecordSocketSend" not in text or owner not in text: fail.append(path.name+": AMBER socket delegation missing")
if fail:
    print("\n".join("FAIL: "+x for x in fail),file=sys.stderr); raise SystemExit(1)
print("character record transport retired v3: PASS (client/server direct lpsmSock sends=0)")

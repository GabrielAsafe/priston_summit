#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
cases=(
 (ROOT/"SrcGame/src/compat/client_server/server_legacy/svrEventSpawn.cpp", "SummitClientEventSpawnSocketSend"),
 (ROOT/"SrcGame/src/compat/client_server/server_legacy/svrEventGame.h", "SummitClientEventGameSocketSend"),
)
fail=[]
for path,helper in cases:
    text=path.read_text(errors="replace")
    if re.search(r"\blpsmSock\s*->\s*Send\s*\(",text): fail.append(path.name+": direct lpsmSock send remains")
    if helper not in text or "ClientLegacyCommandSender().SendToLegacySocket" not in text: fail.append(path.name+": AMBER delegation missing")
if fail:
    print("\n".join("FAIL: "+x for x in fail),file=sys.stderr); raise SystemExit(1)
print("client event transport retired v3: PASS (spawn/game direct lpsmSock sends=0)")

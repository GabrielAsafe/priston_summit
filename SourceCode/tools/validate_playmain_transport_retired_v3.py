#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
cases=(
 (ROOT/"SrcGame/src/core/gameplay/runtime/playmain.cpp", "SummitPlayMainClientSocketSend", "ClientLegacyCommandSender().SendToLegacySocket"),
 (ROOT/"SrcServer/src/core/server/gameplay_runtime/playmain.cpp", "SummitPlayMainServerSocketSend", "ServerLegacyPacketSender().SendToLegacySocket"),
)
fail=[]
for path,helper,owner in cases:
    text=path.read_text(errors="replace")
    if re.search(r"\blpsmSock\s*->\s*Send\s*\(", text): fail.append(path.name+": direct lpsmSock send remains")
    if helper not in text or owner not in text: fail.append(path.name+": AMBER delegation missing")
if fail:
    print("\n".join("FAIL: "+x for x in fail),file=sys.stderr); raise SystemExit(1)
print("playmain transport retired v3: PASS (client/server direct lpsmSock sends=0)")

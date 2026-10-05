#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
p=ROOT/"SrcGame/src/compat/client_server/server_legacy/PartySever.cpp"
t=p.read_text(errors="replace")
fail=[]
if re.search(r"\blpsmSock\s*->\s*Send\s*\(",t): fail.append("direct party socket send remains")
for token in ("SummitPartyLegacySocketSend", "ClientLegacyCommandSender().SendToLegacySocket"):
    if token not in t: fail.append("missing delegation token: "+token)
if fail:
    print("\n".join("FAIL: "+x for x in fail),file=sys.stderr); raise SystemExit(1)
print("client party transport retired v3: PASS (direct lpsmSock sends=0)")

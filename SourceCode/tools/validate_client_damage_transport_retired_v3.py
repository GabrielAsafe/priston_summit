#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
p=ROOT/"Shared/gameplay/combat/Svr_Damge_legacy_client.inl"
t=p.read_text(errors="replace")
fail=[]
if re.search(r"\blpsmSock\s*->\s*Send\s*\(", t):
    fail.append("direct lpsmSock send remains in active client damage mirror")
for token in ("SendDamageLegacySocket", "ClientLegacyCommandSender().SendToLegacySocket"):
    if token not in t: fail.append("missing transport delegation token: "+token)
if fail:
    print("\n".join("FAIL: "+x for x in fail), file=sys.stderr); raise SystemExit(1)
print("client damage transport retired v3: PASS (direct lpsmSock sends=0)")

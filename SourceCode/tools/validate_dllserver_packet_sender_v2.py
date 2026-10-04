#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcServer/src/core/server/legacy_runtime/DllServer.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcServer/src/core/server/legacy_runtime/DllServer.cpp"')
frag=base[start:start+240]
checks=[
 ('packet sender included','systems/network/v2/legacy/LegacyPacketSender.h' in src),
 ('helper uses packet sender','ServerLegacyPacketSender().Send(' in src),
 ('three replies use helper',src.count('SummitDllServerSend(')==4),
 ('no direct player socket send','lpsmSock->Send' not in src),
 ('budget retired','"lpsmSock": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'dllserver packet sender v2: PASS ({len(checks)}/{len(checks)})')

#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcServer/src/systems/gameplay/events/server_legacy/svrEventSpawn.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
frag=base[base.find('"SrcServer/src/systems/gameplay/events/server_legacy/svrEventSpawn.cpp"'):]
frag=frag[:260]
checks=[
 ('packet sender included','systems/network/v2/legacy/LegacyPacketSender.h' in src),
 ('event helper uses packet sender','ServerLegacyPacketSender().Send(' in src),
 ('four sends routed through helper',src.count('SummitEventSpawnSend(')==5),
 ('no direct player socket send','lpsmSock->Send' not in src),
 ('budget retired','"lpsmSock": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'event spawn packet sender v2: PASS ({len(checks)}/{len(checks)})')

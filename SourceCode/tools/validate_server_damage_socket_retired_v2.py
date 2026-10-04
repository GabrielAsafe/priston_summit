#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'Shared/gameplay/combat/Svr_Damge_legacy_server.inl').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"Shared/gameplay/combat/Svr_Damge_legacy_server.inl"')
frag=base[start:start+250]
checks=[
 ('all 41 active sends use packet helper',src.count('SummitDamagePlayerSend(')==42),
 ('no direct player socket send remains','lpsmSock->Send' not in src),
 ('budget fully retired','"lpsmSock": 0' in frag),
 ('legacy socket alias retained only for legacy APIs','lpsmSock = lpPlayInfo->lpsmSock;' in src),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'server damage socket retirement v2: PASS ({len(checks)}/{len(checks)})')

#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'Shared/gameplay/combat/Svr_Damge_legacy_server.inl').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"Shared/gameplay/combat/Svr_Damge_legacy_server.inl"')
frag=base[start:start+250]
checks=[
 ('packet sender included','systems/network/v2/legacy/LegacyPacketSender.h' in src),
 ('damage helper uses packet sender','ServerLegacyPacketSender().Send(' in src),
 ('player sends remain routed through helper',src.count('SummitDamagePlayerSend(') >= 33),
 ('lpPlayInfo direct sends retired','lpPlayInfo->lpsmSock->Send' not in src),
 ('lpPlayInfo2 direct sends retired','lpPlayInfo2->lpsmSock->Send' not in src),
 ('broadcast player direct send retired','rsPlayInfo[cnt2].lpsmSock->Send' not in src),
 ('direct socket count does not exceed intermediate ceiling',src.count('lpsmSock->Send') <= 10),
 ('budget does not exceed intermediate ceiling',('"lpsmSock": 10' in frag) or ('"lpsmSock": 0' in frag)),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'server damage player packet sender v2: PASS ({len(checks)}/{len(checks)})')

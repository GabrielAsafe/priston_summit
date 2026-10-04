#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'Shared/security/integrity/ClientFuncPos_legacy_server.inl').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"Shared/security/integrity/ClientFuncPos_legacy_server.inl"')
frag=base[start:start+230]
checks=[
 ('packet sender included','systems/network/v2/legacy/LegacyPacketSender.h' in src),
 ('integrity helper uses packet sender','ServerLegacyPacketSender().Send(' in src),
 ('two integrity sends use helper',src.count('SummitIntegrityPacketSend(')==3),
 ('no direct player socket send','lpsmSock->Send' not in src),
 ('budget retired','"lpsmSock": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'integrity packet sender v2: PASS ({len(checks)}/{len(checks)})')

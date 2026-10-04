#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
contract=(ROOT/'Shared/network/v2/BackendChannelSender.h').read_text(errors='ignore')
client=(ROOT/'SrcGame/src/systems/network/v2/legacy/LegacyBackendChannelSender.cpp').read_text(errors='ignore')
server=(ROOT/'SrcServer/src/systems/network/v2/legacy/LegacyBackendChannelSender.cpp').read_text(errors='ignore')
checks=[
 ('green backend channel contract','class IBackendChannelSender' in contract),
 ('three explicit backend channels',all(t in contract for t in ('Data = 0','User','Extend'))),
 ('green contract has no legacy socket names','smWsock' not in contract and 'smWINSOCK' not in contract),
 ('client adapter owns data/user/extend sockets',all(t in client for t in ('smWsockDataServer->Send','smWsockUserServer->Send','smWsockExtendServer->Send'))),
 ('server adapter owns data/user/extend sockets',all(t in server for t in ('smWsockDataServer->Send','smWsockUserServer->Send','smWsockExtendServer->Send'))),
 ('adapters expose target-specific composition roots','ClientLegacyBackendChannelSender()' in client and 'ServerLegacyBackendChannelSender()' in server),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

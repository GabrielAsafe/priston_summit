#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
pairs=[
 ('SrcGame/src/systems/combat/damage/Damage.cpp','ClientLegacyBackendChannelSender'),
 ('SrcServer/src/systems/combat/damage/Damage.cpp','ServerLegacyBackendChannelSender'),
]
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
checks=[]
for rel,sender in pairs:
 src=(ROOT/rel).read_text(errors='ignore')
 i=base.find(f'"{rel}"'); frag=base[i:i+380]
 checks += [
  (f'{rel} guards User against primary alias', src.count('BackendChannel::User') >= 4 and src.count('AvailableAndDistinctFromPrimary') >= 4),
  (f'{rel} guards Extend against primary alias', src.count('BackendChannel::Extend') >= 4),
  (f'{rel} direct User sends retired','smWsockUserServer->Send' not in src),
  (f'{rel} direct Extend sends retired','smWsockExtendServer->Send' not in src),
  (f'{rel} User budget retired','"smWsockUserServer": 0' in frag),
  (f'{rel} Extend budget retired','"smWsockExtendServer": 0' in frag),
 ]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'damage user/extend backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

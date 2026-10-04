#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
pairs=[
 ('SrcGame/src/systems/commerce/legacy/sinShop.cpp','ClientLegacyBackendChannelSender'),
 ('SrcServer/src/systems/commerce/legacy/sinShop.cpp','ServerLegacyBackendChannelSender'),
]
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
checks=[]
for rel,sender in pairs:
 src=(ROOT/rel).read_text(errors='ignore')
 start=base.find(f'"{rel}"'); frag=base[start:start+260]
 checks += [
  (f'{rel} uses backend sender', sender in src and 'BackendChannel::Data' in src),
  (f'{rel} direct Data send retired','smWsockDataServer->Send' not in src),
  (f'{rel} preserves success mapping','.Succeeded() ? TRUE : FALSE' in src),
  (f'{rel} Data budget retired','"smWsockDataServer": 0' in frag),
 ]
for name,ok in checks: print(('PASS' if ok else 'FAIL')+': '+name)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'shop backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

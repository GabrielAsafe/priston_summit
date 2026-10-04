#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcGame/src/systems/commerce/premium/HaPremiumItem.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcGame/src/systems/commerce/premium/HaPremiumItem.cpp"')
frag=base[start:start+260]
checks=[
 ('premium includes backend channel sender','LegacyBackendChannelSender.h' in src),
 ('premium use-item packet delegates to data channel','BackendChannel::Data' in src and 'PacketView(&smPacket' in src),
 ('direct data socket send retired','smWsockDataServer->Send' not in src),
 ('legacy data socket symbol retired','smWsockDataServer' not in src),
 ('socket budget retired','"smWsockDataServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'premium backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

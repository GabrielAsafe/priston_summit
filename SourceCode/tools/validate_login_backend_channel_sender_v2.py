#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcGame/src/systems/authentication/Login.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcGame/src/systems/authentication/Login.cpp"')
frag=base[start:start+220]
checks=[
 ('login includes backend channel adapter','LegacyBackendChannelSender.h' in src),
 ('login queries data channel availability','BackendChannel::Data' in src and '.Available(' in src),
 ('login sends packet through backend service','backend.Send(' in src and 'PacketView(&packet' in src),
 ('login no direct data socket send','smWsockDataServer->Send' not in src),
 ('login no legacy data socket dependency','smWsockDataServer' not in src),
 ('socket budget retired','"smWsockDataServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'login backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

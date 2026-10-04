#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcServer/src/systems/inventory/legacy/sinInvenTory.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcServer/src/systems/inventory/legacy/sinInvenTory.cpp"')
frag=base[start:start+360]
checks=[
 ('server inventory includes backend sender','LegacyBackendChannelSender.h' in src),
 ('party sync uses user backend channel','BackendChannel::User' in src and 'InterfaceParty.PartyMemberCount' in src),
 ('character sync uses data backend channel','BackendChannel::Data' in src),
 ('direct data socket sends retired','smWsockDataServer->Send' not in src),
 ('direct user socket sends retired','smWsockUserServer->Send' not in src),
 ('data budget retired','"smWsockDataServer": 0' in frag),
 ('user budget retired','"smWsockUserServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'server inventory backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

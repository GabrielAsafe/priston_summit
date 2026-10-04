#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp"')
frag=base[start:start+360]
checks=[
 ('inventory includes backend channel sender','LegacyBackendChannelSender.h' in src),
 ('inventory backend helper exists','void SendInventoryBackend(' in src),
 ('inventory data operations delegate',src.count('BackendChannel::Data') >= 16),
 ('inventory party sync delegates to user channel','BackendChannel::User' in src and 'InterfaceParty.PartyMemberCount' in src),
 ('direct data socket sends retired','smWsockDataServer->Send' not in src),
 ('direct user socket sends retired','smWsockUserServer->Send' not in src),
 ('legacy backend socket symbols retired','smWsockDataServer' not in src and 'smWsockUserServer' not in src),
 ('data budget retired','"smWsockDataServer": 0' in frag),
 ('user budget retired','"smWsockUserServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'inventory backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcGame/src/systems/ui/messages/legacy_sin/sinMessageBox.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcGame/src/systems/ui/messages/legacy_sin/sinMessageBox.cpp"')
frag=base[start:start+340]
checks=[
 ('message box includes backend sender','LegacyBackendChannelSender.h' in src),
 ('four user-channel flows delegate',src.count('BackendChannel::User') >= 4),
 ('battle-points data flow delegates','BackendChannel::Data' in src and 'UPDATE_BATTLE_POINTS' in src),
 ('direct user sends retired','smWsockUserServer->Send' not in src),
 ('direct data sends retired','smWsockDataServer->Send' not in src),
 ('user budget retired','"smWsockUserServer": 0' in frag),
 ('data budget retired','"smWsockDataServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'message box backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

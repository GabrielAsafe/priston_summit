#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcGame/src/systems/progression/battle_pass/BattlePass.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcGame/src/systems/progression/battle_pass/BattlePass.cpp"')
frag=base[start:start+280]
checks=[
 ('battle pass includes backend sender','LegacyBackendChannelSender.h' in src),
 ('battle pass claim uses user channel','BackendChannel::User' in src and 'PacketView(&packet' in src),
 ('direct user socket send retired','smWsockUserServer->Send' not in src),
 ('socket budget retired','"smWsockUserServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'battle pass backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

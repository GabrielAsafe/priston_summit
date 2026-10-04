#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcGame/src/systems/security/anticheat/AntiCheat.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcGame/src/systems/security/anticheat/AntiCheat.cpp"')
frag=base[start:start+240]
checks=[
 ('anti-cheat includes backend sender','LegacyBackendChannelSender.h' in src),
 ('anti-cheat data packet helper exists','SendAntiCheatDataPacket' in src),
 ('anti-cheat preserves availability check','AntiCheatDataChannelAvailable()' in src and 'exit(0);' in src),
 ('three cheat reports delegate',src.count('SendAntiCheatDataPacket(&sPacket') == 3),
 ('direct data socket sends retired','smWsockDataServer->Send' not in src),
 ('legacy data socket symbol retired','smWsockDataServer' not in src),
 ('socket budget retired','"smWsockDataServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'anticheat backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

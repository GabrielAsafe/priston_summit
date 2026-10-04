#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
files=[
 ROOT/'SrcGame/src/systems/combat/damage/Damage.cpp',
 ROOT/'SrcServer/src/systems/combat/damage/Damage.cpp',
]
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
checks=[]
for path in files:
 src=path.read_text(errors='ignore')
 rel=path.relative_to(ROOT).as_posix()
 start=base.find(f'"{rel}"'); frag=base[start:start+320]
 checks.extend([
   (f'{path.name}@{path.parts[-5]} includes backend sender','LegacyBackendChannelSender.h' in src),
   (f'{path.name}@{path.parts[-5]} has two data-channel sends',src.count('BackendChannel::Data') >= 2),
   (f'{path.name}@{path.parts[-5]} direct data sends retired','smWsockDataServer->Send' not in src),
   (f'{path.name}@{path.parts[-5]} data budget retired','"smWsockDataServer": 0' in frag),
 ])
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'damage data backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

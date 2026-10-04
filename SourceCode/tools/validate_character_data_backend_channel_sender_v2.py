#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
rel='SrcServer/src/systems/gameplay/character/core/character.cpp'
src=(ROOT/rel).read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
i=base.find(f'"{rel}"'); frag=base[i:i+300]
checks=[
 ('character core includes backend sender','LegacyBackendChannelSender.h' in src),
 ('warning packet routes through Data channel','OPCODE_WARNING_BLINK_ATT' in src and 'ServerLegacyBackendChannelSender().Send' in src and 'BackendChannel::Data' in src),
 ('character core direct Data send retired','smWsockDataServer->Send' not in src),
 ('character core Data budget retired','"smWsockDataServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'character data backend channel sender v2: PASS ({len(checks)}/{len(checks)})')

#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'SrcServer/src/systems/combat/skills/legacy/sinSkill_Info.cpp'
BASELINE = ROOT / 'tools/architecture/direct_socket_send_baseline.json'
text = SRC.read_text(errors='ignore')
base = BASELINE.read_text(errors='ignore')
checks = [
    ('legacy packet sender included', 'systems/network/v2/legacy/LegacyPacketSender.h' in text),
    ('helper uses server packet sender', 'ServerLegacyPacketSender().Send(' in text),
    ('all eight skill payloads use helper', text.count('SummitSkillInfoSend(lpPlayInfo, &') == 8),
    ('no direct player socket sends remain', 'lpsmSock->Send' not in text),
    ('socket budget retired', '"SrcServer/src/systems/combat/skills/legacy/sinSkill_Info.cpp"' in base and '"lpsmSock": 0' in base[base.find('"SrcServer/src/systems/combat/skills/legacy/sinSkill_Info.cpp"'):base.find('"SrcServer/src/systems/combat/skills/legacy/sinSkill_Info.cpp"')+240]),
]
failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}: {name}")
if failed:
    raise SystemExit(1)
print(f'skill info packet sender v2: PASS ({len(checks)}/{len(checks)})')

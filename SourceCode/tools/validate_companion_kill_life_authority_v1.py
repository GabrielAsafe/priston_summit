#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
p=ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp'
s=p.read_text()
start=s.index('void KillEntity(CompanionSlot &slot)')
end=s.index('\n}\n\n} // namespace', start)+2
block=s[start:end]
checks=[
 ('KillEntity delegates lethal life mutation','ServerLegacyLifeAdapter().ConsumeLethal' in block),
 ('KillEntity preserves entity serial identity','EntityId(static_cast<std::uint32_t>(slot.entity->dwObjectSerial))' in block),
 ('KillEntity consumes current life for exact lethal result','slot.entity->smCharInfo.Life[0]);' in block),
 ('KillEntity no direct life write','slot.entity->smCharInfo.Life[0] = 0' not in block),
 ('KillEntity no direct death motion','SetMotionFromCode(CHRMOTION_STATE_DEAD)' not in block),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: sys.exit(1)
print('companion kill life authority v1: PASS')

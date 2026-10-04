#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
checks=[
 ('companion invalid-link kill uses life adapter','HoSkillCode && !companionOwnedSkillPet' in src and src.count('std::max(1, static_cast<int>(smCharInfo.Life[0]))') >= 3),
 ('mystic timeout kill uses life adapter','if(Counter > 16 * 60 * 2)' in src and 'ServerLegacyLifeAdapter().ConsumeLethal' in src),
 ('mystic target damage uses lethal command','const int mysticDamage' in src and 'lpTargetChar->dwObjectSerial)), mysticDamage' in src),
 ('legacy mystic direct subtraction retired','lpTargetChar->smCharInfo.Life[0] -= GetRandomPos(smMonsterInfo.SkillDamage[0], smMonsterInfo.SkillDamage[1]) * 2;' not in src),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'character main simple lethal v2: PASS ({len(checks)}/{len(checks)})')

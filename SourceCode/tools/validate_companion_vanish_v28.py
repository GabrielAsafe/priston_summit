#!/usr/bin/env python3
from pathlib import Path
import re, sys
root=Path(__file__).resolve().parents[1]
exe=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
hdr=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
syscpp=(root/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
sysh=(root/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='ignore')
char=(root/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
cat=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
legacy=(root/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
scan=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='ignore')
err=[]
checks={
 'catalog ready': r'\{\s*SKILL_VANISH\s*,\s*COMPANION_SKILL_ROLE_BUFF_SELF\s*,\s*true\s*,\s*"Vanish"',
 'buff support': r'case\s+SKILL_VANISH\s*:',
 'visual play code': r'(case\s+SKILL_VANISH\s*:\s*return\s+SKILL_PLAY_VANISH|DefaultLegacySkillAdapter\(\)\.PlayCode)',
 'duration table': r'case\s+SKILL_VANISH\s*:\s*return\s+Vanish_Time\[p\]',
 'runtime point': r'int\s+vanishPoint\s*;',
 'single bonus': r'Vanish_Damage\[vanishPoint\s*-\s*1\]',
 'aoe cancel policy': r'const\s+bool\s+rangeAttack\s*=\s*CompanionSkillAreaRadius',
 'target visibility helper': r'CanMonsterSeeCompanionWithVanish',
 'system target bridge': r'CanMonsterTargetCompanion',
 'attack loop gate': r'CanMonsterTargetCompanion\(this,\s*lpTargetChar\)',
 'legacy single oracle': r'lpTransAttackData->Power\s*\+=\s*\(lpTransAttackData->Power\*Vanish_Damage',
 'legacy range cancel oracle': r'rsSetSkillRangeAttack[\s\S]{0,5000}?dwSkill_Vanish_Time\s*=\s*0',
 'legacy sight oracle': r'Real_Sight\*Vanish_Range',
}
texts={
 'catalog ready':cat,'buff support':exe,'visual play code':exe,'duration table':exe,
 'runtime point':hdr,'single bonus':exe,'aoe cancel policy':exe,
 'target visibility helper':exe+hdr,'system target bridge':syscpp+sysh,
 'attack loop gate':char,'legacy single oracle':legacy,'legacy range cancel oracle':legacy,
 'legacy sight oracle':scan,
}
for name,pat in checks.items():
    if not re.search(pat,texts[name],re.S): err.append(name)
ready=len(re.findall(r',\s*true\s*,\s*"',cat))
blocked=len(re.findall(r',\s*false\s*,\s*"',cat))
if ready < 112: err.append(f'expected >=112 READY, got {ready}')
if blocked != 16: err.append(f'expected 16 passive CATALOG_ONLY entries, got {blocked}')
if err:
    print('Companion Vanish V28: FAIL')
    for e in err: print(' -',e)
    sys.exit(1)
print(f'Companion Vanish V28: PASS ({ready} READY / {blocked} passive CATALOG_ONLY)')

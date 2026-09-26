#!/usr/bin/env python3
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
h=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
sys=(root/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
cat=(root/'Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp').read_text(errors='ignore')
ui=(root/'SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp').read_text(errors='ignore')
need=['TryExecuteCompanionSupportLoadout','SKILL_AGONY','SKILL_GRAND_HEALING','COMPANION_SKILL_NOT_NEEDED','POTION_STATE','SKILL_SPIRIT_ELEMENTAL','Spirit_Elemental_RegenMana','magicSkillDamagePercent','SKILL_EXTREME_SHIELD','E_Shield_BlockRate','SKILL_ZENITH','Zenith_Element','SKILL_IMPULSION','SKILL_DIVINE_LIGHTNING','SKILL_M_METEO','CompanionSkillMaxTargets']
blob='\n'.join([cpp,h,sys,cat,ui])
miss=[x for x in need if x not in blob and x not in ('SKILL_AGONY','SKILL_GRAND_HEALING','SKILL_SPIRIT_ELEMENTAL','SKILL_EXTREME_SHIELD','SKILL_ZENITH','SKILL_IMPULSION','SKILL_DIVINE_LIGHTNING','SKILL_M_METEO')]
if miss:
 print('Companion Skill Support V5: FAIL'); [print(' - missing',x) for x in miss]; raise SystemExit(1)
for name in ['Agony','Grand Healing','Spirit Elemental','Extreme Shield','Zenith','Impulsion','Divine Lightning','Meteor']:
 if not re.search(r'SkillBehavior::[A-Za-z]+,\s*true,\s*"'+re.escape(name)+r'"',cat):
  raise SystemExit('Companion Skill Support V5: FAIL\n - canonical skill not executable: '+name)
ready_count=len(re.findall(r'MakeSkill\([^\n]+SkillBehavior::[A-Za-z]+,\s*true,',cat))
if ready_count!=142: raise SystemExit(f'Companion Skill Support V5: FAIL\n - expected 142 executable skills, got {ready_count}')
if 'LogCompanionPotionState' not in sys or 'POTION_STATE' not in cpp or 'ITEM_USE' not in sys: raise SystemExit('Companion Skill Support V5: FAIL\n - potion telemetry incomplete')
if 'SUPORTE / RECAST' not in ui: raise SystemExit('Companion Skill Support V5: FAIL\n - UI support label missing')
print('Companion Skill Support V5: PASS')

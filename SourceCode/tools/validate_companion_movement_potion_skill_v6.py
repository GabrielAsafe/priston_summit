#!/usr/bin/env python3
from pathlib import Path
import re,sys
root=Path(__file__).resolve().parents[1]
character=(root/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
system=(root/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
exec_cpp=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
record=(root/'Shared/persistence/character_record/record_legacy_server.inl').read_text(errors='ignore')
cat=(root/'Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp').read_text(errors='ignore')
err=[]
for tok in ('int followY = companionOwnedSkillPet ? lpLinkChar->pY : lpMasterPlayInfo->Position.y','safeTeleportY = followY + (2 * fONE)','safeTeleportY = y + (2 * fONE)','PosiAround.y = pY','AutoMoveStep = 0','MotionInfo->State != CHRMOTION_STATE_RUN','MotionInfo->State != CHRMOTION_STATE_DAMAGE','MotionInfo->State != CHRMOTION_STATE_ATTACK','MotionInfo->State != CHRMOTION_STATE_SKILL'):
    if tok not in character: err.append('movement/teleport marker missing: '+tok)
if system.count('MotionInfo->State != CHRMOTION_STATE_RUN') < 2: err.append('follow/chase/retreat RUN forcing incomplete')
for tok in ('SetMotionFromCode(CHRMOTION_STATE_RUN)','MotionInfo->State != CHRMOTION_STATE_RUN'):
    if tok not in system: err.append('companion RUN marker missing: '+tok)
for tok in ('potionMonitorValid','sPotionUpdate[0] == recordData.GameSaveInfo.sPotionUpdate[1]','sPotionCount[PotionType][candidate] > 0','lpStatus) *lpStatus = 2','no physical stack and no synchronized monitor count','lpStatus) *lpStatus = -13'):
    if tok not in record: err.append('PotionMonitor fallback missing: '+tok)
for tok in ('case SKILL_BOMB_SHOT:','BombShot_Damage[p]','BombShot_DemonDamage[p]','BombShot_Area[p]','case SKILL_FLAME_WAVE:','Flame_Wave_FireDamage_Percent[p]','CompanionSkillBoxHalfWidth','CompanionSkillBoxForwardLength','IsInsideCompanionForwardBox'):
    if tok not in exec_cpp: err.append('V6 skill adapter missing: '+tok)
for name in ('Bomb Shot','Flame Wave','Perforation','Piercing'):
    if not re.search(r'SkillBehavior::[A-Za-z]+,\s*true,\s*"'+re.escape(name)+r'"',cat): err.append('canonical skill not executable: '+name)
ready=len(re.findall(r'MakeSkill\([^\n]+SkillBehavior::[A-Za-z]+,\s*true,',cat))
if ready!=142: err.append(f'expected 142 canonical executable skills, got {ready}')
if 'status > 0 ? status : 1' not in exec_cpp: err.append('potion source status is not preserved in ITEM_CHECK telemetry')
if err:
    print('Companion movement/potion/skill V6: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion movement/potion/skill V6: PASS (safe teleport + RUN + PotionMonitor + {ready} executable)')

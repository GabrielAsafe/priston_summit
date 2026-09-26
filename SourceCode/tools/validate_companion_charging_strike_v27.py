#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
cpp=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
h=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
syscpp=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
client=(ROOT/'Shared/gameplay/character/character_legacy_client.inl').read_text(errors='ignore')
legacy=(ROOT/'Shared/gameplay/skills/SkillSub_legacy_server.inl').read_text(errors='ignore')
dmg=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
cat=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
err=[]
line=next((x for x in cat.splitlines() if 'SKILL_CHARGING_STRIKE' in x),'')
if ', true,' not in line: err.append('Charging Strike not READY')
for tok in ['chargedStrikeTargetSerial','chargedStrikeExecuteAt','chargedStrikePoint','UpdateCompanionChargedStrike']:
 if tok not in h: err.append('runtime/API missing '+tok)
for tok in ['Charging_Strike_CharingDamagePercent[p]','MotionLoopSpeed = 60 + (Charging_Strike_Time[point - 1] * 2)',
            'MotionInfo->EventFrame[2]','ticks * 1000U + 69U','UpdateCompanionChargedStrike(smCHAR *caster']:
 if tok not in cpp: err.append('server missing '+tok)
if 'UpdateCompanionChargedStrike(character, owner, managedSlot->skillRuntime' not in syscpp: err.append('system pump missing')
for tok in ['SKILL_PLAY_CHARGING_STRIKE','Charging_Strike_Time[point - 1] * 2','character->MotionLoop = 1']:
 if tok not in client: err.append('client visual missing '+tok)
for tok in ['SkillChargingFlag = 1','Charging_Strike_Time[lpSkill->Point - 1] * 2']:
 if tok not in legacy: err.append('legacy animation oracle missing '+tok)
for tok in ['case SKILL_PLAY_CHARGING_STRIKE:','Charging_Strike_CharingDamagePercent[Point]']:
 if tok not in dmg: err.append('legacy damage oracle missing '+tok)
ready=len(re.findall(r'\{\s*SKILL_[A-Z0-9_]+,\s*COMPANION_SKILL_ROLE_[A-Z_]+,\s*true,',cat))
if ready<111: err.append(f'expected >=111 READY, got {ready}')
if err:
 print('Companion Charging Strike V27: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Charging Strike V27: PASS ({ready} READY; full-charge delayed hit)')

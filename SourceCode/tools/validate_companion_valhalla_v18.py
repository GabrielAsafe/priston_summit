#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
ORACLE=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='ignore')
POL=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
err=[]
for code in ('SKILL_TRIUMPH_OF_VALHALLA','SKILL_HALL_OF_VALHALLA'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
for tok in ('triumphValhallaExpiresAt','triumphValhallaMaxDamageBonus','hallValhallaExpiresAt','hallValhallaEvasionPercent','hallValhallaMaxDamageBonus'):
    if tok not in H or tok not in CPP: err.append('Valhalla runtime missing '+tok)
for tok in ('T_Of_Valhalla_Damage[p]','caster->smCharInfo.Level / 4','bonus / 2','runtime.hallValhallaExpiresAt = 0','candidateRuntime->hallValhallaExpiresAt'):
    if tok not in CPP: err.append('Triumph missing '+tok)
for tok in ('dwSkill_TriumphOfValhalla_Param = T_Of_Valhalla_Damage[point] + lv','dwSkill_TriumphOfValhalla_Param = (T_Of_Valhalla_Damage[point] + lv) / 2'):
    if tok not in ORACLE: err.append('Triumph oracle missing '+tok)
for tok in ('LearnedPointForCode(caster->smCharInfo.JOB_CODE, runtime.skillPoints, SKILL_TRIUMPH_OF_VALHALLA)','Hall_Of_Valhalla_AddEvation[p]','Hall_Of_Valhalla_AddEvation[p] / 2','500 * 500','ActiveHallValhallaEvasion(runtime, now)'):
    if tok not in CPP: err.append('Hall missing '+tok)
for tok in ('wSkill_HallOfValhalla_Param[0] = Hall_Of_Valhalla_AddEvation[point]','wSkill_HallOfValhalla_Param[1] = T_Of_Valhalla_Damage[lpTransCommand->SParam - 1] + lv','Hall_Of_Valhalla_AddEvation[point] / 2'):
    if tok not in ORACLE: err.append('Hall oracle missing '+tok)
# Basic attack semantics must reroll min..max+bonus, not add a flat bonus after a completed roll.
for tok in ('RandomBetween(caster->smCharInfo.Attack_Damage[0], caster->smCharInfo.Attack_Damage[1] + valhallaBonus)','ActiveValhallaMaxDamageBonus(runtime, now)'):
    if tok not in CPP: err.append('Valhalla basic reroll missing '+tok)
for tok in ('GetRandomPos(lpTransAttackData2->Power[0], lpTransAttackData2->Power[1] + (short)lpPlayInfo->dwSkill_TriumphOfValhalla_Param)','GetRandomPos(lpTransAttackData2->Power[0], lpTransAttackData2->Power[1] + (short)lpPlayInfo->wSkill_HallOfValhalla_Param[1])'):
    if tok not in POL: err.append('Valhalla reroll oracle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 97: err.append(f'expected at least 97 READY after V18, got {ready}')
if err:
    print('Companion Valhalla V18: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Valhalla V18: PASS ({ready} READY; Triumph + Hall)')

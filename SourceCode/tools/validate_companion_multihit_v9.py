#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
SYS=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
LEG=(ROOT/'Shared/gameplay/skills/SkillSub_legacy_server.inl').read_text(errors='ignore')
POL=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
err=[]
for tok in ('multiHitSkillCode','multiHitTargetSerial','multiHitRemaining','multiHitNextTime','UpdateCompanionMultiHit'):
    if tok not in H or tok not in CPP: err.append('scheduler contract missing '+tok)
for tok in ('case SKILL_RAVING:','Raving_Damage[p]','Ravind_Speed[point - 1]','ApplyRavingLifeCost','Raving_UseLife[p]'):
    if tok not in CPP: err.append('Raving V9 missing '+tok)
for tok in ('case SKILL_AVALANCHE:','Avalanche_Damage[p]','Avalanche_ArrowNum[point - 1]'):
    if tok not in CPP: err.append('Avalanche V9 missing '+tok)
if 'damage = perArrow *' in CPP: err.append('Avalanche still aggregates all arrows into one burst')
for tok in ('case SKILL_MULTISPARK:','M_Spark_Num[point - 1]','M_Spark_Damage_Percent[p] * sparkCount','visualParam & 0xF'):
    if tok not in CPP: err.append('Multi Spark V9 missing '+tok)
for code in ('SKILL_RAVING','SKILL_MULTISPARK','SKILL_AVALANCHE'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append('catalog not READY '+code)
if 'UpdateCompanionMultiHit(character, owner, managedSlot->skillRuntime, dwPlayServTime)' not in SYS:
    err.append('CompanionSystem does not advance scheduled hits')
# Legacy oracle markers
for tok in ('lpCurPlayer->MotionLoop = Ravind_Speed[lpSkill->Point - 1]','PlaySkillSubLife(((int)(Raving_UseLife[point - 1] * 10)*lpChar->smCharInfo.Life[1]) / 1000)','lpCurPlayer->MotionLoop = Avalanche_ArrowNum[lpSkill->Point - 1]','cnt = GetRandomPos((cnt / 2) + 1, cnt)','lpCurPlayer->AttackSkil |= (cnt << 12)'):
    if tok not in LEG: err.append('legacy multi-hit oracle missing '+tok)
for tok in ('case SKILL_PLAY_RAVING:','Raving_Damage[Point]','case SKILL_PLAY_AVALANCHE:','Avalanche_Damage[Point]','case SKILL_PLAY_MULTI_SPARK:','M_Spark_Damage_Percent[Point] * Param'):
    if tok not in POL: err.append('legacy damage oracle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 69: err.append(f'expected at least 69 READY after V9, got {ready}')
if err:
    print('Companion Multi-hit V9: FAIL')
    for e in err: print(' -',e)
    sys.exit(1)
print(f'Companion Multi-hit V9: PASS ({ready} READY; Raving/Avalanche scheduler + Multi Spark param)')

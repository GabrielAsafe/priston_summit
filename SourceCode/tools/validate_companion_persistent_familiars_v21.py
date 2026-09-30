#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
SYS=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
DMG=(ROOT/'Shared/legacy_oracle/server/systems/combat/server_legacy/Svr_Damge.cpp.inl').read_text(errors='ignore')
SUB=(ROOT/'Shared/legacy_oracle/server/systems/combat/skills/server_support/SkillSub.cpp.inl').read_text(errors='ignore')
ON=(ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl').read_text(errors='ignore')
CLIENT=(ROOT/'Shared/legacy_oracle/client/systems/rendering/effects/legacy_new/core/HoEffectManager.cpp.inl').read_text(errors='ignore')
ASSA=(ROOT/'Shared/legacy_oracle/client/systems/rendering/effects/assa/AssaParticle.cpp.inl').read_text(errors='ignore')
CHAR=(ROOT/'Shared/gameplay/character/character_legacy_client.inl').read_text(errors='ignore')
STATUS=(ROOT/'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp').read_text(errors='ignore')
err=[]
for code in ('SKILL_MAGNETIC_SPHERE','SKILL_DANCING_SWORD','SKILL_SUMMON_MUSPELL'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
for tok in ('persistentEffectSkillCode','persistentEffectExpiresAt','persistentEffectNextAttackAt','persistentEffectPoint','persistentEffectElement'):
    if tok not in H or tok not in CPP: err.append('persistent runtime missing '+tok)
for tok in ('FindNearestPersistentEffectTarget','300 * 300','UpdateCompanionPersistentEffect'):
    if tok not in CPP: err.append('persistent target/update missing '+tok)
if 'UpdateCompanionPersistentEffect(character, owner, managedSlot->skillRuntime, dwPlayServTime)' not in SYS:
    err.append('persistent effect not pumped from CompanionSystem')
# Magnetic Sphere: autonomous lightning hit, mechanic +30%, exact delay and duration.
for tok in ('Magnetic_Sphere_AddDamage[p][0]','Magnetic_Sphere_AttackDelay[p]','Magnetic_Sphere_Time[p]',
            'target->smCharInfo.Brood == smCHAR_MONSTER_MECHANIC','sITEMINFO_LIGHTING'):
    if tok not in CPP: err.append('Magnetic Sphere semantics missing '+tok)
for tok in ('SKILL_PLAY_MAGNETIC_SPHERE','Magnetic_Sphere_AddDamage[lpPlayInfo->dwSkill_MagneticSphere_Param][0]'):
    if tok not in DMG: err.append('Magnetic Sphere damage oracle missing '+tok)
if 'SkillMetalionSphere(this, (float)Magnetic_Sphere_Time[point - 1], (float)Magnetic_Sphere_AttackDelay[point - 1])' not in CHAR:
    err.append('Magnetic Sphere client cadence oracle missing')
# Dancing Sword: random Ice/Fire chosen at cast, server 900ms delay validation, ice slow.
for tok in ('runtime.persistentEffectElement = rand() % 2','Dancing_Sword_Damage[p][0]','Dancing_Sword_AttackDelay[p]) * 900U',
            'runtime.persistentEffectElement == 0 ? sITEMINFO_ICE : sITEMINFO_FIRE','ServerLegacyStatusEffectAdapter().ApplyElementalSlow16Hz(', '5, 230 - 10 * runtime.persistentEffectPoint, resistance'):
    if tok not in CPP: err.append('Dancing Sword semantics missing '+tok)
for tok in ('int durationTicks = durationSeconds * 16','resolved->PlayDistortion = 0'):
    if tok not in STATUS: err.append('Dancing Sword slow adapter missing '+tok)
for tok in ('cnt = rand() % 2','SendProcessSKillToServer(SKILL_PLAY_DANCING_SWORD, lpSkill->Point, cnt, 0)'):
    if tok not in SUB: err.append('Dancing Sword cast oracle missing '+tok)
for tok in ('Dancing_Sword_AttackDelay[pow1] * 900','Dancing_Sword_Damage[pow1][0]'):
    if tok not in DMG: err.append('Dancing Sword damage oracle missing '+tok)
if 'SendDancingSwordDamage(AttackMonster)' not in ASSA or 'AttackMonster = FindNearMonster(pChar)' not in ASSA:
    err.append('Dancing Sword autonomous target oracle missing')
# Muspell: timed attacks + block + undead reduction.
for tok in ('Summon_Muspell_Damage[p][0]','Summon_Muspell_Attack_Delay[p]','Summon_Muspell_Time[p]',
            'Summon_Muspell_BlockPercent[p]','Summon_Muspell_UndeadAbsorbPercent[p]'):
    if tok not in CPP: err.append('Muspell semantics missing '+tok)
for tok in ('Summon_Muspell_Damage[lpPlayInfo->dwSkill_SummonMuspell_Param][0]','SKILL_PLAY_SUMMON_MUSPELL'):
    if tok not in DMG: err.append('Muspell damage oracle missing '+tok)
for tok in ('dwSkill_SummonMuspell_Time = dwPlayServTime + Summon_Muspell_Time[point] * 1000','dwSkill_SummonMuspell_Param = point'):
    if tok not in ON: err.append('Muspell state oracle missing '+tok)
if 'm_AttackMonster = FindNearMonster(m_pAttachChar)' not in CLIENT or 'SendMuspellDamage(m_AttackMonster)' not in CLIENT:
    err.append('Muspell autonomous target oracle missing')
for tok in ('Summon_Muspell_BlockPercent[lpPlayInfo->dwSkill_SummonMuspell_Param]','Summon_Muspell_UndeadAbsorbPercent[lpPlayInfo->dwSkill_SummonMuspell_Param]'):
    if tok not in CHAR: err.append('Muspell defensive oracle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 102: err.append(f'expected at least 102 READY after V21, got {ready}')
if err:
    print('Companion persistent familiars V21: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion persistent familiars V21: PASS ({ready} READY; Magnetic/Dancing/Muspell)')

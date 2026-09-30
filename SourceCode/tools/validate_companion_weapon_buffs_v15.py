#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
SYS=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
POL=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
ORACLE=(ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl').read_text(errors='ignore')
STATUS=(ROOT/'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp').read_text(errors='ignore')
err=[]
for code in ('SKILL_GOD_BLESS','SKILL_FROST_JAVELIN'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
    if 'case '+code+':' not in CPP: err.append(code+' missing buff dispatch')
# God Bless is flat, post-formula, and affects normal + skill damage.
for tok in ('modifier.outgoingFlatDamage = God_Bless_AddDamage[p]','damage += ActiveOutgoingFlatDamage(*runtime)','ActiveOutgoingFlatDamage(runtime)'):
    if tok not in CPP: err.append('God Bless missing '+tok)
for tok in ('lpTransAttackData->Power += lpPlayInfo->dwSkill_GodsBless_Param','dwSkill_GodsBless_Param = God_Bless_AddDamage[point]'):
    src=POL if 'lpTransAttackData' in tok else ORACLE
    if tok not in src: err.append('God Bless oracle missing '+tok)
# Frost Javelin is basic-only and weapon-gated at hit time.
for tok in ('modifier.frostJavelinPoint = point','(rightItemCode & sinITEM_MASK2) == sinWT1','Frost_Javelin_IceAddDamage[p][0]','target->smCharInfo.Resistance[sITEMINFO_ICE]','ServerLegacyStatusEffectAdapter().ApplyElementalSlow16Hz(','Frost_Javelin_IceTime[p]','Frost_Javelin_SpeedSubPercent[p]'):
    if tok not in CPP: err.append('Frost Javelin missing '+tok)
for tok in ('int durationTicks = durationSeconds * 16','resolved->PlayDistortion = 0'):
    if tok not in STATUS: err.append('Frost Javelin status adapter missing '+tok)
for tok in ('lpPlayInfo->dwSkill_FrostJavelin_Time && !dwAttSkillCode','dwWeaponCode == sinWT1','Frost_Javelin_IceAddDamage[lpPlayInfo->dwSkill_FrostJavelin_Param][0]','PlaySlowCount = 16 * Frost_Javelin_IceTime','PlaySlowSpeed = 240 - (240 * Frost_Javelin_SpeedSubPercent'):
    if tok not in POL: err.append('Frost Javelin oracle missing '+tok)
# Exactly two direct attack hooks and a managed-companion bridge.
if CHAR.count('gameplay::companions::ApplyOutgoingBasicDamage(this, lpTargetChar, &cnt);') != 2:
    err.append('expected exactly two outgoing basic-damage hooks')
if 'ApplyCompanionOutgoingBasicDamageModifiers(attacker, target, rightItemCode,' not in SYS:
    err.append('CompanionSystem outgoing bridge missing')
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 90: err.append(f'expected at least 90 READY after V15, got {ready}')
if err:
    print('Companion weapon buffs V15: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion weapon buffs V15: PASS ({ready} READY; God Bless + Frost Javelin)')

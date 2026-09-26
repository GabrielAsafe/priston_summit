#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
SYS=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
ORACLE=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='ignore')
POL=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
err=[]
for code in ('SKILL_SPARK_SHIELD','SKILL_HOLY_REFLECTION','SKILL_DIVINE_INHALATION'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
    if 'case '+code+':' not in CPP: err.append(code+' missing buff dispatch')
for tok in ('sparkShieldDamage','holyReflectionPercent','divineInhalationPoint'):
    if tok not in H or tok not in CPP: err.append('reactive runtime field missing '+tok)
# Spark Shield: depends on learned Spark, triggers on block, Mechanic +50%, Lightning resistance.
for tok in ('Spark_Shield_Damage[p] * Spark_DamagePercent[sparkPoint - 1]','attacker->smCharInfo.Brood == smCHAR_MONSTER_MECHANIC','sITEMINFO_LIGHTING','Spark Shield is triggered by the block event'):
    if tok not in CPP: err.append('Spark Shield missing '+tok)
for tok in ('dwSkill_SparkShield_Param = (Spark_Shield_Damage[point] * Spark_DamagePercent','case SKILL_PLAY_SPARK_SHIELD:'):
    if tok not in ORACLE and tok not in POL: err.append('Spark Shield oracle missing '+tok)
# Holy Reflection: any Undead attack, attacker damage roll, Bionic resistance, then normal Absorption via ApplySkillDamage.
for tok in ('holyReflectionPercent = Holy_Reflection_Return_Damage[p]','attacker->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD','attacker->smCharInfo.Resistance[sITEMINFO_BIONIC]','ApplySkillDamage(target, owner, attacker, reflected)'):
    if tok not in CPP: err.append('Holy Reflection missing '+tok)
for tok in ('pow = GetRandomPos(lpChar->smCharInfo.Attack_Damage[0], lpChar->smCharInfo.Attack_Damage[1])','pow = (pow*lpPlayInfo->dwSkill_HolyReflection_Param) / 100'):
    if tok not in ORACLE: err.append('Holy Reflection oracle missing '+tok)
# Divine Inhalation: extra block only vs Undead and low-byte heal encoding on successful block.
for tok in ('blockChance += D_Inhalation_Block[point - 1]','D_Inhalation_Life[divinePoint - 1] * resistance','heal &= 0xFF','target->smCharInfo.Life[0] = (short)std::min'):
    if tok not in CPP: err.append('Divine Inhalation missing '+tok)
for tok in ('D_Inhalation_Block[lpPlayInfo->dwSkill_DivineInhalation_Param]','D_Inhalation_Life[lpPlayInfo->dwSkill_DivineInhalation_Param]'):
    if tok not in CHAR: err.append('Divine Inhalation oracle missing '+tok)
if 'ApplyCompanionIncomingDamageModifiers(target, attacker, state.owner,' not in SYS:
    err.append('owner-aware incoming bridge missing')
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 93: err.append(f'expected at least 93 READY after V16, got {ready}')
if err:
    print('Companion reactive defense V16: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion reactive defense V16: PASS ({ready} READY; Spark Shield + Holy Reflection + Divine Inhalation)')

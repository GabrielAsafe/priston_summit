#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
SYS=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
INV=(ROOT/'SrcServer/src/systems/inventory/legacy/sinInvenTory.cpp').read_text(errors='ignore')
ORACLE=(ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl').read_text(errors='ignore')
err=[]
for code in ('SKILL_METAL_ARMOR','SKILL_HOLY_BODY','SKILL_ENERGY_SHIELD','SKILL_VAGUE','SKILL_GODLY_SHIELD'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
    if 'case '+code+':' not in CPP: err.append(code+' missing buff dispatch')
# Hook must happen before existing Absorption in both legacy monster attack branches.
if CHAR.count('gameplay::companions::ApplyIncomingDamage(lpTargetChar, this, &cnt);') != 2:
    err.append('expected exactly two monster->companion incoming damage hooks')
if 'ApplyCompanionIncomingDamageModifiers(target, attacker,' not in SYS:
    err.append('CompanionSystem incoming-damage bridge missing')
for tok in ('incomingFlatReduction','incomingDamageReductionPercent','undeadDamageReductionPercent','evasionPercent','energyShieldPercent'):
    if tok not in H or tok not in CPP: err.append('runtime modifier missing '+tok)
# Metal Armor: % Defence on sheet + exact Physical Absorption point average for server flat reduction.
for tok in ('Metal_Armor_Defense[p]','LearnedPointForCode(caster->smCharInfo.JOB_CODE,','SKILL_PHYSICAL_ABSORB','(P_Absorb[ap][0] + P_Absorb[ap][1]) / 2'):
    if tok not in CPP: err.append('Metal Armor missing '+tok)
if 'sinDefense += (Metal_Armor_Defense[ContinueSkill[i].Point - 1] * (TempDefense)) / 100' not in INV:
    err.append('Metal Armor defence oracle missing')
if 'dwSkill_MetalArmor_Param = (P_Absorb[lpTransCommand->SParam - 1][0] + P_Absorb[lpTransCommand->SParam - 1][1]) / 2' not in ORACLE:
    err.append('Metal Armor server reduction oracle missing')
# Holy Body: only Undead.
for tok in ('modifier.undeadDamageReductionPercent = ClampPercent(HolyBody_Absorb[p])','attacker->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD'):
    if tok not in CPP: err.append('Holy Body missing '+tok)
if 'TransAttackData.Power -= (TransAttackData.Power*lpPlayInfo->dwSkill_HolyBody_Param) / 100' not in ORACLE:
    err.append('Holy Body oracle missing')
# Energy Shield: full percentage reduction and matching MP loss.
for tok in ('modifier.energyShieldPercent = ClampPercent(Energy_Shield_DecDamage[p])','const int absorbed = (value * ClampPercent(modifier.energyShieldPercent)) / 100','DrainCompanionSkillResources(target, absorbed, 0)'):
    if tok not in CPP: err.append('Energy Shield missing '+tok)
if 'temp = (TransAttackData.Power*lpPlayInfo->dwSkill_EnergyShield_Param) / 100' not in ORACLE:
    err.append('Energy Shield oracle missing')
# Vague and Godly Shield.
for tok in ('modifier.evasionPercent = ClampPercent(Vague_EvasionPercent[p])','(rand() % 100) < ClampPercent(modifier.evasionPercent)','modifier.incomingDamageReductionPercent = ClampPercent(Godly_Shield_AbsorbPercent[p])'):
    if tok not in CPP: err.append('Vague/Godly missing '+tok)
if 'if((rand() % 100) < Vague_EvasionPercent[lpPlayInfo->dwSkill_Vague_Param])' not in CHAR:
    err.append('Vague oracle missing')
if 'Godly_Shield_AbsorbPercent[lpPlayInfo->dwSkill_GodlyShied_Param]' not in CHAR:
    err.append('Godly Shield oracle missing')
# V13 READY Virtual Life is corrected to include legacy incoming reduction.
for tok in ('case SKILL_VIRTUAL_LIFE:','modifier.incomingDamageReductionPercent = ClampPercent(Virtual_Life_Percent[p])'):
    if tok not in CPP: err.append('Virtual Life incoming mitigation missing '+tok)
if 'dwSkill_VirtualLife_Param) / 100' not in CHAR:
    err.append('Virtual Life oracle missing')
# Extreme Shield must now have a real block roll in the monster->companion path.
for tok in ('int blockChance = (int)target->smCharInfo.Chance_Block','std::min(50, blockChance)','(rand() % 100) < blockChance'):
    if tok not in CPP: err.append('block runtime missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 88: err.append(f'expected at least 88 READY after V14, got {ready}')
if err:
    print('Companion defensive runtime V14: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion defensive runtime V14: PASS ({ready} READY; Metal/HolyBody/Energy/Vague/Godly + VirtualLife/block)')

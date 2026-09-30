#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
SYS=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
SYSH=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='ignore')
SYSH += (ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/CompanionLegacyCharacterHooks.h').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
ORACLE=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='ignore')
POL=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
err=[]
for code in ('SKILL_FORCE_OF_NATURE','SKILL_HOLY_VALOR'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
for tok in ('forceNatureExpiresAt','forceNatureDamage','forceNatureFalconDamage','holyValorExpiresAt','holyValorPercent'):
    if tok not in H or tok not in CPP: err.append('group runtime missing '+tok)
# Received group buffs must be outside four self-buff slots.
if 'CompanionSkillRuntime *const *companionRuntimes' not in H or 'partyRuntimes' not in SYS:
    err.append('party runtime plumbing missing')
# Force of Nature: full caster, half recipients within 500/height 80, owner legacy fields, flat pre-absorption + skill damage.
for tok in ('Force_Of_Nature_AddDamage[p]','Force_Of_Nature_AddDamage[p] / 2','Force_Of_Nature_FalconAddDamage[p]','500 * 500','abs(dy) < 80','owner->dwSkill_ForceOfNature_Time','ActiveForceNatureDamage(*runtime, dwPlayServTime)','ActiveForceNatureDamage(runtime, now)'):
    if tok not in CPP: err.append('Force of Nature missing '+tok)
for tok in ('wSkill_ForceOfNature_Param[0] = Force_Of_Nature_AddDamage[point]','wSkill_ForceOfNature_Param[0] = Force_Of_Nature_AddDamage[point] / 2','lpTransAttackData->Power += lpPlayInfo->wSkill_ForceOfNature_Param[0]'):
    if tok not in ORACLE and tok not in POL: err.append('Force of Nature oracle missing '+tok)
# Holy Valor: party state, Holy Body replacement, Undead-only post-absorption bonus.
for tok in ('owner->dwSkill_HolyBody_Time = 0','owner->dwSkill_HolyValor_Time = expiresAt','Holy_Valor_Damage[p]','target->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD','ApplyCompanionOutgoingBasicPostAbsorptionModifiers'):
    if tok not in CPP: err.append('Holy Valor missing '+tok)
if CHAR.count('gameplay::companions::ApplyOutgoingBasicPostAbsorption(this, lpTargetChar, &cnt);') != 2:
    err.append('expected two post-absorption basic hooks')
if 'ApplyOutgoingBasicPostAbsorption' not in SYS or 'ApplyOutgoingBasicPostAbsorption' not in SYSH:
    err.append('post-absorption system bridge missing')
for tok in ('dwSkill_HolyValor_Param = Holy_Valor_Damage[point]','if(lpChar->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD)','cnt += (cnt*lpPlayInfo->dwSkill_HolyValor_Param) / 100'):
    if tok not in ORACLE and tok not in POL: err.append('Holy Valor oracle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 95: err.append(f'expected at least 95 READY after V17, got {ready}')
if err:
    print('Companion group buffs V17: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion group buffs V17: PASS ({ready} READY; Force of Nature + Holy Valor)')

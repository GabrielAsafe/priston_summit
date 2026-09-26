#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
SYS=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
INV=(ROOT/'SrcServer/src/systems/inventory/legacy/sinInvenTory.cpp').read_text(errors='ignore')
err=[]
line=next((ln for ln in CAT.splitlines() if '{ SKILL_REGENERATION_FIELD,' in ln),'')
if ', true,' not in line: err.append('Regeneration Field not READY')
for tok in ('regenerationFieldExpiresAt','regenerationFieldPoint','regenerationFieldNextPulseAt','regenerationFieldPulseExpiresAt','regenerationFieldPulsePoint'):
    if tok not in H or tok not in CPP: err.append('regen field runtime missing '+tok)
for tok in ('Regeneration_Field_Time[p]','Regeneration_Field_Area[p]','now + 5000U','now + 4000U','abs(dy) >= 16'):
    if tok not in CPP: err.append('aura pulse semantics missing '+tok)
for tok in ('Regeneration_Field_LifeRegen[p]','Regeneration_Field_ManaRegen[p] / 2.0f','caster->smCharInfo.Life_Regen + fieldLifeRegen','caster->smCharInfo.Mana_Regen + fieldManaRegen'):
    if tok not in CPP: err.append('regen integration missing '+tok)
if 'UpdateCompanionRegenerationFieldAura(character, party, partyRuntimes, partyCount' not in SYS:
    err.append('field aura not pumped from CompanionSystem')
for tok in ('cSkill.Regeneration_Field(5, point, 1)','cnt = Regeneration_Field_Area[point - 1]'):
    if tok not in CHAR: err.append('legacy field pulse oracle missing '+tok)
for tok in ('Regeneration_Field_ManaRegen[ContinueSkill[i].Point - 1] / ContinueSkill[i].Flag','Regeneration_Field_LifeRegen[ContinueSkill[i].Point - 1]'):
    if tok not in INV: err.append('legacy regen flag oracle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 98: err.append(f'expected at least 98 READY after V19, got {ready}')
if err:
    print('Companion Regeneration Field V19: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Regeneration Field V19: PASS ({ready} READY; caster full + party 5s pulses)')

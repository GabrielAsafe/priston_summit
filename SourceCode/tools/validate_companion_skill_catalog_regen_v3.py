#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
P={
'catalog':ROOT/'Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp',
'h':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h',
'cpp':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp',
'system':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp',
'legacy_regen':ROOT/'SrcServer/src/core/server/legacy_sin_runtime/sinSubMain.cpp',
'legacy_inv':ROOT/'SrcServer/src/systems/inventory/legacy/sinInvenTory.cpp'}
err=[]
for p in P.values():
    if not p.exists(): err.append(f'missing {p.relative_to(ROOT)}')
if err:
    print('Companion canonical skill catalog + resource regen V3: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
T={k:p.read_text(errors='ignore') for k,p in P.items()}
pat=re.compile(r'MakeSkill\((0x[0-9A-Fa-f]+)u,\s*(\d+),\s*(\d+),\s*(\d+),\s*SkillBehavior::([A-Za-z]+),\s*(true|false),\s*"([^"]+)"')
entries=pat.findall(T['catalog'])
if len(entries)!=160: err.append(f'expected 160 canonical T1-T5 entries, got {len(entries)}')
if len({e[0] for e in entries})!=len(entries): err.append('duplicate skill IDs in canonical catalog')
t14=[e for e in entries if int(e[3])<=4]; t5=[e for e in entries if int(e[3])==5]
if len(t14)!=128 or len(t5)!=32: err.append(f'expected 128 T1-T4 + 32 T5, got {len(t14)} + {len(t5)}')
for owner in range(1,9):
    if len([e for e in entries if int(e[1])==owner])!=20: err.append(f'owner class {owner}: expected 20 skills')
ready=[e for e in entries if e[5]=='true']
if len(ready)!=142: err.append(f'expected 142 executable skills, got {len(ready)}')
for tok in ('GetCompanionSkillCatalogCount','GetCompanionSkillCatalogEntryAt','GetCompanionSkillCatalogEntry','CompanionSkillRoleName'):
    if tok not in T['h'] or tok not in T['cpp']: err.append('catalog compatibility API missing '+tok)
for tok in ('DefaultSkillRegistry','DefaultLegacySkillAdapter().ResolveSlot','DefaultLegacySkillAdapter().PlayCode'):
    if tok not in T['cpp'] and tok not in T['catalog']: err.append('Skills V2 route missing '+tok)
for tok in ('Automation_Time[p]','Berserker_Time[p]'):
    if tok not in T['cpp']: err.append('safe buff adapter missing '+tok)
for tok in ('modifier.attackMinDelta = Berserker_AddAttack[p];','modifier.attackMaxDelta = Berserker_AddAttack[p];','std::max(-(int)caster->smCharInfo.Absorption, Berserker_SubAbsorb[p])'):
    if tok not in T['cpp']: err.append('Berserker legacy semantics mismatch '+tok)
if 'sinTempDamage2[0] += Berserker_AddAttack' not in T['legacy_inv'] or 'sinTempAbsorb += Berserker_SubAbsorb' not in T['legacy_inv']: err.append('legacy Berserker oracle markers missing')
for tok in ('lastResourceRegenTime','lifeRegenAccumulator','manaRegenAccumulator','staminaRegenAccumulator'):
    if tok not in T['h'] or tok not in T['cpp']: err.append('regen runtime missing '+tok)
for tok in ('strength / 2.0f','health) / 180.0f','Life_Regen','spirit * 1.2f','health / 2.0f','/ 115.0f','Mana_Regen','StaminaFunction','Level / 7.0f','Stamina_Regen','staminaPerSecond *= 0.6f'):
    if tok not in T['cpp']: err.append('regen formula missing '+tok)
for tok in ('(fStrength / 2)','/ 180 + sinChar->Life_Regen','(fSpirit*1.2f)','/ 115 + sinChar->Mana_Regen'):
    if tok not in T['legacy_regen']: err.append('legacy regen oracle missing '+tok)
r=T['system'].find('UpdateCompanionResourceRegeneration('); q=T['system'].find('TryUseCompanionConsumable(')
if r<0 or q<0 or r>q: err.append('natural regen must execute before emergency potion use')
if 'RESOURCE_REGEN' not in T['system']: err.append('RESOURCE_REGEN telemetry missing')
if err:
    print('Companion canonical skill catalog + resource regen V3: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion canonical skill catalog + resource regen V3: PASS ({len(entries)} skills, {len(ready)} executable)')

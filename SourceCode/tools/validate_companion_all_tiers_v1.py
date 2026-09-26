#!/usr/bin/env python3
from pathlib import Path
import collections,re,sys
ROOT=Path(__file__).resolve().parents[1]
def rd(p): return (ROOT/p).read_text(encoding='utf-8',errors='ignore')
def req(c,m):
    if not c:
        print('FAIL:',m);sys.exit(1)
cat=rd('Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp')
svc=rd('Shared/gameplay/skills/v2/services/SkillService.cpp')
adapter=rd('Shared/gameplay/skills/v2/legacy/LegacySkillAdapter.h')
exe=rd('SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp')
hdr=rd('SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h')
syscpp=rd('SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp')
proto=rd('Shared/gameplay/companions/CompanionProtocol.h')
client=rd('Shared/gameplay/character/character_legacy_client.inl')
pat=re.compile(r'MakeSkill\((0x[0-9A-Fa-f]+)u,\s*(\d+),\s*(\d+),\s*(\d+),\s*SkillBehavior::([A-Za-z]+),\s*(true|false),\s*"([^"]*)",\s*(0x[0-9A-Fa-f]+)u,\s*(0x[0-9A-Fa-f]+)u,')
rows=pat.findall(cat)
req(len(rows)==160,f'expected 160 canonical T1-T5 rows, got {len(rows)}')
active=[r for r in rows if r[5]=='true']
passive=[r for r in rows if r[4]=='Passive']
req(len(active)==142,f'expected 142 executable skills, got {len(active)}')
req(len(passive)==18,f'expected 18 passives, got {len(passive)}')
req(all(r[5]=='false' for r in passive),'passive definitions must not be directly executable')
req(all(int(r[7],16)!=0 for r in active),'executable definition has zero legacy play code')
req(all(int(r[7],16)==0 for r in passive),'passive definition unexpectedly owns a play code')
by_tier=collections.Counter(int(r[3]) for r in rows)
req(by_tier==collections.Counter({1:32,2:32,3:32,4:32,5:32}),f'tier distribution mismatch: {dict(by_tier)}')
by_owner=collections.Counter(int(r[1]) for r in rows)
req(len(by_owner)==8 and set(by_owner.values())=={20},f'class distribution mismatch: {dict(by_owner)}')
for _,_,slot,tier,*_ in rows:
    req(int(tier)==((int(slot)-1)//4)+1,f'slot/tier mismatch slot={slot} tier={tier}')
req('COMPANION_MAX_SKILL_SLOTS = 64' in hdr,'logical slot capacity is not 64')
req('DefaultLegacySkillAdapter().ResolveSlot' in exe,'Companion does not resolve skills through shared SkillService/legacy adapter')
req('DefaultLegacySkillAdapter().PlayCode' in exe,'Companion does not source play code from shared SkillRegistry')
req('DefaultLegacySkillAdapter().IsDamage' in exe and 'DefaultLegacySkillAdapter().IsBuff' in exe,
    'Companion behavior classification is not delegated to shared SkillService')
req('case SKILL_IMPACT: return SKILL_PLAY_IMPACT;' not in exe,'legacy per-skill play switch still present in Companion executor')
req('CompanionSkillCatalog.inl' not in exe,'deprecated companion catalog is still included at runtime')
req('SkillBehavior::DamageSingle' in svc and 'IsBuffBehavior' in svc,'SkillService behavior routing missing')
req('service_.LegacyPlayCode' in adapter and 'service_.PresentationPlayCode' in adapter,'legacy adapter is not a thin service boundary')
# Keep difficult formulas/legacy semantics behind the boundary while catalog/classification is canonical.
for token in ['DamageForSkill','ApplyBuffModifier','VenomSpear_Time','AssassinEyeParam','PlayHolyMind','PlayDistortion',
              'Soul_Sucker_Absorb','Extinction_Amount','OpenMonsterFromSkill','Resurrection_Percent',
              'reactiveDamagePercent','GetReactiveDamage']:
    req(token in exe or token in syscpp or token in hdr,'legacy behavior bridge lost token '+token)
req('COMPANION_VISUAL_PROTOCOL_VERSION = 4' in proto,'visual protocol not v4')
req('logicalSkillCode' in proto and 'skillPlayCode' in proto,'logical/presentation identity not split')
req('SummitT5PrepareRemoteCompanionVisual' in client,'client companion visual bridge missing')
req('FindCatalogSkillForSlot' in exe and 'COMPANION_MAX_SKILL_SLOTS' in exe,'scalable slot routing missing')
print('PASS companion all-tiers execution contract (Skills V2)')
print('catalog=160 active=142 passive=18 tiers='+str(dict(sorted(by_tier.items())))+' classes=8')

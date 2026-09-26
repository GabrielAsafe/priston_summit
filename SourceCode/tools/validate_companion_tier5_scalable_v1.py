#!/usr/bin/env python3
from pathlib import Path
import re,sys,collections
ROOT=Path(__file__).resolve().parents[1]
def rd(p): return (ROOT/p).read_text(encoding='utf-8',errors='ignore')
def req(c,m):
    if not c:
        print('FAIL:',m);sys.exit(1)
cat=rd('Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp')
ext=rd('Shared/gameplay/skills/v2/legacy/LegacyExtendedSkillMetadata.inl')
exe=rd('SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp')
hdr=rd('SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h')
proto=rd('Shared/gameplay/companions/CompanionProtocol.h')
syscpp=rd('SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp')
client=rd('Shared/gameplay/character/character_legacy_client.inl')
skillclient=rd('Shared/gameplay/skills/SkillSub_legacy_client.inl')
sinclient=rd('Shared/gameplay/skills/sinSkill_legacy_client.inl')
rowpat=re.compile(r'MakeSkill\((0x[0-9A-Fa-f]+)u,\s*(\d+),\s*(\d+),\s*(\d+),\s*SkillBehavior::([A-Za-z]+),\s*(true|false),\s*"([^"]*)",\s*(0x[0-9A-Fa-f]+)u,\s*(0x[0-9A-Fa-f]+)u,\s*(-?\d+),\s*(true|false),\s*"([^"]*)",\s*"([^"]*)",\s*(true|false),\s*(-?\d+),\s*"([^"]*)",\s*"([^"]*)"\)')
rows=rowpat.findall(cat)
req(len(rows)==160,f'expected 160 canonical definitions, got {len(rows)}')
t5=[r for r in rows if int(r[3])==5]
req(len(t5)==32,f'expected 32 Tier5 definitions, got {len(t5)}')
active=[r for r in t5 if r[5]=='true']; passive=[r for r in t5 if r[4]=='Passive']
req(len(active)==30,f'expected 30 castable Tier5 skills, got {len(active)}')
req(len(passive)==2,f'expected 2 Tier5 passives, got {len(passive)}')
by_owner=collections.defaultdict(set)
for r in t5: by_owner[int(r[1])].add(int(r[2]))
req(len(by_owner)==8,'expected 8 Tier5 classes')
for owner,slots in by_owner.items(): req(slots=={17,18,19,20},f'class {owner} Tier5 slots={sorted(slots)}')
req('COMPANION_MAX_SKILL_SLOTS = 64' in hdr,'logical capacity must be 64')
ext_rows=re.findall(r'EXT_SKILL\((SKILL_T5_[A-Z0-9_]+),\s*(JOBCODE_[A-Z_]+),\s*(\d+),\s*5,',ext)
req(len(ext_rows)==32,f'expected 32 legacy requirement rows, got {len(ext_rows)}')
req('SKILL_PLAY_T5' not in ext,'legacy requirement bridge still duplicates play-code metadata')
req('DefaultLegacySkillAdapter().ResolveSlot' in exe,'Companion Tier5 slot routing bypasses shared SkillService')
req('DefaultLegacySkillAdapter().PlayCode' in exe,'Companion Tier5 play routing bypasses shared SkillRegistry')
req('CompanionExtendedSkillRegistry.inl' not in exe,'old companion-owned extended registry still included')
req('logicalSkillCode' in proto and 'skillPlayCode' in proto,'protocol identity split missing')
req('runtime->logicalSkillCode = entry.logicalSkillCode' in client,'client does not consume logical skill identity')
req('SummitT5PrepareRemoteCompanionVisual' in client,'remote Tier5 presentation bridge missing')
req('gSummitT5RemotePresentation' not in skillclient,'Tier5 has a second presentation catalog')
req('definition->presentation' in skillclient and 'definition->presentation.canonicalTimerPath' in sinclient,
    'client presentation/timer does not consume canonical SkillDefinition')
# Critical T5 network/presentation aliases. Network stays E1..E4; presentation may use proven legacy art/motion.
by_id={int(r[0],16):r for r in t5}
expected={
 0x01100003:(0xE3,0x0B), 0x02100003:(0xE3,0x2E),
 0x03100002:(0xE2,0x1E), 0x03100003:(0xE3,0x1F),
 0x04100001:(0xE1,0x4F), 0x05100001:(0xE1,0x5F), 0x05100003:(0xE3,0x5E),
 0x06100001:(0xE1,0x6D), 0x07100001:(0xE1,0x70), 0x07100004:(0xE4,0x7E),
 0x08100001:(0xE1,0x8B), 0x08100002:(0xE2,0x89),
}
for sid,(network,presentation) in expected.items():
    r=by_id.get(sid); req(r is not None,f'missing skill {sid:#x}')
    req(int(r[7],16)==network and int(r[8],16)==presentation,
        f'alias mismatch {sid:#x}: network={r[7]} presentation={r[8]}')
req('sinContinueSkillSet(pUseSkill);' in rd('Shared/gameplay/skills/Morayion_legacy_client.inl'),
    'Stone Skin does not register ContinueSkill state')
print('PASS companion Tier5 scalable contract (Skills V2)')
print('catalog=160 tier5=32 castable_t5=30 passive_t5=2 jobs=8 capacity=64')

#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
def rd(p): return (ROOT/p).read_text(errors='ignore')
protocol=rd('Shared/gameplay/companions/CompanionProtocol.h')
client=rd('Shared/gameplay/character/character_legacy_client.inl')
skillclient=rd('Shared/gameplay/skills/SkillSub_legacy_client.inl')
catalog=rd('Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp')
adapter=rd('Shared/gameplay/skills/v2/legacy/LegacySkillAdapter.h')
executor_h=rd('SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h')
executor=rd('SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp')
system=rd('SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp')
checks={
 'visual protocol v4':'COMPANION_VISUAL_PROTOCOL_VERSION = 4' in protocol,
 'logical skill replicated':'logicalSkillCode' in protocol and 'entry.logicalSkillCode = slot.skillRuntime.lastSkillCode' in system,
 'skill play code replicated':'skillPlayCode' in protocol and 'entry.skillPlayCode = slot.skillRuntime.lastSkillPlayCode' in system,
 'skill point replicated':'skillPoint' in protocol and 'entry.skillPoint = slot.skillRuntime.lastSkillPoint' in system,
 'skill target replicated':'skillTargetSerial' in protocol and 'entry.skillTargetSerial = slot.skillRuntime.lastSkillTargetSerial' in system,
 'skill motion replicated':'skillMotionState' in protocol and 'entry.skillMotionState = slot.skillRuntime.lastSkillMotionState' in system,
 'skill sequence replicated':'skillSequence' in protocol and 'entry.skillSequence = slot.skillRuntime.skillVisualSequence' in system,
 'runtime sequence increments':'++runtime.skillVisualSequence' in executor,
 'single canonical play source':'DefaultLegacySkillAdapter().PlayCode' in executor,
 'single canonical presentation source':'definition->legacy.presentationPlayCode' in skillclient,
 'no T5 presentation side table':'gSummitT5RemotePresentation' not in skillclient,
 'presentation metadata sourced from definition':'definition->presentation' in skillclient and 'presentation.actorParticle' in skillclient and 'presentation.targetParticle' in skillclient,
 'network/presentation separated':'SummitSkillNetworkPlayCode' in skillclient and 'SummitSkillPresentationPlayCode' in skillclient,
 'legacy adapter thin':'service_.LegacyPlayCode' in adapter and 'service_.PresentationPlayCode' in adapter,
 'impact mapping in canonical catalog':'MakeSkill(0x02010004u' in catalog and '0x24u, 0x24u' in catalog,
 'fire bolt mapping in canonical catalog':'MakeSkill(0x08010002u' in catalog and '0x82u, 0x82u' in catalog,
 'healing mapping in canonical catalog':'MakeSkill(0x07010001u' in catalog and '0x71u, 0x71u' in catalog,
 'client explicit skill event':'static void ApplySkillEvent(smCHAR *character, Runtime &runtime)' in client,
 'client BeginSkill presentation':'BeginSkill(character->AttackSkil, 0, target, 0, 0, 0)' in client,
 'EventSkill bridge':'character->EventAttack(FALSE)' in client and 'character->EventSkill()' in client,
 'runtime holds visual facts':all(x in executor_h for x in ['lastSkillPlayCode','lastSkillPoint','lastSkillTargetSerial','skillVisualSequence']),
}
failed=[n for n,ok in checks.items() if not ok]
if failed:
    for n in failed: print('FAIL:',n)
    sys.exit(1)
print('Companion skill visual replication V1 + Skills V2 presentation bridge: PASS')

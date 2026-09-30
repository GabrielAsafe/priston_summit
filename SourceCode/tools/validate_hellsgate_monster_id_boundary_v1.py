#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
h=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.h').read_text()
cpp=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
checks=[
 ('public kill hook uses EntityId','MonsterKilled(gameplay::EntityId monster)' in h and 'MonsterKilled(smCHAR' not in h),
 ('event compares ids','bossId == monster' in cpp and 'lpChar->dwObjectSerial' not in cpp[cpp.index('void cHELLSGATE::MonsterKilled'):cpp.index('void cHELLSGATE::StartEvent')]),
 ('invalid id guarded','monster.Valid()' in cpp),
 ('legacy caller translates entity id through defeat event','PublishMonsterDefeated(' in onserver and 'MonsterDefeatedEvent(' in onserver and 'gameplay::EntityId(static_cast<std::uint32_t>(lpChar->dwObjectSerial))' in onserver and 'cHg.MonsterKilled(' not in onserver),
 ('boss spawn id unchanged','spawned.Succeeded() ? spawned.entity' in cpp),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('hellsgate monster id boundary: PASS')

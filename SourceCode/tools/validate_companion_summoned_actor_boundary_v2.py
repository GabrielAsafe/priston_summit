#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacySummonedActorAdapter.cpp').read_text(errors='replace')
contract=(root/'Shared/gameplay/monsters/v2/SummonedActorCommands.h').read_text(errors='replace')
checks={
 'green contract has no smCHAR':'smCHAR' not in contract,
 'executor delegates summon configuration':'ServerLegacySummonedActorAdapter().Configure' in cpp,
 'executor has no summon direct life write':'summon->smCharInfo.Life[0] =' not in cpp and 'spawnedSkillPet->smCharInfo.Life[0] =' not in cpp,
 'executor has no summon direct attack write':'summon->smCharInfo.Attack_Damage[0] =' not in cpp and 'spawnedSkillPet->smCharInfo.Attack_Damage[0] =' not in cpp,
 'adapter owns life assignment':'actor->smCharInfo.Life[0]' in adapter and 'actor->smCharInfo.Life[1]' in adapter,
 'adapter owns lifetime assignment':'actor->dwUpdateCharInfoTime' in adapter,
 'adapter owns movement assignment':'actor->sMoveStepCount[0]' in adapter,
 'legacy clamp preserved':'std::min(32767, std::max(1, request.profile.life))' in adapter,
}
for n,ok in checks.items():print(('PASS' if ok else 'FAIL')+': '+n)
if not all(checks.values()):sys.exit(1)
print(f'companion summoned actor boundary v2: PASS ({len(checks)}/{len(checks)})')

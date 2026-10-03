#!/usr/bin/env python3
from pathlib import Path
import re,sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='replace')
# Isolate the automatic support-skill resurrection branch, which used to write Life[0] directly.
start=cpp.find('if(descriptor.skillCode == SKILL_RESURRECTION)')
end=cpp.find('if(descriptor.skillCode == SKILL_FORCE_OF_NATURE', start)
block=cpp[start:end] if start>=0 and end>start else ''
checks={
 'support resurrection branch found':bool(block),
 'no direct dead target life write':'deadTarget->smCharInfo.Life[0] =' not in block,
 'revive routed through life commands':'ServerLegacyLifeAdapter().Revive(deadTargetId, 50)' in block,
 'legacy half-life amount preserved':'Revive(deadTargetId, 50)' in block,
 'target clear retained':'ClearAcquiredTarget(deadTargetId)' in block,
 'restart motion retained':'CHRMOTION_STATE_RESTART' in block,
 'post revive side effects require success':'if(reviveResult.Ok())' in block,
}
for n,ok in checks.items():print(('PASS' if ok else 'FAIL')+': '+n)
if not all(checks.values()):sys.exit(1)
print(f'companion resurrection authority v2: PASS ({len(checks)}/{len(checks)})')

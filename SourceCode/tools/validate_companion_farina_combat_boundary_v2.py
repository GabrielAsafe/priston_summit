#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='replace')
checks={
 'farina delta preserves legacy formula':'farinaAttackRating.attackRating = Farina_AttackRate[point - 1] + caster->smCharInfo.Level' in cpp,
 'farina applies through combat buff boundary':'ServerLegacyCombatBuffAdapter().Apply' in cpp and 'farinaAttackRating' in cpp,
 'farina removes through combat buff boundary':'ServerLegacyCombatBuffAdapter().Remove' in cpp and 'farinaAttackRating' in cpp,
 'no direct farina attack rating increment':'caster->smCharInfo.Attack_Rating += Farina_AttackRate' not in cpp,
 'no saved direct attack rating restore':'savedAttackRating' not in cpp,
}
for n,ok in checks.items():print(('PASS' if ok else 'FAIL')+': '+n)
if not all(checks.values()):sys.exit(1)
print(f'companion Farina combat boundary v2: PASS ({len(checks)}/{len(checks)})')

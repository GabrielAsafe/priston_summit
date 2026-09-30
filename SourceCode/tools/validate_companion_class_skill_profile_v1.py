#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
profile = (root / 'Shared/gameplay/companions/v2/CompanionSkillProfile.h').read_text(errors='replace')
executor = (root / 'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='replace')
system = (root / 'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
smoke = (root / 'Shared/gameplay/companions/v2/tests/companion_skill_profile_smoke.cpp').read_text(errors='replace')

mappings = [
    '{1, 0x02010004u, false}',
    '{2, 0x01020001u, false}',
    '{3, 0x04010003u, false}',
    '{4, 0x03010004u, false}',
    '{5, 0x06020002u, false}',
    '{6, 0x05010001u, false}',
    '{7, 0x08010002u, false}',
    '{8, 0x07010002u, true}',
]
checks = [
    ('all eight historical default skill mappings preserved', all(x in profile for x in mappings)),
    ('profile exposes damage lookup', 'DefaultDamageSkillForClass' in profile),
    ('profile exposes healing role', 'HasCompanionHealingRole' in profile),
    ('executor consumes canonical damage profile', 'DefaultDamageSkillForClass' in executor),
    ('executor consumes canonical heal role', 'HasCompanionHealingRole' in executor),
    ('system consumes canonical heal role', system.count('HasCompanionHealingRole') >= 2),
    ('executor has no concrete JOBCODE symbols', 'JOBCODE_' not in executor),
    ('system has no concrete JOBCODE symbols', 'JOBCODE_' not in system),
    ('portable smoke covers unknown classes', 'DefaultDamageSkillForClass(9) == 0u' in smoke and '!HasCompanionHealingRole(9)' in smoke),
]
failed = [label for label, ok in checks if not ok]
for label, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + label)
if failed:
    sys.exit(1)
print('companion class skill profile v1: PASS')

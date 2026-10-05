#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
character = (root / 'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
active_character = re.sub(r'/\*.*?\*/', '', character, flags=re.S)
active_character = re.sub(r'//.*', '', active_character)
policy = (root / 'Shared/gameplay/combat/v2/ActorDamageLifePolicy.h').read_text(errors='replace')
runner = root / 'Shared/gameplay/combat/v2/tests/run_actor_damage_life_policy_tests.sh'

checks = {
    'character includes actor damage life policy': '#include "gameplay/combat/v2/ActorDamageLifePolicy.h"' in character,
    'normal actor damage no direct life subtraction': 'lpTargetChar->smCharInfo.Life[0] -= cnt;' not in active_character,
    'normal actor damage resolves V2 policy': 'ActorDamageLifePolicy::Resolve(' in character,
    'normal actor damage consumes through life commands': 'ServerLegacyLifeAdapter().ConsumeLethal(targetId, lifePlan.amount)' in character,
    'linked quest actor death uses life command': 'lpChar->smCharInfo.Life[0] = 0;' not in active_character,
    'all active target zero assignments retired': active_character.count('lpTargetChar->smCharInfo.Life[0] = 0;') == 0,
    'kelvezu forced zero uses life command': 'ServerLegacyLifeAdapter().Deplete(' in character,
    'post-damage source self-kill uses life command': re.search(r'(?<!->)smCharInfo\.Life\[0\]\s*=\s*0;', active_character) is None,
    'quest npc life floor uses life command': 'ServerLegacyLifeAdapter().EnsureMinimum(' in character,
    'virtual life reset uses life command': 'ServerLegacyLifeAdapter().RestoreToMaximum(' in character,
    'direct exact life floor assignment retired': 'smCharInfo.Life[0] = (smCharInfo.Life[1] >> 4);' not in active_character,
    'direct virtual max assignment retired': 'smCharInfo.Life[0] = smCharInfo.Life[1];' not in active_character,
    'virtual life uses non-lethal command': 'ServerLegacyLifeAdapter().ConsumeNonLethal(targetId, lifePlan.amount)' in character,
    'policy models virtual life absorption': 'virtualLifeAbsorbed' in policy and 'consumeNonLethal' in policy,
    'portable smoke exists': runner.exists(),
}
failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('character actor damage life authority v3: PASS')

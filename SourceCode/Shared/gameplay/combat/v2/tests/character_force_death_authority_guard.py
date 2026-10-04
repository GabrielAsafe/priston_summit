#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[5]
character = (ROOT / 'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(encoding='utf-8', errors='replace')
adapter = (ROOT / 'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyLifeAdapter.cpp').read_text(encoding='utf-8', errors='replace')

# This branch predates the dedicated Kill() command used by the closeout donor.
# Its canonical lethal command is ConsumeLethal(); guard that authority rather
# than reintroducing an older API solely to satisfy the textual closeout guard.
assert character.count('ServerLegacyLifeAdapter().ConsumeLethal(') >= 4, 'expected lethal life paths behind V2 life commands'
lethal_order = re.compile(
    r'entity->smCharInfo\.Life\[0\] = static_cast<short>\(after\);[ \t]*\r?\n'
    r'[ \t]*if\(after <= 0\)[ \t]*\r?\n[ \t]*entity->SetMotionFromCode\(CHRMOTION_STATE_DEAD\);'
)
assert lethal_order.search(adapter), 'ConsumeLethal life-first death ordering changed'

# Preserve the one observed motion-first linked-character legacy sequence.
motion_first = re.compile(
    r'lpChar->SetMotionFromCode\(CHRMOTION_STATE_DEAD\);[ \t]*\r?\n[ \t]*'
    r'lpChar->smCharInfo\.Life\[0\] = 0;'
)
assert motion_first.search(character), 'motion-first force-death ordering changed'

# Kelvezu forced-zero remains a distinct semantic path.
kelvezu_forced_zero = re.compile(
    r'lpTargetChar->smCharInfo\.Life\[0\] = 0;[ \t]*\r?\n[ \t]*cnt = 1;'
)
assert kelvezu_forced_zero.search(character), 'Kelvezu forced-zero ordering changed'

print('character force-death authority guard: PASS')

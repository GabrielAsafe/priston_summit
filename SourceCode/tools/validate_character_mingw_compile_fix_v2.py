#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
path = root / 'SrcServer/src/systems/gameplay/character/core/character.cpp'
text = path.read_text(errors='replace')

checks = []
checks.append(('poison max promotes PlayPoison[0] to int',
               'std::max(0, static_cast<int>(PlayPoison[0]))' in text))
checks.append(('old mixed-type std::max removed',
               'std::max(0, PlayPoison[0])' not in text))

needle = '''case smCHAR_STATE_ENEMY:\n\t\t\t{\n\n\t\t\t\tdwBattleTime = dwPlayTime;'''
checks.append(('SendTransAttack enemy case has lexical scope', needle in text))
checks.append(('ScopedAttackRatingBuff remains inside scoped case',
               'gameplay::combat::legacy::ScopedAttackRatingBuff attackRatingBuff(this, attackRatingDelta);' in text))

failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(f"[{'PASS' if ok else 'FAIL'}] {name}")
if failed:
    print(f'FAIL: {len(failed)} invariant(s)')
    sys.exit(1)
print(f'PASS: {len(checks)}/{len(checks)} MinGW compile-fix invariants')

#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
start=text.find('int smCHAR::EventSkill()')
end=text.find('\nint smCHAR::', start+10)
if start < 0 or end < 0:
    print('FAIL: EventSkill body not found',file=sys.stderr); sys.exit(1)
body=text[start:end]
fail=[]
writes=re.findall(r'smCharInfo\.Attack_Rating\s*(?:\+=|-=|=)',body)
if writes: fail.append(f'direct Attack_Rating writes remain: {len(writes)}')
if body.count('ScopedAttackRatingBuff attackRatingBuff') != 4:
    fail.append(f'expected 4 scoped buffs, found {body.count("ScopedAttackRatingBuff attackRatingBuff")}')
if body.count('attackRatingBuff.Release()') != 4:
    fail.append(f'expected 4 releases, found {body.count("attackRatingBuff.Release()")}')
if 'LegacyScopedCombatBuff.h' not in text[:start]:
    fail.append('character core missing scoped buff adapter include')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('EventSkill attack-rating authority v2: PASS (4/4 scoped; direct writes=0)')

#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
start=text.find('int smCHAR::SendTransAttack(')
end=text.find('\nint smCHAR::PlayAttackFromPosi(', start+10)
if start < 0 or end < 0:
    print('FAIL: SendTransAttack body not found', file=sys.stderr); sys.exit(1)
body=text[start:end]
fail=[]
if re.search(r'smCharInfo\.Attack_Rating\s*(?:\+=|-=|=)', body):
    fail.append('direct Attack_Rating write remains in SendTransAttack')
if body.count('ScopedAttackRatingBuff attackRatingBuff') != 1:
    fail.append('expected one scoped attack-rating buff')
if body.count('attackRatingBuff.Release()') != 1:
    fail.append('expected one scoped buff release')
if 'SKILL_SCOUT_HAWK' not in body or 'GetSkillPower' not in body:
    fail.append('Scout Hawk accuracy path missing')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('SendTransAttack attack-rating authority v2: PASS')

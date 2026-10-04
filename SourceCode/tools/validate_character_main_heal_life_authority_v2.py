#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
start=text.find('int smCHAR::Main()')
end=text.find('\nint smCHAR::',start+10)
body=text[start:end] if start>=0 and end>=0 else ''
fail=[]
patterns=[r'smCharInfo\.Life\[0\]\s*\+=\s*\(smCharInfo\.Life\[1\]\s*>>\s*1\)',
          r'smCharInfo\.Life\[0\]\s*\+=\s*\(smCharInfo\.Life\[1\]\s*>>\s*2\)']
for pat in patterns:
    if re.search(pat,body): fail.append('direct potion/NPC heal Life write remains')
if body.count('ServerLegacyLifeAdapter().Restore(') < 5:
    fail.append('expected monster regen + two heal restore delegations in Main')
if 'PotionLog == 1' not in body or 'snCHAR_SOUND_NPC_MAGICMASTER' not in body:
    fail.append('legacy heal triggers not preserved')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('character Main heal life authority v2: PASS (2/2 heals delegated)')

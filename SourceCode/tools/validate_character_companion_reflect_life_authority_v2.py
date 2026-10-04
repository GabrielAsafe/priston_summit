#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
start=text.find('int smCHAR::Main()')
end=text.find('\nint smCHAR::',start+10)
body=text[start:end] if start>=0 and end>=0 else ''
fail=[]
if body.count('GetReactiveDamage(lpTargetChar, cnt)') != 2:
    fail.append('expected 2 companion reflect sites')
if body.count('ServerLegacyLifeAdapter().ConsumeLethal(') < 2:
    fail.append('reflect sites are not delegated to lethal life commands')
if re.search(r'smCharInfo\.Life\[0\]\s*=\s*\(short\)std::max\(0,\s*\(int\)smCharInfo\.Life\[0\]\s*-\s*companionReflect\)', body):
    fail.append('direct companion reflect Life write remains')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('character companion reflect life authority v2: PASS (2/2 delegated)')

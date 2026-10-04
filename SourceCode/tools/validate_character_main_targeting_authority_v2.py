#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
start=text.find('int smCHAR::Main()')
end=text.find('\nint smCHAR::', start+10)
if start < 0 or end < 0:
    print('FAIL: smCHAR::Main body not found', file=sys.stderr); sys.exit(1)
body=text[start:end]
fail=[]
writes=re.findall(r'(?:\b|->)lpTargetPlayInfo\s*=\s*(?:0|NULL)', body)
if writes:
    fail.append(f'direct lpTargetPlayInfo clears remain in Main: {len(writes)}')
calls=body.count('SummitClearCharacterPlayerTarget(')
if calls != 9:
    fail.append(f'expected 9 targeting delegations, found {calls}')
if 'ServerLegacyTargetingAdapter().ClearPlayerTarget(' not in text[:start]:
    fail.append('targeting helper is not backed by V2 TargetingService adapter')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('character Main targeting authority v2: PASS (9/9 player-target clears delegated)')

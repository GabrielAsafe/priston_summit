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
restore='ServerLegacyLifeAdapter().Restore('
if body.count(restore) < 3:
    fail.append(f'expected at least 3 life restore delegations in Main, found {body.count(restore)}')
if re.search(r'smCharInfo\.Life\[0\]\s*\+=\s*rsGetMonsterLifeRegen\(this\)', body):
    fail.append('direct monster regen Life write remains')
if body.count('rsGetMonsterLifeRegen(this)') != 3:
    fail.append(f'expected 3 monster regen calls, found {body.count("rsGetMonsterLifeRegen(this)")}')
if fail:
    [print('FAIL:', x, file=sys.stderr) for x in fail]; sys.exit(1)
print('character monster regen life authority v2: PASS (3/3 delegated)')

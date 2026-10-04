#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
start=text.find('int smCHAR::EventSkill()'); end=text.find('\nint smCHAR::',start+10); body=text[start:end]
fail=[]
if re.findall(r'(?:lpCurPlayer->)?MotionLoopSpeed\s*(?:=|\+=|-=)',body): fail.append('direct MotionLoopSpeed writes remain')
if re.findall(r'(?:lpCurPlayer->)?MotionLoop\s*(?:=|\+=|-=)',body): fail.append('direct MotionLoop writes remain')
if body.count('SummitConfigureCharacterSkillLoop(')!=2: fail.append('expected two V2 animation calls in EventSkill')
if 'ServerLegacySkillAnimationAdapter().ConfigureLoop' not in text[:start]: fail.append('V2 animation helper missing')
if fail:[print('FAIL:',x,file=sys.stderr) for x in fail];sys.exit(1)
print('EventSkill animation authority v2: PASS (direct MotionLoop writes=0)')

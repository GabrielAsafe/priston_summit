#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'Shared/gameplay/skills/SkillSub_legacy_server.inl').read_text(errors='replace')
start=text.find('int PlaySkillAttack('); end=text.find('\nint ',start+10)
if start<0: print('FAIL: PlaySkillAttack missing',file=sys.stderr);sys.exit(1)
body=text[start:end if end>=0 else len(text)]
fail=[]
speed_writes=re.findall(r'lpCurPlayer->MotionLoopSpeed\s*(?:=|\+=|-=)',body)
loop_writes=re.findall(r'lpCurPlayer->MotionLoop\s*(?:=|\+=|-=)',body)
if speed_writes: fail.append(f'MotionLoopSpeed direct writes remain: {len(speed_writes)}')
if loop_writes: fail.append(f'MotionLoop direct writes remain: {len(loop_writes)}')
if body.count('SummitConfigurePlaySkillLoop(') < 36: fail.append('expected all 35 configurations plus Ravind branch to use V2 helper')
head=text[:start]
for needle in ('ServerLegacySkillAnimationAdapter().ConfigureLoop','LegacySkillAnimationAdapter.h'):
    if needle not in head: fail.append('missing '+needle)
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail];sys.exit(1)
print('PlaySkillAttack animation authority v2: PASS (direct MotionLoop writes=0)')

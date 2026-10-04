#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
path=ROOT/'Shared/gameplay/skills/SkillSub_legacy_server.inl'
text=path.read_text(errors='replace')
start=text.find('int PlaySkillAttack(')
if start < 0:
    print('FAIL: PlaySkillAttack not found', file=sys.stderr); sys.exit(1)
end=text.find('\nint ', start+10)
body=text[start:end if end>=0 else len(text)]
fail=[]
writes=re.findall(r'smCharInfo\.Attack_Rating\s*(?:\+=|-=|=)', body)
if writes: fail.append(f'direct Attack_Rating writes remain in PlaySkillAttack: {len(writes)}')
if body.count('ScopedAttackRatingBuff attackRatingBuff') != 11:
    fail.append(f'expected 11 scoped attack-rating buffs, found {body.count("ScopedAttackRatingBuff attackRatingBuff")}')
if body.count('attackRatingBuff.Release()') != 11:
    fail.append(f'expected 11 explicit releases, found {body.count("attackRatingBuff.Release()")}')
helper=(ROOT/'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyScopedCombatBuff.h').read_text(errors='replace')
for needle,label in [
    ('ServerLegacyCombatBuffAdapter().Apply','scoped helper applies via V2 adapter'),
    ('ServerLegacyCombatBuffAdapter().Remove','scoped helper removes via V2 adapter'),
    ('~ScopedAttackRatingBuff()','scoped helper has RAII fallback')]:
    if needle not in helper: fail.append(label)
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('PlaySkillAttack attack-rating authority v2: PASS (11/11 scoped; direct writes=0)')

#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
cpp=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
cat=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
client=(ROOT/'Shared/gameplay/character/character_legacy_client.inl').read_text(errors='ignore')
legacy=(ROOT/'Shared/gameplay/skills/SkillSub_legacy_server.inl').read_text(errors='ignore')
err=[]
line=next((x for x in cat.splitlines() if 'SKILL_FARINA' in x),'')
if ', true,' not in line: err.append('Farina not READY')
for tok in ['case SKILL_FARINA:', 'Farina_AttackRate[point - 1] + caster->smCharInfo.Level',
            'GetAttackSpeedFrame(caster->smCharInfo.Attack_Speed, Farina_Speed[point - 1])',
            'ServerLegacySkillAnimationAdapter().ConfigureLoop(']:
    if tok not in cpp: err.append('server missing '+tok)
for tok in ['SKILL_PLAY_FARINA', 'GetAttackSpeedFrame(character->smCharInfo.Attack_Speed, Farina_Speed[point - 1])',
            'character->MotionLoop = 1']:
    if tok not in client: err.append('client visual missing '+tok)
for tok in ['Farina_AttackRate[lpSkill->Point - 1] + lpCurPlayer->smCharInfo.Level',
            'GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, Farina_Speed[lpSkill->Point - 1])']:
    if tok not in legacy: err.append('legacy oracle missing '+tok)
ready=len(re.findall(r'\{\s*SKILL_[A-Z0-9_]+,\s*COMPANION_SKILL_ROLE_[A-Z_]+,\s*true,',cat))
if ready < 107: err.append(f'expected at least 107 READY after V24, got {ready}')
if err:
    print('Companion Farina V24: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Farina V24: PASS ({ready} READY; attack-rate + motion-speed semantics)')

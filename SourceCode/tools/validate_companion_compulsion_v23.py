#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
cat=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
cpp=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
h=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.h').read_text(errors='ignore')
ai=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='ignore')
err=[]
line=next((x for x in cat.splitlines() if 'SKILL_COMPULSION' in x),'')
if ', true,' not in line: err.append('Compulsion not READY')
for tok in ['ApplyCompanionCompulsionTaunt','Compulsion_Area[p]','Compulsion_Time[p]','Compulsion_AddAbsorb[p]','lpCompulsionChar = caster']:
 if tok not in cpp: err.append('executor missing '+tok)
if '*lpCompulsionChar;' not in h: err.append('smCHAR companion taunt pointer missing')
for tok in ['lpChar->lpCompulsionChar','lpChar->lpTargetChar = lpChar->lpCompulsionChar','lpChar->lpCompulsionChar = 0']:
 if tok not in ai: err.append('AI missing '+tok)
for boss in ['snCHAR_SOUND_DEATHKNIGHT','snCHAR_SOUND_BABEL','snCHAR_SOUND_KELVEZU','snCHAR_SOUND_MOKOVA']:
 if boss not in cpp: err.append('boss immunity missing '+boss)
ready=len(re.findall(r'\{\s*SKILL_[A-Z0-9_]+,\s*COMPANION_SKILL_ROLE_[A-Z_]+,\s*true,',cat))
if ready < 106: err.append(f'expected at least 106 READY after V23, got {ready}')
if err:
 print('Companion Compulsion V23: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Compulsion V23: PASS ({ready} READY; flat absorb + companion-target taunt)')

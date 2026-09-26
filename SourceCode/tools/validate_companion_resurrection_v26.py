#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
cpp=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
syscpp=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
cat=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
client=(ROOT/'SrcGame/src/systems/network/client/netplay.cpp').read_text(errors='ignore')
finddead=(ROOT/'SrcGame/src/systems/network/client/netplay.cpp').read_text(errors='ignore')
err=[]
line=next((x for x in cat.splitlines() if 'SKILL_RESURRECTION' in x),'')
if ', true,' not in line: err.append('Resurrection not READY')
for tok in ['descriptor.skillCode == SKILL_RESURRECTION','Resurrection_Percent[descriptor.point - 1]',
            'deadTarget->smCharInfo.Life[1] / 2','CHRMOTION_STATE_RESTART','bestDistance = 300 * 300']:
 if tok not in cpp: err.append('executor missing '+tok)
for tok in ['ResurrectionCorpseSeconds','kDeadVisualHoldMs','partySlot.entity->lpMasterPlayInfo == state->owner']:
 if tok not in syscpp: err.append('lifecycle missing '+tok)
if 'sinSetLife(lpCurPlayer->smCharInfo.Life[1] / 2)' not in client: err.append('legacy half-life oracle missing')
if 'NearDist = 300 * 300' not in finddead: err.append('legacy 300 range oracle missing')
ready=len(re.findall(r'\{\s*SKILL_[A-Z0-9_]+,\s*COMPANION_SKILL_ROLE_[A-Z_]+,\s*true,',cat))
if ready < 110: err.append(f'expected >=110 READY, got {ready}')
if err:
 print('Companion Resurrection V26: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Resurrection V26: PASS ({ready} READY; chance + 50% HP + corpse window)')

#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
contract=(ROOT/'Shared/gameplay/monsters/v2/MonsterCommands.h').read_text(errors='replace')
adapter=(ROOT/'SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.cpp').read_text(errors='replace')
char=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
fail=[]
for text,needle,label in [
(contract,'ReleaseFollower','monster command contract release'),
(adapter,'LegacyMonsterAdapter::ReleaseFollower','legacy adapter release implementation'),
(adapter,'target->smCharInfo.Brood = target->PlayHolyIncantation[0]','adapter owns brood restore'),
(adapter,'target->lpMasterPlayInfo = 0','adapter owns master detach'),
(char,'ServerLegacyMonsterAdapter().ReleaseFollower','smCHAR::Main delegates follower expiry')]:
    if needle not in text: fail.append(label)
start=char.find('int smCHAR::Main()'); end=char.find('\nint smCHAR::',start+10); body=char[start:end]
expiry=body[body.find('if(PlayHolyIncantation[1] > 0)'):body.find('if(UseObject_VirtualLife)')]
for forbidden in ('smCharInfo.Brood = PlayHolyIncantation[0]','lpMasterPlayInfo = 0'):
    if forbidden in expiry: fail.append('direct follower expiry mutation remains: '+forbidden)
if fail:[print('FAIL:',x,file=sys.stderr) for x in fail];sys.exit(1)
print('monster follower release v2: PASS')

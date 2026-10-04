#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
start=text.find('int smCHAR::SendTransAttack(')
end=text.find('\nint smCHAR::PlayAttackFromPosi(',start+10)
body=text[start:end] if start>=0 and end>=0 else ''
fail=[]
if 'lpsmSock->Send' in body:
    fail.append(f'direct socket sends remain in SendTransAttack: {body.count("lpsmSock->Send")}')
if body.count('SummitSendCharacterPacket(lpPlayInfo, &smTransCommand, smTransCommand.size);') != 7:
    fail.append(f'expected 7 packet sender delegations, found {body.count("SummitSendCharacterPacket(lpPlayInfo, &smTransCommand, smTransCommand.size);")}')
if body.count('OPCODE_SUCCESS_EVATION') < 6:
    fail.append('evasion packet branches were not preserved')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('SendTransAttack packet sender v2: PASS (7/7 direct sends delegated)')

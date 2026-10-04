#!/usr/bin/env python3
from pathlib import Path
import re,sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
start=text.find('int smCHAR::Main()')
end=text.find('\nint smCHAR::', start+10)
body=text[start:end] if start>=0 and end>=0 else ''
fail=[]
if 'lpsmSock->Send' in body:
    fail.append(f'direct lpsmSock->Send remains in Main: {body.count("lpsmSock->Send")}')
if body.count('SummitSendCharacterPacket(') != 3:
    fail.append(f'expected 3 packet sender delegations, found {body.count("SummitSendCharacterPacket(")}')
if 'ServerLegacyPacketSender().Send(' not in text[:start]:
    fail.append('packet helper is not backed by IPacketSender adapter')
for opcode in ('OPCODE_PROCESS_SKILL2','OPCODE_COMIC_SKIN','OPCODE_DEAD_PLAYER'):
    if opcode not in body: fail.append(f'missing preserved packet opcode {opcode}')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('character Main packet sender v2: PASS (3/3 direct sends delegated)')

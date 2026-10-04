#!/usr/bin/env python3
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
rel='SrcServer/src/systems/combat/server_legacy/CRollHandler.cpp'
text=(ROOT/rel).read_text(errors='replace')
base=json.loads((ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text())
files=base.get('files',base)
fail=[]
if 'lpsmSock->Send' in text: fail.append('direct roll socket send remains')
if text.count('SummitRollSend(') != 6: fail.append(f'expected helper + 5 calls, found {text.count("SummitRollSend(")}')
if 'ServerLegacyPacketSender().Send(' not in text: fail.append('roll helper is not backed by IPacketSender')
if files.get(rel,{}).get('max_counts',{}).get('lpsmSock') != 0: fail.append('roll baseline is not zero')
for token in ('OPCODE_ROLL','OPCODE_ENDROLL','OPCODE_WHISPERMESSAGE'):
    if token not in text: fail.append(f'missing preserved roll opcode {token}')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('roll packet sender v2: PASS (5/5 direct sends delegated, baseline=0)')

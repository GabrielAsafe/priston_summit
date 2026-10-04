#!/usr/bin/env python3
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
rel='SrcServer/src/core/server/legacy_runtime/PartySever.cpp'
text=(ROOT/rel).read_text(errors='replace')
base=json.loads((ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text())
files=base.get('files',base)
fail=[]
if 'lpsmSock->Send' in text: fail.append('direct PartySever socket send remains')
if text.count('SummitPartySend(') != 11: fail.append(f'expected helper + 10 calls, found {text.count("SummitPartySend(")}')
if 'ServerLegacyPacketSender().Send(' not in text: fail.append('PartySever helper is not backed by IPacketSender')
if 'ServerLegacyOnlinePlayerRegistry().IdOf(recipient)' not in text: fail.append('PartySever helper does not resolve PlayerId via registry')
if files.get(rel,{}).get('max_counts',{}).get('lpsmSock') != 0: fail.append('PartySever baseline is not zero')
for token in ('OPCODE_PARTY_RELEASE','OPCODE_PARTY_UPDATE','OPCODE_PARTY_PLAYUPDATE','OPCODE_ADDEXP_PARTY'):
    if token not in text: fail.append(f'missing preserved party opcode {token}')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('party packet sender v2: PASS (10/10 direct sends delegated, baseline=0)')

#!/usr/bin/env python3
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
rel='SrcServer/src/systems/crafting/professions/JOBS.cpp'
text=(ROOT/rel).read_text(errors='replace')
base=json.loads((ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text())
files=base.get('files',base)
fail=[]
if 'lpsmSock->Send' in text:
    fail.append(f'direct JOBS socket sends remain: {text.count("lpsmSock->Send")}')
if text.count('SummitJobsSend(') != 22:
    fail.append(f'expected helper definition + 21 calls, found {text.count("SummitJobsSend(")}')
if 'ServerLegacyPacketSender().Send(' not in text:
    fail.append('JOBS helper is not backed by IPacketSender')
if files.get(rel,{}).get('max_counts',{}).get('lpsmSock') != 0:
    fail.append('JOBS lpsmSock baseline is not zero')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('JOBS packet sender v2: PASS (21/21 direct sends delegated, baseline=0)')

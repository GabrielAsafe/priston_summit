#!/usr/bin/env python3
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
rel='SrcServer/src/systems/progression/class_rank/Rank.cpp'
text=(ROOT/rel).read_text(errors='replace')
base=json.loads((ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text())
files=base.get('files',base)
fail=[]
if 'lpsmSock->Send' in text: fail.append('direct class-rank socket send remains')
if text.count('SummitClassRankSend(') != 5: fail.append(f'expected helper + 4 calls, found {text.count("SummitClassRankSend(")}')
if 'ServerLegacyPacketSender().Send(' not in text: fail.append('class-rank helper is not backed by IPacketSender')
if files.get(rel,{}).get('max_counts',{}).get('lpsmSock') != 0: fail.append('class-rank baseline is not zero')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('class rank packet sender v2: PASS (4/4 direct sends delegated, baseline=0)')

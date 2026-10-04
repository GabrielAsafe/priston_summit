#!/usr/bin/env python3
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
rel='SrcServer/src/systems/community/news/News.cpp'
text=(ROOT/rel).read_text(errors='replace')
base=json.loads((ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text())
files=base.get('files',base)
fail=[]
if 'lpsmSock->Send' in text:
    fail.append(f'direct news socket sends remain: {text.count("lpsmSock->Send")}')
if text.count('SummitNewsSend(') != 7:
    fail.append(f'expected helper definition + 6 calls, found {text.count("SummitNewsSend(")}')
if 'ServerLegacyPacketSender().Send(' not in text:
    fail.append('News helper is not backed by IPacketSender')
if files.get(rel,{}).get('max_counts',{}).get('lpsmSock') != 0:
    fail.append('News lpsmSock baseline is not zero')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('news packet sender v2: PASS (6/6 direct sends delegated, baseline=0)')

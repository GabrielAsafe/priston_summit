#!/usr/bin/env python3
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
rel='SrcServer/src/systems/gameplay/character/core/character.cpp'
text=(ROOT/rel).read_text(errors='replace')
baseline=json.loads((ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text())
files=baseline.get('files', baseline)
fail=[]
if 'lpsmSock->Send' in text:
    fail.append(f'direct lpsmSock->Send remains in character core: {text.count("lpsmSock->Send")}')
entry=files.get(rel,{})
if entry.get('max_counts',{}).get('lpsmSock') != 0:
    fail.append('character core lpsmSock baseline is not frozen at zero')
if entry.get('role') != 'RED':
    fail.append('character core role unexpectedly changed')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('character socket retirement v2: PASS (lpsmSock direct sends=0, baseline=0)')

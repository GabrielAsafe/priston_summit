#!/usr/bin/env python3
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
rel='SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp'
text=(ROOT/rel).read_text(errors='replace')
base=json.loads((ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text())
files=base.get('files',base)
fail=[]
if 'lpsmSock->Send' in text: fail.append('direct QuestHandler socket send remains')
if text.count('SummitQuestSend(') != 8: fail.append(f'expected helper + 7 calls, found {text.count("SummitQuestSend(")}')
if 'ServerLegacyPacketSender().Send(' not in text: fail.append('QuestHandler helper is not backed by IPacketSender')
if files.get(rel,{}).get('max_counts',{}).get('lpsmSock') != 0: fail.append('QuestHandler baseline is not zero')
for token in ('OPCODE_SPOILER_INFO','OPCODE_QUEST_INFO'):
    if token not in text: fail.append(f'missing preserved quest opcode {token}')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('quest handler packet sender v2: PASS (7/7 direct sends delegated, baseline=0)')

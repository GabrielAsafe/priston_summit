#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/social/party/v2/PartyCommands.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/social/party/v2/legacy/LegacyPartyCommandsAdapter.cpp').read_text(errors='replace')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
start=onserver.index('\t\tcase OpCode::OPCODE_PARTY_JOIN:')
end=onserver.index('\n\n\t\tcase OpCode::OPCODE_PARTY_COMMAND:', start)
body=onserver[start:end]
checks=[
 ('green party commands contract', 'class IPartyCommands' in contract and 'Join(PlayerId requester, PlayerId target)' in contract),
 ('adapter resolves requester and target', 'players_.Resolve(requesterId)' in adapter and 'players_.Resolve(targetId)' in adapter),
 ('target party join preserved', 'requester->JoinParty(target->lpPartyMaster)' in adapter),
 ('requester party invite preserved', 'target->JoinParty(requester->lpPartyMaster)' in adapter),
 ('new party creation preserved', 'target->CreateParty(requester)' in adapter),
 ('onserver delegates join', 'ServerLegacyPartyCommandsAdapter().Join' in body),
 ('onserver authenticates requester', 'ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in body),
 ('onserver no target lookup', 'srFindUserFromSerial' not in body),
 ('onserver no direct party mutation', all(token not in body for token in ['->JoinParty(', '->CreateParty(', 'lpPartyMaster']))
]
failed=[]
for label,ok in checks:
    print(('PASS: ' if ok else 'FAIL: ')+label)
    if not ok: failed.append(label)
if failed: sys.exit(1)
print('party commands join boundary: PASS')

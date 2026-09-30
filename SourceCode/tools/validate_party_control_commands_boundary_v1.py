#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/social/party/v2/PartyCommands.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/social/party/v2/legacy/LegacyPartyCommandsAdapter.cpp').read_text(errors='replace')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
start=onserver.index('\t\tcase OpCode::OPCODE_PARTY_COMMAND:')
end=onserver.index('\n\n\t\tcase OpCode::OPCODE_PARTY_PLAYUPDATE:', start)
body=onserver[start:end]
checks=[
 ('contract exposes control commands', all(token in contract for token in ['TransferLeadership','Release(PlayerId requester)','Kick(PlayerId requester, PlayerId target)','Leave(PlayerId requester)'])),
 ('adapter preserves leadership transfer', 'requester->LeavePartyMaster(target)' in adapter),
 ('adapter preserves party release', 'requester->ReleaseParty()' in adapter),
 ('adapter preserves kick semantics', 'target->SecedeParty()' in adapter),
 ('adapter preserves leave semantics', 'requester->SecedeParty()' in adapter),
 ('adapter owns party state precondition', adapter.count('!requester->dwPartyInfo || !requester->lpPartyMaster') >= 4),
 ('onserver delegates leadership transfer', 'partyCommands.TransferLeadership(requesterId, targetId)' in body),
 ('onserver delegates release', 'partyCommands.Release(requesterId)' in body),
 ('onserver delegates kick', 'partyCommands.Kick(requesterId, targetId)' in body),
 ('onserver delegates leave', 'partyCommands.Leave(requesterId)' in body),
 ('onserver preserves feedback messages', all(token in body for token in ['srMsg_026','srMsg_025','srMsg_023','srMsg_022'])),
 ('onserver has no direct party control mutation', all(token not in body for token in ['LeavePartyMaster(', 'ReleaseParty(', 'SecedeParty('])),
]
failed=[]
for label,ok in checks:
    print(('PASS: ' if ok else 'FAIL: ')+label)
    if not ok: failed.append(label)
if failed: sys.exit(1)
print('party control commands boundary: PASS')

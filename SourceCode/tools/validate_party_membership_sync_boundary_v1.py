#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/social/party/v2/PartyCommands.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/social/party/v2/legacy/LegacyPartyCommandsAdapter.cpp').read_text(errors='replace')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
start=onserver.index('\t\tcase OpCode::OPCODE_PARTY_PLAYUPDATE:')
end=onserver.index('\n\n\t\tcase OpCode::OPCODE_PARTY_POTION:', start)
body=onserver[start:end]
checks=[
 ('green membership sync dto', 'struct PartyMembershipSync' in contract and 'std::vector<PlayerId> members' in contract),
 ('commands expose sync', 'SynchronizeMembership(PlayerId receiver, const PartyMembershipSync& sync)' in contract),
 ('adapter preserves max-count rejection', 'sync.members.size() >= static_cast<std::size_t>(PARTY_PLAYER_MAX)' in adapter),
 ('adapter preserves unchanged-party fast path', 'incomingCode == currentCode' in adapter and 'currentCount == static_cast<int>(sync.members.size())' in adapter),
 ('adapter clears old master members', 'receiver->lpPartyPlayers[index]->dwPartyInfo = PARTY_STATE_NONE' in adapter),
 ('adapter rebuilds master slot', 'receiver->dwPartyInfo = PARTY_STATE_MASTER' in adapter and 'receiver->lpPartyMaster = receiver' in adapter),
 ('adapter rebuilds member slots', 'member->dwPartyInfo = PARTY_STATE_MEMBER' in adapter and 'member->lpPartyMaster = receiver' in adapter),
 ('adapter preserves count average', 'receiver->PartyUserCount = static_cast<int>(sync.members.size())' in adapter and 'levelAverage / receiver->PartyUserCount' in adapter),
 ('onserver parses only membership ids', 'PartyMembershipSync sync' in body and 'dwObjectSerial' in body),
 ('onserver delegates membership sync', 'ServerLegacyPartyCommandsAdapter().SynchronizeMembership' in body),
 ('onserver no direct legacy update call', 'UpdateServerPartyPlayer' not in body),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: sys.exit(1)
print('party membership sync boundary: PASS')

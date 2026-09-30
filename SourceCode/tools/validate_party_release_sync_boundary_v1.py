#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/social/party/v2/PartyCommands.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/social/party/v2/legacy/LegacyPartyCommandsAdapter.cpp').read_text(errors='replace')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
start=onserver.index('\t\tcase OpCode::OPCODE_PARTY_RELEASE:')
end=onserver.index('\n\n\t\tcase OpCode::OPCODE_WAREHOUSE:', start)
body=onserver[start:end]
checks=[
 ('contract exposes clear membership', 'ClearMembership(PlayerId member)' in contract),
 ('adapter resolves member', 'players_.Resolve(memberId)' in adapter),
 ('adapter clears party state', 'member->dwPartyInfo = PARTY_STATE_NONE' in adapter and 'member->lpPartyMaster = 0' in adapter),
 ('onserver delegates release sync', 'ServerLegacyPartyCommandsAdapter().ClearMembership' in body),
 ('onserver authenticates member', 'ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in body),
 ('onserver has no direct party state clear', 'dwPartyInfo = 0' not in body and 'lpPartyMaster = 0' not in body),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: sys.exit(1)
print('party release sync boundary: PASS')

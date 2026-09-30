#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/quests/v2/QuestAdminService.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestAdminService.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_bytes().decode('latin1')
checks=[
 ('contract exposes admin service','class IQuestAdminService' in contract and 'MarkCompletedForTesting' in contract),
 ('adapter resolves player','ServerLegacyOnlinePlayerRegistry().Resolve(player)' in adapter),
 ('adapter owns repository test completion','ServerLegacyQuestRepository().MarkSingleCompletedForTesting' in adapter),
 ('adapter has no quest handler dependency','cQuestHandler' not in adapter and 'cQuestHandler.h' not in adapter),
 ('onserver no quest handler include','systems/gameplay/quests/server/cQuestHandler.h' not in onserver),
 ('onserver no direct quest handler','cQuestHandler.' not in onserver),
 ('onserver delegates moderator completion','ServerLegacyQuestAdminService().MarkCompletedForTesting' in onserver),
 ('tier5 dev quest ids preserved', all(token in onserver for token in [
    'QUESTID_T5_A_MATTER_OF_STRENGTH','QUESTID_T5_CRY_ME_A_RIVER','QUESTID_T5_THE_LAST_STAGE','QUESTID_T5_QUEST_ARENA'])),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: raise SystemExit(1)
print('quest admin boundary: PASS')

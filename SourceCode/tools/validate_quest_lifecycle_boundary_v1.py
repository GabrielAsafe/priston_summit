#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/quests/v2/QuestLifecycleService.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestLifecycleService.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_bytes().decode('latin1')
checks=[
 ('contract exposes lifecycle service','class IQuestLifecycleService' in contract and 'VerifyPlayerLevel' in contract),
 ('adapter resolves player','ServerLegacyOnlinePlayerRegistry().Resolve(player)' in adapter),
 ('adapter composes repository','ServerLegacyQuestRepository().ListInvalidForLevel' in adapter and 'cQuestHandler' not in adapter),
 ('adapter cancels through command port','ServerLegacyQuestCommands().Cancel' in adapter),
 ('onserver no direct verify level','cQuestHandler.VerifyLevel' not in onserver),
 ('onserver delegates lifecycle',onserver.count('ServerLegacyQuestLifecycleService().VerifyPlayerLevel') == 2),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: raise SystemExit(1)
print('quest lifecycle boundary: PASS')

#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/quests/v2/QuestObjectiveService.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestObjectiveService.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_bytes().decode('latin1')
checks=[
 ('contract exposes objective service','class IQuestObjectiveService' in contract),
 ('contract has in-progress query','IsInProgress' in contract),
 ('contract has pending query','HasPendingObjectives' in contract),
 ('contract has complete command','CompleteObjectives' in contract),
 ('contract has cancel command','CancelProgress' in contract),
 ('adapter composes repository','ServerLegacyQuestRepository()' in adapter and 'cQuestHandler' not in adapter),
 ('adapter reads in-progress through repository','TryIsInProgress' in adapter),
 ('adapter reads pending through repository','TryHasPendingObjectives' in adapter),
 ('adapter completes through repository','CompleteObjectives' in adapter),
 ('adapter cancels through repository','CancelProgress' in adapter),
 ('onserver has no direct objective progress query','cQuestHandler.IsQuestInProgress' not in onserver),
 ('onserver has no direct pending query','cQuestHandler.HasPendingQuestObjectives' not in onserver),
 ('onserver has no direct objective complete','cQuestHandler.CompleteQuestObjectives' not in onserver),
 ('onserver has no direct silent cancel','cQuestHandler.CancelQuestProgressSilent' not in onserver),
 ('arena quest id preserved','gameplay::QuestId(QUESTID_T5_QUEST_ARENA)' in onserver),
 ('cry quest id preserved','gameplay::QuestId(QUESTID_T5_CRY_ME_A_RIVER)' in onserver),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: raise SystemExit(1)
print('quest objective boundary: PASS')

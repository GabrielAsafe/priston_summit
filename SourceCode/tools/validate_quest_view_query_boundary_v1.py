#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/quests/v2/QuestViewQuery.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestViewQuery.cpp').read_text(errors='replace')
handler=(root/'SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp').read_text(errors='replace')
body=handler[handler.index('void cQUESTHANDLER::GetQuestInfo'):handler.index('static bool EnsureTier5ArenaObjective')]
checks=[
 ('green quest view query contract', 'class IQuestViewQuery' in contract and 'QuestDetailView' in contract),
 ('detail query supports three legacy kinds', all(token in adapter for token in ['RepeatedQuest','DailyQuest','SingleQuest'])),
 ('detail query loads objectives', 'QuestMonsters' in adapter and 'MonsterKilled' in adapter and 'out.objectives' in adapter),
 ('detail query loads rewards', 'SELECT * FROM Rewards' in adapter and 'SELECT * FROM ItemRewards' in adapter),
 ('current ten-entry objective cap preserved', 'kLegacyQuestFullEntryMax = 10u' in adapter and 'defCount) < kLegacyQuestFullEntryMax' in adapter),
 ('current ten-entry reward cap preserved', 'out.itemRewards.size() < kLegacyQuestFullEntryMax' in adapter),
 ('handler delegates quest detail', 'ServerLegacyQuestViewQuery().TryGetDetail' in body),
 ('handler no direct quest db', 'SQLConnection' not in body and 'db->Prepare' not in body),
 ('handler preserves quest info packet', 'OPCODE_QUEST_INFO' in body and 'smTRANS_QUEST_FULL' in body and 'QUEST_FULL_ENTRY_MAX' in body),
 ('handler preserves TCoin reward grant', 'reward.tCoin' in handler and 'cCoinShop_T.AddPlayerCoin' in handler),
 ('handler resolves reward names through item catalog', 'ServerLegacyItemCatalogAdapter().TryGetDefinitionByContentKey' in body and 'itemDefinition.displayName' in body),
 ('handler has no direct legacy item tables', 'sItem[' not in body and 'DefaultItems[' not in body and 'sinItem.h' not in handler),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: sys.exit(1)
print('quest view query boundary: PASS')

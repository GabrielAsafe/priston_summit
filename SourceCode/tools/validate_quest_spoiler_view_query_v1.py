#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/quests/v2/QuestViewQuery.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestViewQuery.cpp').read_text(errors='replace')
handler=(root/'SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp').read_text(errors='replace')
body=handler[handler.index('void cQUESTHANDLER::GetSpoilerInfo'):handler.index('#include "core/server/legacy_runtime/onserver.h"')]
checks=[
 ('contract exposes spoiler batches', 'QuestSpoilerEntryView' in contract and 'QuestSpoilerBatchView' in contract and 'TryGetSpoilerBatches' in contract),
 ('adapter supports legacy spoiler kinds', all(token in adapter for token in ['spoilerNumber != 21','spoilerNumber != 7','spoilerNumber != 9','spoilerNumber != 10','spoilerNumber != 11'])),
 ('progress state query preserved', 'SELECT * FROM Progress WHERE QuestKind=? AND PlayerName=?' in adapter),
 ('daily completion day gate preserved', "SELECT * FROM Complete WHERE QuestKind=? AND PlayerName=? AND DAY(GETDATE()) <= DAY(EndDate) AND MONTH(GETDATE())=MONTH(EndDate)" in adapter),
 ('single completion query preserved', 'SELECT * FROM Complete WHERE QuestKind=? AND PlayerName=?' in adapter),
 ('level eligible queries preserved', all(token in adapter for token in ['RepeatedQuest WHERE LevelMin <= ? AND LevelMax >= ?','DailyQuest WHERE LevelMin <= ? AND LevelMax >= ?','SingleQuest WHERE LevelMin <= ? AND LevelMax >= ?'])),
 ('level ineligible queries preserved', all(token in adapter for token in ['RepeatedQuest WHERE LevelMin > ? OR LevelMax < ?','DailyQuest WHERE LevelMin > ? OR LevelMax < ?','SingleQuest WHERE LevelMin > ? OR LevelMax < ?'])),
 ('active and completed list sources preserved', 'SELECT * FROM Progress WHERE PlayerName=?' in adapter and 'SELECT * FROM Complete WHERE PlayerName=?' in adapter),
 ('handler sends sentinel', 'QuestCount = -1' in body and 'OPCODE_SPOILER_INFO' in body),
 ('handler delegates spoiler read', 'ServerLegacyQuestViewQuery().TryGetSpoilerBatches' in body),
 ('handler preserves packet max', 'MAX_QUEST_PACKET_INFO' in body),
 ('handler preserves trailing batch packet', body.count('SummitQuestSend(lpPlayInfo') >= 3),
 ('handler routes packets through packet sender boundary', 'ServerLegacyPacketSender().Send' in handler),
 ('handler has no direct QuestDB', 'SQLConnection' not in handler and 'DATABASEID_QuestDB' not in handler and 'db->Prepare' not in handler),
]
failed=[]
for label,ok in checks:
    print(('PASS: ' if ok else 'FAIL: ')+label)
    if not ok: failed.append(label)
if failed: sys.exit(1)
print('quest spoiler view query: PASS')

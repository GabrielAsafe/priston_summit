#!/usr/bin/env python3
from pathlib import Path
import re
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/quests/v2/QuestCatalog.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestCatalog.cpp').read_text(errors='replace')
gateway=(root/'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.cpp').read_text(errors='replace')
header=(root/'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.h').read_text(errors='replace')
runtime=(root/'SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp').read_text(errors='replace')
checks=[
 ('catalog contract', 'class IQuestCatalog' in contract and 'struct QuestDefinition' in contract),
 ('repeated kind preserved', re.search(r'kRepeatedQuestKind\s*=\s*21\s*;', adapter) is not None),
 ('daily kind preserved', re.search(r'kDailyQuestKind\s*=\s*7\s*;', adapter) is not None),
 ('single kind preserved', re.search(r'kSingleQuestKind\s*=\s*9\s*;', adapter) is not None),
 ('repeated table preserved', 'ReadRows("RepeatedQuest"' in adapter),
 ('daily table preserved', 'ReadRows("DailyQuest"' in adapter),
 ('single table preserved', 'ReadRows("SingleQuest"' in adapter),
 ('unique match preserved', 'if(matches != 1)' in adapter),
 ('party/type select preserved', 'SELECT PartyFlag,QuestType FROM ' in adapter),
 ('gateway has no QuestDB SQL', 'SQLConnection' not in gateway and 'ReadQuestRows' not in gateway and 'DATABASEID_QuestDB' not in gateway),
 ('gateway depends on catalog', 'questCatalog_.TryGetDefinition' in gateway and 'const quests::IQuestCatalog& questCatalog_' in header),
 ('runtime injects catalog', 'ServerLegacyQuestCatalog()' in runtime),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('quest catalog boundary: PASS')

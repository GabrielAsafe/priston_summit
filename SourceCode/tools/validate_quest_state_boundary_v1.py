#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
contract = (root / 'Shared/gameplay/quests/v2/QuestStateQuery.h').read_text()
adapter = (root / 'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestStateQuery.cpp').read_text()
readers = (root / 'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionReaders.cpp').read_text()
gateways = (root / 'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.cpp').read_text()
runtime = (root / 'SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp').read_text()
service = (root / 'Shared/gameplay/quests/v2/QuestService.h').read_text()

checks = [
    ('contract exposes IQuestStateQuery', 'class IQuestStateQuery' in contract),
    ('contract distinguishes unreadable via bool', 'virtual bool TryGetState' in contract),
    ('adapter reads completion through repository', 'repository_.TryIsCompleted' in adapter),
    ('adapter reads progress through repository', 'repository_.TryIsInProgress' in adapter),
    ('progression reader no direct cQuestHandler', 'cQuestHandler' not in readers),
    ('story gateway delegates quest orchestration', 'questService_.EnsureStarted' in gateways),
    ('quest service reads modern state', 'states_.TryGetState' in service),
    ('story gateway no direct completion query', 'QueryCompletedQuest' not in gateways),
    ('story gateway no direct progress query', 'IsQuestInProgress' not in gateways and 'QueryQuestInProgress' not in gateways),
    ('runtime injects server quest query', runtime.count('ServerLegacyQuestStateQuery()') >= 2),
]
failed = [name for name, ok in checks if not ok]
if failed:
    for name in failed:
        print('FAIL:', name)
    sys.exit(1)
for name, _ in checks:
    print('PASS:', name)
print('quest state boundary: PASS')

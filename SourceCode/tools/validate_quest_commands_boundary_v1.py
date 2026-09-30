#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/quests/v2/QuestCommands.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestCommands.cpp').read_text()
gateway=(root/'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.cpp').read_text()
runtime=(root/'SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp').read_text()
service=(root/'Shared/gameplay/quests/v2/QuestService.h').read_text()
checks=[
 ('IQuestCommands contract', 'class IQuestCommands' in contract),
 ('start command', 'virtual bool Start' in contract),
 ('cancel command', 'virtual bool Cancel' in contract),
 ('reward command', 'virtual bool ClaimReward' in contract),
 ('legacy adapter owns StartQuest', 'cQuestHandler.StartQuest' in adapter),
 ('legacy adapter owns CancelQuest', 'cQuestHandler.CancelQuest' in adapter),
 ('legacy adapter owns GetReward', 'cQuestHandler.GetReward' in adapter),
 ('progression has no direct cQuestHandler', 'cQuestHandler' not in gateway),
 ('progression delegates quest orchestration', 'questService_.EnsureStarted' in gateway),
 ('quest service uses command port', 'commands_.Start' in service),
 ('runtime injects commands into service', 'ServerLegacyQuestCommands()' in runtime and 'questService(' in runtime),
]
failed=[n for n,o in checks if not o]
for n,o in checks: print(('PASS: ' if o else 'FAIL: ')+n)
if failed: sys.exit(1)
print('quest commands boundary: PASS')

#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
service=(root/'Shared/gameplay/quests/v2/QuestService.h').read_text(errors='replace')
gateway=(root/'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.cpp').read_text(errors='replace')
runtime=(root/'SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp').read_text(errors='replace')
checks=[
 ('green QuestService', 'class QuestService' in service and 'EnsureStarted' in service),
 ('service reads state', service.count('states_.TryGetState') >= 2),
 ('service short circuits completed', 'if(state.completed)' in service),
 ('service accepts already active', 'if(state.inProgress)' in service),
 ('service invokes command once', 'commands_.Start' in service),
 ('service confirms post state', 'return !state.completed && state.inProgress' in service),
 ('progression delegates orchestration', 'questService_.EnsureStarted' in gateway),
 ('progression has no direct quest state port', 'questStates_' not in gateway),
 ('progression has no direct quest command port', 'questCommands_' not in gateway),
 ('runtime owns service', 'quests::QuestService questService;' in runtime),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('quest service boundary: PASS')

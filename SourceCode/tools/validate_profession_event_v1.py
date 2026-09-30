#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
event=(root/'Shared/gameplay/events/v2/ProfessionProgressedEvent.h').read_text()
service=(root/'Shared/gameplay/professions/v2/ProfessionService.h').read_text()
runtime=(root/'SrcServer/src/systems/gameplay/professions/v2/ProfessionRuntime.cpp').read_text()
checks={
 'event payload': all(x in event for x in ('playerName','ProfessionType','previous','current')),
 'event hub injection':'events::GameplayEventHub* eventHub = 0' in service,
 'publish after mutation':'PublishProgressed(playerName, type, current, next)' in service,
 'no duplicate event':'previous.level == current.level && previous.experience == current.experience' in service,
 'server event hub wired':'&events::ServerGameplayEventHub()' in runtime,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('profession progressed event v1: PASS')

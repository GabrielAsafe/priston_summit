#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/events/v2/MonsterDefeatedEvent.h').read_text()
runtime_h=(root/'SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.h').read_text()
runtime_cpp=(root/'SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.cpp').read_text()
hg=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
checks=[
 ('event carries only entity identity', 'struct MonsterDefeatedEvent' in contract and 'EntityId monster' in contract and 'PlayerId' not in contract),
 ('runtime exposes single-defeat publisher', 'PublishMonsterDefeated' in runtime_h and 'PublishMonsterDefeated' in runtime_cpp),
 ('runtime rejects invalid entity', 'if (!event.monster.Valid())' in runtime_cpp),
 ('hellsgate subscribes to physical defeat event', 'Subscribe<gameplay::events::MonsterDefeatedEvent>' in hg and 'MonsterKilled(event.monster)' in hg),
 ('kill kernel publishes defeat event', 'PublishMonsterDefeated(' in onserver and 'MonsterDefeatedEvent(' in onserver),
 ('kill kernel no direct hellsgate call', 'cHg.MonsterKilled(' not in onserver),
 ('credited kill event remains separate', 'PublishMonsterKilled(' in onserver),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('monster defeated event bridge: PASS')

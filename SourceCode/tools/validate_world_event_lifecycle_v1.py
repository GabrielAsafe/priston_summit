#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/events/v2/WorldEventLifecycleEvents.h').read_text()
runtime_h=(root/'SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.h').read_text()
runtime_cpp=(root/'SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.cpp').read_text()
hg=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
checks=[
 ('started event exists','struct WorldEventStarted' in contract and 'eventKey' in contract),
 ('ended event exists','struct WorldEventEnded' in contract and 'completed' in contract),
 ('runtime publish api','PublishWorldEventStarted' in runtime_h and 'PublishWorldEventEnded' in runtime_h),
 ('runtime publishes through hub','ServerGameplayEventHub().Publish(event)' in runtime_cpp),
 ('hellsgate emits start','PublishWorldEventStarted' in hg and 'WorldEventStarted("hells_gate")' in hg),
 ('hellsgate emits end','PublishWorldEventEnded' in hg and 'WorldEventEnded("hells_gate", bKilled)' in hg),
 ('hellsgate timing preserved','dwTickWaiting = GetCurrentTime();' in hg and 'dwTick = GetCurrentTime();' in hg),
 ('hellsgate spawn preserved','ServerLegacySpawnAdapter().Spawn' in hg),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('world event lifecycle boundary: PASS')

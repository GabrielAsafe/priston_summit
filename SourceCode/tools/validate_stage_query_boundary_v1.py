#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/world/v2/IStageQuery.h').read_text(errors='replace')
stage=(root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyStageAdapter.cpp').read_text(errors='replace')
worldh=(root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyWorldAdapter.h').read_text(errors='replace')
world=(root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyWorldAdapter.cpp').read_text(errors='replace')
checks={
 'stage query contract': all(x in contract for x in ('struct StageSnapshot','class IStageQuery','TryGetStage(MapId','IsReady(MapId','TryGetGroundHeight(MapId')),
 'stage owns legacy geometry': all(x in stage for x in ('StageArea[map.value].lpStage','GetHeight(x, z)')),
 'world depends stage port': 'const IStageQuery& stages_' in worldh and 'stages_.TryGetGroundHeight' in world,
 'world no direct stage area': 'StageArea[' not in world,
 'server composition': 'ServerLegacyStageAdapter()' in world,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('stage query boundary v1: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('stage query boundary v1: PASS')

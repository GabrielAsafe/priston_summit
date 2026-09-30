#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
ids=(root/'Shared/gameplay/core/v2/GameplayIds.h').read_text(errors='replace')
contract=(root/'Shared/gameplay/world/v2/IWorldQuery.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyWorldAdapter.cpp').read_text(errors='replace')
stage=(root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyStageAdapter.cpp').read_text(errors='replace') if (root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyStageAdapter.cpp').exists() else ''
harvest=(root/'SrcServer/src/systems/gameplay/gathering/v2/legacy/LegacyHarvestPlayerQueryAdapter.cpp').read_text(errors='replace')
checks={
 'map zero valid sentinel': 'InvalidValue = 0xFFFFFFFFu' in ids and 'value != InvalidValue' in ids,
 'world query contract': all(x in contract for x in ('class IWorldQuery','TryGetPosition(EntityId','TryGetGroundHeight(MapId','IsSameArea(EntityId')),
 'legacy player position': all(x in adapter for x in ('players_.Resolve(PlayerId(entity.value))','player->Position.Area','WorldPosition(MapId(')),
 'legacy stage height': (all(x in adapter for x in ('StageArea[map.value].lpStage','GetHeight(x, z)')) or ('stages_.TryGetGroundHeight' in adapter and all(x in stage for x in ('StageArea[map.value].lpStage','GetHeight(x, z)')))),
 'harvest consumes world query': 'ServerLegacyWorldAdapter().TryGetPosition' in harvest and 'WorldFixedToGrid' in harvest,
 'harvest no direct position read': 'player->Position.' not in harvest,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('world query boundary v1: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('world query boundary v1: PASS')

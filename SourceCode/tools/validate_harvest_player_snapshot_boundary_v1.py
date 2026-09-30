#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
contract = (root / 'Shared/gameplay/gathering/v2/HarvestPlayerQuery.h').read_text()
adapter_cpp = (root / 'SrcServer/src/systems/gameplay/gathering/v2/legacy/LegacyHarvestPlayerQueryAdapter.cpp').read_text()
mining = (root / 'SrcServer/src/systems/gameplay/mining/v2/MiningInteractionRuntime.cpp').read_text()
gather = (root / 'SrcServer/src/systems/gameplay/gathering/v2/GatheringRuntime.cpp').read_text()
allow = (root / 'tools/architecture/legacy_boundary_allowlist.txt').read_text()
checks = {
    'green harvest player query': 'class IHarvestPlayerQuery' in contract and 'HarvestPlayerSnapshot' in contract and 'PlayerId player' in contract,
    'green contract legacy-clean': all(x not in contract for x in ('rsPLAYINFO', 'smCHAR', 'smd3d', 'onserver.h', 'FLOATNS')),
    'amber adapter owns player resolution': 'rsPLAYINFO* player' in adapter_cpp and 'players_.Resolve(playerId)' in adapter_cpp,
    'amber adapter owns position translation': 'ServerLegacyWorldAdapter().TryGetPosition' in adapter_cpp and all(x in adapter_cpp for x in ('WorldFixedToGrid(position.x)', 'WorldFixedToGrid(position.y)', 'WorldFixedToGrid(position.z)')),
    'mining consumes snapshot': 'ServerLegacyHarvestPlayerQueryAdapter().TryGet(playerId, player)' in mining and 'HarvestPlayerSnapshot player' in mining,
    'gathering consumes snapshot': 'ServerLegacyHarvestPlayerQueryAdapter().TryGet(playerId, player)' in gather and 'HarvestPlayerSnapshot player' in gather,
    'mining green runtime legacy-clean': all(x not in mining for x in ('rsPLAYINFO', 'smd3d.h', 'FLOATNS', 'ServerLegacyOnlinePlayerRegistry')),
    'gathering green runtime legacy-clean': all(x not in gather for x in ('rsPLAYINFO', 'smd3d.h', 'onserver.h', 'FLOATNS', 'ServerLegacyOnlinePlayerRegistry')),
    'harvest allowlist debt removed': all(x not in allow for x in ('MiningInteractionRuntime.h | symbol:rsPLAYINFO', 'GatheringRuntime.h | symbol:rsPLAYINFO', 'MiningInteractionRuntime.cpp | include:smd3d.h', 'MiningInteractionRuntime.cpp | symbol:rsPLAYINFO', 'GatheringRuntime.cpp | include:smd3d.h', 'GatheringRuntime.cpp | include:onserver.h', 'GatheringRuntime.cpp | symbol:rsPLAYINFO')),
}
for k, v in checks.items():
    print(('PASS' if v else 'FAIL') + ': ' + k)
if not all(checks.values()):
    sys.exit(1)
print('harvest player snapshot boundary v1: PASS (legacy player/position translation isolated in AMBER adapter)')

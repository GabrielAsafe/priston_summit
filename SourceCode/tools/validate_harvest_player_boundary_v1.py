#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
mining_h=(root/'SrcServer/src/systems/gameplay/mining/v2/MiningInteractionRuntime.h').read_text()
gather_h=(root/'SrcServer/src/systems/gameplay/gathering/v2/GatheringRuntime.h').read_text()
mining_cpp=(root/'SrcServer/src/systems/gameplay/mining/v2/MiningInteractionRuntime.cpp').read_text()
gather_cpp=(root/'SrcServer/src/systems/gameplay/gathering/v2/GatheringRuntime.cpp').read_text()
server=(root/'SrcServer/src/systems/network/v2/legacy/LegacyModernChatIngressAdapter.inl').read_text()
ui=(root/'SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.cpp').read_text()
ui_legacy=(root/'SrcServer/src/systems/gameplay/covenants/v2/legacy/LegacyCovenantUIIngressAdapter.cpp').read_text()
checks={
 'mining public PlayerId':'HarvestMiningNodeForPlayer(PlayerId player' in mining_h,
 'gathering public PlayerId':'HarvestGatheringNodeForPlayer(PlayerId player' in gather_h,
 'mining header no rsPLAYINFO':'rsPLAYINFO' not in mining_h,
 'gathering header no rsPLAYINFO':'rsPLAYINFO' not in gather_h,
 'mining resolves amber':'ServerLegacyHarvestPlayerQueryAdapter().TryGet(playerId' in mining_cpp,
 'gathering resolves amber':'ServerLegacyHarvestPlayerQueryAdapter().TryGet(playerId' in gather_cpp,
 'server derives authenticated id':'HarvestMiningNodeForPlayer(gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in server and 'HarvestGatheringNodeForPlayer(gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in server,
 'ui derives authenticated id':(('ServerLegacyOnlinePlayerRegistry().IdOf(owner)' in ui) or ('ownerId, node.nodeId' in ui_legacy and 'PlayerId ownerId' in ui_legacy)),
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('harvest player boundary v1: PASS (Mining/Gathering public surfaces use PlayerId; legacy player translation remains isolated)')

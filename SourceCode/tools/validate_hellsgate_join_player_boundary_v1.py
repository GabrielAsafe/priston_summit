#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
header = (root / 'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.h').read_text(errors='replace')
source = (root / 'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text(errors='replace')
router = (root / 'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
onserver = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')

checks = [
    ('public join uses PlayerId', 'RequestJoin(gameplay::PlayerId player)' in header and 'RecvJoinHg(rsPLAYINFO' not in header),
    ('event validates player through modern player adapter', 'ServerLegacyPlayerAdapter().IsOnline(playerId)' in source),
    ('join feedback uses chat delivery', 'ServerLegacyChatDeliveryAdapter().SendToPlayer' in source and 'SendQuickMessage2' not in source),
    ('join stores PlayerId identity', 'vPlayers.push_back(playerId)' in source),
    ('join teleports by PlayerId', 'ServerLegacyTeleportAdapter().Teleport' in source and 'playerId, gameplay::WorldPosition' in source),
    ('packet router dispatches authenticated PlayerId', 'class HellsGateJoinHandler' in router and 'cHg.RequestJoin(player)' in router),
    ('onserver authenticates once before routing', 'RouteServerGameplayPacket(' in onserver and 'ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in onserver),
]
failed = []
for name, ok in checks:
    print(('PASS' if ok else 'FAIL') + ': ' + name)
    if not ok:
        failed.append(name)
if failed:
    sys.exit(1)
print('hellsgate join player boundary: PASS')

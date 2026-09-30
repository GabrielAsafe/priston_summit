#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
contract = (root / 'Shared/gameplay/world/v2/ITeleportService.h').read_text(errors='replace')
adapter = (root / 'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyTeleportAdapter.cpp').read_text(errors='replace')
hells = (root / 'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text(errors='replace')
router = (root / 'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
onserver = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')

checks = {
    'green teleport contract': all(x in contract for x in ('class ITeleportService', 'Teleport(PlayerId', 'WorldPosition')),
    'adapter preserves opcode': all(x in adapter for x in ('OPCODE_WARPFIELD', 'packet.WParam', 'packet.SParam = destination.x', 'packet.LParam = destination.z')),
    'adapter uses packet sender': 'sender_.Send' in adapter and 'ServerLegacyPacketSender()' in adapter,
    'hells gate uses boundary': 'ServerLegacyTeleportAdapter().Teleport' in hells,
    'join receives authenticated id from router': 'class HellsGateJoinHandler' in router and 'cHg.RequestJoin(player)' in router,
    'onserver authenticates id before router': 'RouteServerGameplayPacket(' in onserver and 'ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in onserver,
    'hells gate preserves destination': 'MapId(23u)' in hells and 'WorldGridToFixed(-3624)' in hells and 'WorldGridToFixed(-43172)' in hells,
    'global send warp helper removed': 'void SendWarpField(' not in onserver and 'extern void SendWarpField' not in hells,
}
failed = [name for name, ok in checks.items() if not ok]
if failed:
    print('teleport boundary v1: FAIL: ' + ', '.join(failed), file=sys.stderr)
    sys.exit(1)
print('teleport boundary v1: PASS')

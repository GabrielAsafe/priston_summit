#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
router=(root/'Shared/network/v2/PacketRouter.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
checks={
 'green router contract': all(x in router for x in ('class IProtocolHandler','class PacketRouter','RouteStatus Route(','bool Register(')),
 'covenant handler isolated': all(x in adapter for x in ('CovenantHandler','sizeof(covenants::protocol::CovenantUIActionPacket)','HandleCovenantUIAction(player, *action)')),
 'companion handler isolated': all(x in adapter for x in ('CompanionHandler','sizeof(companions::protocol::CompanionUIActionPacket)','ServerLegacyCompanionUIIngressAdapter().HandleAction(player, *action)')),
 'onserver generic route': 'RouteServerGameplayPacket(' in onserver,
 'onserver no covenant parse': 'reinterpret_cast<const gameplay::covenants::protocol::CovenantUIActionPacket' not in onserver,
 'onserver no companion parse': 'reinterpret_cast<const gameplay::companions::protocol::CompanionUIActionPacket' not in onserver,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('network packet router boundary: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('network packet router boundary: PASS')

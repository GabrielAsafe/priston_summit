#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/network/v2/PacketSender.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp').read_text(errors='replace')
cov=(root/'SrcServer/src/systems/gameplay/covenants/v2/legacy/LegacyCovenantUIIngressAdapter.cpp').read_text(errors='replace')
checks={
 'packet sender contract': all(x in contract for x in ('struct PacketView','enum class SendStatus','class IPacketSender','virtual SendResult Send(PlayerId player')),
 'legacy transport isolated': all(x in adapter for x in ('players_.Resolve(playerId)','SendToLegacySocket(player->lpsmSock, packet)','SendStatus::Disconnected','SendStatus::TransportFailure')),
 'covenant uses packet sender': cov.count('ServerLegacyPacketSender().Send(') >= 2,
 'covenant no direct socket send': 'lpsmSock->Send' not in cov,
 'covenant no smwsock include': 'smwsock.h' not in cov,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('network packet sender boundary: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('network packet sender boundary: PASS')

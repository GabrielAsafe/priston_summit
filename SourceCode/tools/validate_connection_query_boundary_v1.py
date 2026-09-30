#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/network/v2/ConnectionQuery.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/network/v2/legacy/LegacyConnectionQuery.cpp').read_text(errors='replace')
senderh=(root/'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.h').read_text(errors='replace')
sender=(root/'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp').read_text(errors='replace')
checks={
 'connection query contract': all(x in contract for x in ('struct ConnectionSnapshot','class IConnectionQuery','TryGet(PlayerId','IsConnected(PlayerId','IsCurrent(PlayerHandle')),
 'legacy adapter registry': all(x in adapter for x in ('players_.Resolve(playerId)','players_.HandleOf(player)','player->lpsmSock')),
 'sender depends query': 'const IConnectionQuery& connections_' in senderh and 'connections_.IsConnected(playerId)' in sender,
 'server composition': 'ServerLegacyConnectionQuery()' in sender,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('connection query boundary v1: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('connection query boundary v1: PASS')

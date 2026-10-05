#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
client = (root / 'SrcGame/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text(errors='replace')
sender = (root / 'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp').read_text(errors='replace')
router = (root / 'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
server = (root / 'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text(errors='replace')

checks = [
    ('client preserves join opcode', 'sPacket.code = OpCode::OPCODE_OPEN_HG' in client),
    ('client preserves command packet size', 'sPacket.size = sizeof(smTRANS_COMMAND)' in client),
    ('client routes join through command sender', 'ClientLegacyCommandSender().Send' in client and 'PacketView(&sPacket, sPacket.size' in client),
    ('client hells gate has no direct socket send', 'smWsockServer->Send' not in client),
    ('amber sender delegates primary socket through owned helper', 'return SendToLegacySocket(smWsockServer, packet);' in sender),
    ('amber sender owns client socket send', 'socket->Send(' in sender and 'static_cast<int>(packet.size), TRUE' in sender),
    ('server router receives same opcode', 'class HellsGateJoinHandler' in router and 'OpCode::OPCODE_OPEN_HG' in router and 'cHg.RequestJoin(player)' in router),
    ('server join remains PlayerId based', 'void cHELLSGATE::RequestJoin(gameplay::PlayerId playerId)' in server),
]
failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print("hellsgate client command sender boundary: PASS")

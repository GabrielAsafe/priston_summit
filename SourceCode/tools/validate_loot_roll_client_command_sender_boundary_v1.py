#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
client = (root / 'SrcGame/src/systems/gameplay/loot/roll/CRollWindow.cpp').read_text(errors='replace')
sender = (root / 'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp').read_text(errors='replace')
onserver = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
checks = [
    ('roll opcode preserved for both actions', client.count('sPacket.code = OpCode::OPCODE_ROLL') == 2),
    ('roll id payload preserved', client.count('sPacket.WParam = pWindow1->getID()') == 2),
    ('roll action remains one', 'sPacket.LParam = 1' in client),
    ('skip action remains zero', 'sPacket.LParam = 0' in client),
    ('both roll commands route through sender', client.count('ClientLegacyCommandSender().Send') == 2 and client.count('PacketView(&sPacket, sPacket.size') == 2),
    ('roll ui has no direct socket send', 'smWsockServer->Send' not in client),
    ('amber sender preserves important send semantics', 'LegacyClientCommandSender::SendToLegacySocket' in sender and 'socket->Send' in sender and 'static_cast<int>(packet.size), TRUE' in sender),
    ('server still handles roll opcode', 'case OpCode::OPCODE_ROLL:' in onserver and 'ROLLHANDLER->HandlePacket' in onserver),
]
failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('loot roll client command sender boundary: PASS')

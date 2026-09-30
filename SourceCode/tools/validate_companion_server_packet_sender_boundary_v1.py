#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
companion = (root / 'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text()
sender_h = (root / 'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.h').read_text()
sender_cpp = (root / 'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp').read_text()

checks = [
    ('server packet sender contract remains available', 'class LegacyPacketSender final : public IPacketSender' in sender_h),
    ('legacy packet sender remains sole socket-send owner for migrated flow', 'lpsmSock->Send' in sender_cpp),
    ('companion includes packet sender adapter', 'systems/network/v2/legacy/LegacyPacketSender.h' in companion),
    ('companion routes owner packets through helper', 'ServerLegacyPacketSender().Send' in companion and 'network::PacketView(data, size, opcode)' in companion),
    ('companion has no direct socket send', 'lpsmSock->Send' not in companion),
    ('all four previous send sites route through helper', companion.count('SendOwnerPacket(') >= 5),
    ('runtime state preserves packet wire tuple', 'SendOwnerPacket(state.owner, &packet, packet.size, packet.code)' in companion),
    ('full player info preserves packet wire tuple', 'SendOwnerPacket(owner, &info, info.size, info.code)' in companion),
    ('ui state preserves packet wire tuple', 'SendOwnerPacket(owner, &packet, packet.size, packet.code)' in companion),
]

failed = [label for label, ok in checks if not ok]
for label, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + label)
if failed:
    sys.exit(1)
print('companion server packet sender boundary: PASS')

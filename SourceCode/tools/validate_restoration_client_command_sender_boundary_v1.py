#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
client = (root / 'SrcGame/src/systems/commerce/restoration/cRestaureWindow.cpp').read_text(errors='replace')
sender = (root / 'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp').read_text(errors='replace')
checks = [
    ('restoration list opcode preserved', 'smTransCommand.code = OpCode::OPCODE_GET_RESTAURE' in client),
    ('restore item opcode preserved', 'smTransCommand.code = OpCode::OPCODE_RESTAURE_ITEM' in client),
    ('restore item identity preserved', 'smTransCommand.WParam = Item->psItem.sItemInfo.ItemHeader.Head' in client and 'smTransCommand.LParam = Item->psItem.sItemInfo.ItemHeader.dwChkSum' in client),
    ('both restoration commands route through sender', client.count('ClientLegacyCommandSender().Send') == 2 and client.count('PacketView(&smTransCommand, smTransCommand.size') == 2),
    ('restoration ui has no direct socket send', 'smWsockServer->Send' not in client),
    ('amber sender preserves important send semantics', 'smWsockServer->Send' in sender and 'static_cast<int>(packet.size), TRUE' in sender),
]
failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('restoration client command sender boundary: PASS')

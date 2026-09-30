#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
client = (root / 'SrcGame/src/systems/commerce/premium/HaPremiumItem.cpp').read_text(errors='replace')
sender = (root / 'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp').read_text(errors='replace')
onserver = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
checks = [
    ('delete premium opcode preserved', 'Packet.code = OpCode::DELETE_ITEM_PREMIUM' in client),
    ('premium item kind payload preserved', 'Packet.LParam = chaPremiumitem.UpKeepItem[i].TGAImageNumber' in client),
    ('availability guard preserved through sender', 'sender.Available()' in client),
    ('premium command routes through sender', 'sender.Send(gameplay::network::PacketView' in client),
    ('premium feature has no direct socket send', 'smWsockServer->Send' not in client),
    ('amber sender preserves socket availability semantics', 'return smWsockServer != 0' in sender),
    ('amber sender preserves important send semantics', 'static_cast<int>(packet.size), TRUE' in sender),
    ('server still handles premium delete opcode', 'case DELETE_ITEM_PREMIUM:' in onserver),
]
failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('premium client command sender boundary: PASS')

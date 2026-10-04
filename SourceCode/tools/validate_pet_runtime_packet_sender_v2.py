#!/usr/bin/env python3
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
cpp = (ROOT / 'SrcServer/src/systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.cpp').read_text(errors='ignore')
hdr = (ROOT / 'SrcServer/src/systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.h').read_text(errors='ignore')
base = (ROOT / 'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start = base.find('"SrcServer/src/systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.cpp"')
frag = base[start:start+260]
checks = [
    ('adapter consumes IPacketSender', 'network::IPacketSender& packets_' in hdr),
    ('server composition injects legacy packet sender', 'network::legacy::ServerLegacyPacketSender()' in cpp),
    ('initial pet sync uses PacketView', 'packets_.Send(' in cpp and 'network::PacketView(&command' in cpp),
    ('direct player socket send retired', 'player->lpsmSock->Send' not in cpp),
    ('socket budget retired', '"lpsmSock": 0' in frag),
]
for name, ok in checks:
    print(('PASS' if ok else 'FAIL') + ': ' + name)
if not all(ok for _, ok in checks):
    raise SystemExit(1)
print(f'pet runtime packet sender v2: PASS ({len(checks)}/{len(checks)})')

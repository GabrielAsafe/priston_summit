#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
cpp=(ROOT/'SrcServer/src/systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.cpp').read_text(errors='ignore')
hdr=(ROOT/'SrcServer/src/systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.h').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"SrcServer/src/systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.cpp"')
frag=base[start:start+260]
checks=[
 ('chat adapter consumes IPacketSender','network::IPacketSender& packets_' in hdr),
 ('composition injects legacy packet sender','network::legacy::ServerLegacyPacketSender()' in cpp),
 ('single delivery uses packet sender','const network::SendResult sent = packets_.Send(' in cpp),
 ('broadcast uses player ids','PlayerId(static_cast<std::uint32_t>(legacyPlayer->dwObjectSerial))' in cpp),
 ('direct chat socket sends retired','lpsmSock->Send' not in cpp),
 ('socket budget retired','"lpsmSock": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'chat delivery packet sender v2: PASS ({len(checks)}/{len(checks)})')

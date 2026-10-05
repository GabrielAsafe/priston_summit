#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
src=(ROOT/'Shared/persistence/character_record/record_legacy_server.inl').read_text(errors='ignore')
packet_h=(ROOT/'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.h').read_text(errors='ignore')
packet_cpp=(ROOT/'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
start=base.find('"Shared/persistence/character_record/record_legacy_server.inl"')
frag=base[start:start+250]
checks=[
 ('packet sender included','systems/network/v2/legacy/LegacyPacketSender.h' in src),
 ('session-socket helper uses packet sender','ServerLegacyPacketSender().SendToLegacySocket(' in src),
 ('record transfer preserves explicit socket','SummitRecordSocketSend(lpsmSock, &TransRecord, TransRecord.size)' in src),
 ('record helper does not require PlayerId','ServerLegacyOnlinePlayerRegistry().IdOf(recipient)' not in src and 'SummitRecordPlayerSend(' not in src),
 ('legacy packet sender exposes transitional socket send','SendToLegacySocket(smWINSOCK* socket' in packet_h),
 ('player-id send delegates to socket send','return SendToLegacySocket(player->lpsmSock, packet);' in packet_cpp),
 ('socket send preserves queued legacy transport','socket->Send(' in packet_cpp and ', size, TRUE)' in packet_cpp),
 ('generic direct socket sends retired',src.count('lpsmSock->Send')==0),
 ('no lpPlayInfo direct sends remain','lpPlayInfo->lpsmSock->Send' not in src),
 ('budget fully retired','"lpsmSock": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'character record session socket delivery v2: PASS ({len(checks)}/{len(checks)})')

#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
hg=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
checks=[
 ('packet sender used for participant fanout','ServerLegacyPacketSender().Send' in hg and 'PacketView packet' in hg),
 ('participant fanout uses PlayerId','PlayerId(static_cast<std::uint32_t>(player.dwObjectSerial))' in hg or 'ServerLegacyPacketSender().Send(playerId, packet)' in hg),
 ('leave arena uses packet sender','OPCODE_LEAVE_HG' in hg and ('ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in hg or 'SendLeaveArena(gameplay::PlayerId playerId)' in hg)),
 ('no direct socket send','->lpsmSock->Send' not in hg),
 ('update opcode preserved','OPCODE_UPDATE_HG' in hg),
 ('open opcode preserved','OPCODE_OPEN_HG' in hg),
 ('leave opcode preserved','OPCODE_LEAVE_HG' in hg),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('hellsgate packet delivery boundary: PASS')

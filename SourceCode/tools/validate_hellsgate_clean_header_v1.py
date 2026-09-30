#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
h=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.h').read_text()
cpp=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
checks=[
 ('header has no rsPLAYINFO','rsPLAYINFO' not in h),
 ('header has no smCHAR','smCHAR' not in h),
 ('reward helper uses PlayerId','SendReward(gameplay::PlayerId player)' in h),
 ('leave helper uses PlayerId','SendLeaveArena(gameplay::PlayerId player)' in h),
 ('end flow uses ids','SendReward(playerId)' in cpp and 'SendLeaveArena(playerId)' in cpp),
 ('reward resolves legacy only internally','ServerLegacyOnlinePlayerRegistry().Resolve(playerId)' in cpp or 'ServerLegacyPlayerAdapter().TryGet(playerId, player)' in cpp),
 ('leave does not resolve pointer','SendLeaveArena(gameplay::PlayerId playerId)' in cpp and 'ServerLegacyPacketSender().Send' in cpp),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('hellsgate clean header boundary: PASS')

#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
h=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.h').read_text()
cpp=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
checks=[
 ('header pointer free','rsPLAYINFO' not in h and 'smCHAR' not in h),
 ('implementation pointer free','rsPLAYINFO' not in cpp and 'smCHAR *' not in cpp and 'smCHAR*' not in cpp),
 ('join online check uses player query','ServerLegacyPlayerAdapter().IsOnline(playerId)' in cpp),
 ('no online registry include','LegacyOnlinePlayerRegistry.h' not in cpp),
 ('reward uses modern snapshot','ServerLegacyPlayerAdapter().TryGet(playerId, player)' in cpp),
 ('spawn uses spawn service','ServerLegacySpawnAdapter().Spawn' in cpp),
 ('teleport uses teleport service','ServerLegacyTeleportAdapter().Teleport' in cpp),
 ('chat uses delivery service','ServerLegacyChatDeliveryAdapter().SendToPlayer' in cpp),
 ('packets use packet sender','ServerLegacyPacketSender().Send' in cpp),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('hellsgate pointer-free runtime: PASS')

#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
h=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.h').read_text()
cpp=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
checks=[
 ('participant storage uses PlayerId','std::vector<gameplay::PlayerId> vPlayers' in h),
 ('legacy participant struct removed','sHGPLAYER' not in h and 'sHGPLAYER' not in cpp),
 ('join stores authenticated id','vPlayers.push_back(playerId)' in cpp),
 ('end resolves by PlayerId','ServerLegacyOnlinePlayerRegistry().Resolve(playerId)' in cpp or ('SendReward(playerId)' in cpp and 'SendLeaveArena(playerId)' in cpp)),
 ('fanout sends by PlayerId','ServerLegacyPacketSender().Send(playerId, packet)' in cpp),
 ('legacy FindUserFromCode removed','FindUserFromCode' not in cpp),
 ('capacity unchanged','MAX_HG_PLAYER 8' in h and 'vPlayers.size() >= MAX_HG_PLAYER' in cpp),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('hellsgate participant ids boundary: PASS')

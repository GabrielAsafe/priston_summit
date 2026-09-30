#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
gh=(root/'SrcServer/src/systems/gameplay/events/v2/legacy/LegacyWorldEventRewardGateway.h').read_text()
gc=(root/'SrcServer/src/systems/gameplay/events/v2/legacy/LegacyWorldEventRewardGateway.cpp').read_text()
hg=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
checks=[
 ('gateway reuses green reward contract','public rewards::IRewardGateway' in gh and 'ServerWorldEventRewardService' in gh),
 ('gateway item only','type == rewards::RewardType::Item' in gc and 'reward.amount == 1' in gc),
 ('legacy job context preserved','spJobCode = player->smCharInfo.JOB_CODE' in gc and 'spJobCode = 0' in gc),
 ('legacy command item preserved','CreateCommandItem(player, player, contentKey)' in gc),
 ('gateway explicitly not retry safe','return false;' in gc and 'IsRetrySafe' in gc),
 ('hellsgate uses reward service','ServerWorldEventRewardService().Grant' in hg),
 ('hellsgate reward key preserved','reward.key = "CX101"' in hg and 'reward.amount = 1' in hg),
 ('hellsgate no direct create command item','CreateCommandItem' not in hg and 'spJobCode' not in hg),
 ('hellsgate resolves modern player snapshot','ServerLegacyPlayerAdapter().TryGet(playerId, player)' in hg),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('world event reward boundary: PASS')

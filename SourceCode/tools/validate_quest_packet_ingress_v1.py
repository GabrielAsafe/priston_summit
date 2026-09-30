#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
protocol=(root/'Shared/gameplay/quests/v2/QuestProtocol.h').read_text(errors='replace')
router=(root/'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
checks=[
 ('wire packet is feature local', 'struct QuestCommandPacket' in protocol and 'static_assert(sizeof(QuestCommandPacket) == 24u' in protocol),
 ('start handler registered', 'QuestStartHandler' in router and 'OPCODE_START_QUEST' in router and 'router.Register(questStart)' in router),
 ('cancel handler registered', 'QuestCancelHandler' in router and 'OPCODE_CANCEL_QUEST' in router and 'router.Register(questCancel)' in router),
 ('reward handler registered', 'QuestRewardHandler' in router and 'OPCODE_GET_REWARD' in router and 'router.Register(questReward)' in router),
 ('router uses quest command port', router.count('ServerLegacyQuestCommands()') >= 3),
 ('onserver no start quest case', 'case OpCode::OPCODE_START_QUEST' not in onserver),
 ('onserver no cancel quest case', 'case OpCode::OPCODE_CANCEL_QUEST' not in onserver),
 ('onserver no reward quest case', 'case OpCode::OPCODE_GET_REWARD' not in onserver),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('quest packet ingress: PASS')

#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
router=(root/'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestQueryIngressAdapter.cpp').read_text(errors='replace')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
checks=[
 ('spoiler handler registered', 'QuestSpoilerHandler' in router and 'OPCODE_SPOILER_INFO' in router and 'router.Register(questSpoiler)' in router),
 ('quest info handler registered', 'QuestInfoHandler' in router and 'OPCODE_QUEST_INFO' in router and 'router.Register(questInfo)' in router),
 ('router uses query ingress adapter', router.count('ServerLegacyQuestQueryIngressAdapter()') >= 2),
 ('adapter preserves spoiler call', 'cQuestHandler.GetSpoilerInfo(resolved, spoilerNumber)' in adapter),
 ('adapter preserves quest info call', 'cQuestHandler.GetQuestInfo(resolved, questId, questKind, spoilerNumber)' in adapter),
 ('onserver no spoiler case', 'case OpCode::OPCODE_SPOILER_INFO' not in onserver),
 ('onserver no quest info case', 'case OpCode::OPCODE_QUEST_INFO' not in onserver),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('quest query ingress: PASS')

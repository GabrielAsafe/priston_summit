#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
combat=(root/'Shared/gameplay/combat/Svr_Damge_legacy_server.inl').read_text(errors='replace')
checks=[
 ('combat no quest handler include', 'systems/gameplay/quests/server/cQuestHandler.h' not in combat),
 ('combat no cQuestHandler use', 'cQuestHandler' not in combat),
 ('combat uses quest state query', 'ServerLegacyQuestStateQuery().TryGetState' in combat),
 ('rank5 quest id preserved', 'QuestId(QUESTID_T5_QUEST_ARENA)' in combat),
 ('completion gate preserved', '!rank5QuestState.completed' in combat),
 ('rejection trace preserved', 'reject-rank5-quest' in combat),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('combat quest state boundary: PASS')

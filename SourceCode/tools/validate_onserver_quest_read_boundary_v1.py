#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
text=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_bytes().decode('latin1')
checks=[
 ('onserver no direct completed query','cQuestHandler.QueryCompletedQuest' not in text),
 ('onserver uses quest repository','ServerLegacyQuestRepository().TryIsCompleted' in text),
 ('rank5 quest id preserved','gameplay::QuestId(QUESTID_T5_QUEST_ARENA)' in text),
 ('rank5 bool preserved','rank5QuestComplete' in text),
 ('db failure anti-escalation comment preserved','On DB failure preserve the server-known rank instead of accepting an escalation.' in text),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: raise SystemExit(1)
print('onserver quest read boundary: PASS')

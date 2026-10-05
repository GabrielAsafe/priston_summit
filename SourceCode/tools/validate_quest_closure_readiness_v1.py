#!/usr/bin/env python3
from pathlib import Path
import csv, json, sys
ROOT=Path(__file__).resolve().parents[1]
Q=ROOT/'Content/canonical/gameplay_reference/quests/priston_tale_quests.pt-BR.json'
B=ROOT/'Content/canonical/gameplay_reference/quests/runtime_bindings.json'
T=ROOT/'Content/canonical/gameplay_reference/quests/runtime_binding_tasks.tsv'
SQL=ROOT/'Database/exports/questdb_runtime_snapshot_v1.sql'
DOC=ROOT/'docs/architecture/QUEST_CLOSURE_DATA_CONTRACT_V1.md'
SNAP=ROOT/'tools/quest_port/validate_questdb_runtime_snapshot_v1.py'
BUILD=ROOT/'tools/quest_port/build_quest_binding_tasks.py'

def fail(m): print('quest closure readiness v1: FAIL: '+m,file=sys.stderr); raise SystemExit(1)
q=json.loads(Q.read_text(encoding='utf-8'))
allq=[x for k in ('progression_quests','daily_quests','aline_daily_quests','weekly_quests') for x in q.get(k,[])]
bind=json.loads(B.read_text(encoding='utf-8')).get('bindings',[])
verified=[x for x in bind if x.get('supported') is True]
if len(allq)!=88: fail(f'canonical quest count {len(allq)} != 88')
if len(verified)!=13: fail(f'verified binding count {len(verified)} != 13')
if not T.is_file(): fail('missing runtime_binding_tasks.tsv')
with T.open(encoding='utf-8',newline='') as f: tasks=list(csv.DictReader(f,delimiter='\t'))
if len(tasks)!=75: fail(f'unresolved task count {len(tasks)} != 75')
ids={x['id'] for x in allq}; taskids={x['guide_id'] for x in tasks}; verifiedids={x['guide_id'] for x in verified}
if taskids & verifiedids: fail('verified bindings leaked into unresolved task queue')
if taskids | verifiedids != ids: fail('verified + unresolved IDs do not cover all canonical quests')
for p in (SQL,DOC,SNAP,BUILD):
    if not p.is_file(): fail(f'missing closure artifact: {p.relative_to(ROOT)}')
sql=SQL.read_text(encoding='utf-8',errors='replace')
for table in ('RepeatedQuest','DailyQuest','SingleQuest','QuestMonsters','Rewards','ItemRewards'):
    if f'FROM {table}' not in sql: fail(f'export contract missing table {table}')
doc=DOC.read_text(encoding='utf-8')
for token in ('QUEST_SOURCE_READINESS=PASS','QUEST_RUNTIME_CLOSURE=BLOCKED_MISSING_QUESTDB_SNAPSHOT','Never invent `QuestID`'):
    if token not in doc: fail(f'data contract missing token {token}')
print('quest closure readiness v1: PASS (canonical=88, runtime_verified=13, unresolved_tasks=75, QuestDB_data=BLOCKED_MISSING_SNAPSHOT)')

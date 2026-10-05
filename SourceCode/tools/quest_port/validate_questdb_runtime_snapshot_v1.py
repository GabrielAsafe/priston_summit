#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, sys
from pathlib import Path

SCHEMA='summit.questdb-runtime-snapshot/v1'
TABLE_KIND={'RepeatedQuest':21,'DailyQuest':7,'SingleQuest':9}
REQ_DEF=('table','quest_id','quest_name','quest_title','level_min','level_max','party_flag','quest_desc','quest_type')
REQ_MON=('quest_id','quest_kind','monster_name','quantity','quest_type','quest_map')
REQ_REW=('quest_id','quest_kind','gold','exp','quest_type','coin','tcoin')
REQ_ITEM=('quest_id','quest_kind','item_code','item_count','quest_type')

def fail(msg):
    print('questdb runtime snapshot v1: FAIL: '+msg,file=sys.stderr); raise SystemExit(1)

def require_rows(doc,key,fields):
    rows=doc.get(key)
    if not isinstance(rows,list): fail(f'{key} must be an array')
    for i,row in enumerate(rows):
        if not isinstance(row,dict): fail(f'{key}[{i}] must be an object')
        miss=[f for f in fields if f not in row]
        if miss: fail(f'{key}[{i}] missing fields: {miss}')
    return rows

def main():
    ap=argparse.ArgumentParser(description='Validate an exported production/runtime QuestDB snapshot without inventing bindings.')
    ap.add_argument('snapshot',type=Path)
    args=ap.parse_args()
    if not args.snapshot.is_file(): fail(f'missing snapshot: {args.snapshot}')
    doc=json.loads(args.snapshot.read_text(encoding='utf-8'))
    if doc.get('schema') != SCHEMA: fail(f'expected schema {SCHEMA}')
    defs=require_rows(doc,'definitions',REQ_DEF)
    monsters=require_rows(doc,'quest_monsters',REQ_MON)
    rewards=require_rows(doc,'rewards',REQ_REW)
    items=require_rows(doc,'item_rewards',REQ_ITEM)
    if not defs: fail('definitions is empty; production QuestDB export is required')
    ids=[]; keys=set()
    for i,row in enumerate(defs):
        table=row['table']
        if table not in TABLE_KIND: fail(f'definitions[{i}] invalid table {table!r}')
        for f in ('quest_id','level_min','level_max','party_flag','quest_type'):
            if not isinstance(row[f],int): fail(f'definitions[{i}].{f} must be int')
        if row['quest_id'] <= 0: fail(f'definitions[{i}].quest_id must be positive')
        if row['level_min'] > row['level_max']: fail(f'definitions[{i}] invalid level range')
        key=(table,row['quest_id'],row['quest_type'])
        if key in keys: fail(f'duplicate definition key {key}')
        keys.add(key); ids.append(row['quest_id'])
    # LegacyQuestCatalog queries QuestID across all three definition tables and accepts exactly one row.
    dup_ids=sorted({qid for qid in ids if ids.count(qid)>1})
    if dup_ids: fail(f'QuestID is not globally unique across definition rows: {dup_ids[:20]}')
    defined={(row['quest_id'],TABLE_KIND[row['table']],row['quest_type']) for row in defs}
    for key,rows in (('quest_monsters',monsters),('rewards',rewards),('item_rewards',items)):
        for i,row in enumerate(rows):
            ref=(row['quest_id'],row['quest_kind'],row['quest_type'])
            if ref not in defined:
                fail(f'{key}[{i}] references missing definition {ref}')
    meta=doc.get('source')
    if not isinstance(meta,dict) or not str(meta.get('exported_at','')).strip() or not str(meta.get('database','')).strip():
        fail('source.database and source.exported_at are required for provenance')
    print('questdb runtime snapshot v1: PASS '
          f'(definitions={len(defs)}, objectives={len(monsters)}, rewards={len(rewards)}, item_rewards={len(items)})')

if __name__=='__main__': main()

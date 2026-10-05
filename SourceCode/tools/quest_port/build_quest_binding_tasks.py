#!/usr/bin/env python3
from __future__ import annotations
import argparse, csv, io, json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
QUESTS = ROOT / 'Content/canonical/gameplay_reference/quests/priston_tale_quests.pt-BR.json'
BINDINGS = ROOT / 'Content/canonical/gameplay_reference/quests/runtime_bindings.json'
OUT = ROOT / 'Content/canonical/gameplay_reference/quests/runtime_binding_tasks.tsv'
SECTIONS = ('progression_quests','daily_quests','aline_daily_quests','weekly_quests')


def rows():
    qdoc=json.loads(QUESTS.read_text(encoding='utf-8'))
    bdoc=json.loads(BINDINGS.read_text(encoding='utf-8'))
    bindings={x['guide_id']:x for x in bdoc.get('bindings',[])}
    out=[]
    for section in SECTIONS:
        for q in qdoc.get(section,[]):
            gid=q['id']; b=bindings.get(gid)
            if b and b.get('supported') is True:
                continue
            state='UNVERIFIED_BINDING' if b else 'MISSING_RUNTIME_BINDING'
            runtime_hint=';'.join(str(x) for x in (b or {}).get('runtime_ids',[]))
            if not runtime_hint:
                runtime_hint=';'.join((b or {}).get('runtime_tags',[]))
            qtype=q.get('type','')
            source=q.get('source','')
            evidence=(
                'prove exact QuestDB definition row (table/QuestID/QuestType/level/party) plus objectives/rewards; '
                'or prove an existing non-QuestDB handler/tag is semantically equivalent; do not infer IDs from title/level'
            )
            out.append([gid,q.get('name',''),section,qtype,state,runtime_hint,evidence,source])
    return out


def render():
    s=io.StringIO(newline='')
    w=csv.writer(s,delimiter='\t',lineterminator='\n')
    w.writerow(['guide_id','name','section','canonical_type','status','existing_runtime_hint','required_evidence','canonical_source'])
    w.writerows(rows())
    return s.getvalue()


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--write',action='store_true')
    args=ap.parse_args()
    content=render()
    if args.write:
        OUT.write_text(content,encoding='utf-8')
    elif not OUT.is_file() or OUT.read_text(encoding='utf-8') != content:
        raise SystemExit('quest binding task file is stale; run build_quest_binding_tasks.py --write')
    count=len(rows())
    if count != 75:
        raise SystemExit(f'expected 75 unresolved canonical quest mappings, found {count}')
    print(f'quest binding tasks: PASS (unresolved={count})')

if __name__=='__main__': main()

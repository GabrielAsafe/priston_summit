#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,re
from pathlib import Path

def norm(s): return '/'.join(x for x in str(s).replace('\\','/').split('/') if x not in ('','.'))
def parse_tree(path:Path):
    raw=path.read_text(encoding='utf-8',errors='replace').replace('\xa0',' ')
    rows=[]
    for ln in raw.splitlines():
        if ln.strip()=='.': continue
        pos=max(ln.rfind('├── '),ln.rfind('└── '))
        if pos<0: continue
        prefix=ln[:pos]; name=ln[pos+4:].strip()
        depth=len(prefix)//4
        rows.append((depth,name))
    files=[]; stack=[]
    for i,(depth,name) in enumerate(rows):
        next_depth=rows[i+1][0] if i+1<len(rows) else -1
        stack=stack[:depth]
        is_dir=next_depth>depth
        if is_dir:
            if len(stack)==depth: stack.append(name)
        else:
            files.append(norm('/'.join(stack+[name])))
    return files

def candidates(logical):
    n=norm(logical); stem=re.sub(r'\.(ini|inx)$','',n,flags=re.I)
    return [stem+'.inx',stem+'.ini',n]

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--canonical',type=Path,required=True);ap.add_argument('--server-tree',type=Path,required=True);ap.add_argument('--server-root-prefix',default='char');ap.add_argument('--client-tree',type=Path);ap.add_argument('--report',type=Path,required=True);ap.add_argument('--patch-model-json',action='store_true');a=ap.parse_args()
    server={norm((a.server_root_prefix.rstrip('/')+'/' if a.server_root_prefix else '')+x).casefold():norm((a.server_root_prefix.rstrip('/')+'/' if a.server_root_prefix else '')+x) for x in parse_tree(a.server_tree)}
    client={x.casefold():x for x in parse_tree(a.client_tree)} if a.client_tree else {}
    modelroot=a.canonical/'model_configs'; rows=[]
    for p in sorted((modelroot/'models').glob('*.json')):
        d=json.loads(p.read_text(encoding='utf-8')); logical=d['logical_config']
        sc=next((server[x.casefold()] for x in candidates(logical) if x.casefold() in server),None)
        cc=next((client[x.casefold()] for x in candidates(logical) if x.casefold() in client),None)
        d['inventory_presence']={'server':{'present':bool(sc),'path':sc or ''},'client':{'present':bool(cc),'path':cc or ''}}
        if a.patch_model_json:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
        rows.append({'id':d['id'],'logical_config':logical,'server_present':bool(sc),'server_path':sc or '', 'client_present':bool(cc),'client_path':cc or '', 'consumers':d['consumers']})
    summary={'models':len(rows),'server_present':sum(r['server_present'] for r in rows),'client_present':sum(r['client_present'] for r in rows),'missing_server':sum(not r['server_present'] for r in rows),'missing_client':sum(not r['client_present'] for r in rows),'missing_both':sum((not r['server_present'] and not r['client_present']) for r in rows)}
    a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps({'summary':summary,'models':rows},ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(summary))
if __name__=='__main__':main()

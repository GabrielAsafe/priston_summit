#!/usr/bin/env python3
from __future__ import annotations
import argparse, importlib.util, json, re
from pathlib import Path

def norm(s): return '/'.join(x for x in str(s).replace('\\','/').split('/') if x and x!='.')
def load_parser(root:Path):
    p=root/'app'/'pt_inx_motion_core_v2.py'
    if not p.exists(): p=root/'pt_inx_motion_core_v2.py'
    spec=importlib.util.spec_from_file_location('ptinx',p); mod=importlib.util.module_from_spec(spec); spec.loader.exec_module(mod); return mod

def idx(root): return {norm(p.relative_to(root).as_posix()).casefold():p for p in root.rglob('*') if p.is_file()}
def resolve(index, logical):
    n=norm(logical); stem=re.sub(r'(?i)\.(ini|inx)$','',n)
    for q in (stem+'.inx',n,stem+'.ini'):
        if q.casefold() in index:return index[q.casefold()]
    return None

def add_path(out, root, value):
    if not value:return
    n=norm(value); p=root/n
    if p.is_file():out.add(norm(p.relative_to(root).as_posix()));return
    # INX stores paths relative to the config folder in many files.
    return

def main():
    ap=argparse.ArgumentParser(description='Build a referenced server char asset manifest from canonical model configs and real INX files.')
    ap.add_argument('--canonical',type=Path,required=True);ap.add_argument('--server-char',type=Path,required=True);ap.add_argument('--pt-asset-tool-root',type=Path,required=True);ap.add_argument('--mode',choices=['runtime-core','render-safe'],default='render-safe');ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
    root=a.server_char.resolve(); index=idx(root); parser=load_parser(a.pt_asset_tool_root.resolve())
    configs=[]; required=set(); unresolved=[]
    model_index=json.loads((a.canonical/'model_configs/index.json').read_text(encoding='utf8'))
    for entry in model_index['models']:
        doc=json.loads((a.canonical/'model_configs'/entry['json_path']).read_text(encoding='utf8'))
        logical=doc['logical_config']; p=resolve(index,logical)
        if not p: unresolved.append({'id':doc['id'],'logical_config':logical});continue
        rel=norm(p.relative_to(root).as_posix());required.add(rel)
        row={'id':doc['id'],'logical_config':logical,'config':rel,'dependencies':[]}
        if p.suffix.lower()=='.inx':
            parsed=parser.parse_inx(p); data=p.read_bytes()
            # first cstrings and parser-known linked motion config
            vals=[]
            for off in (0,64,128): vals.append(data[off:off+64].split(b'\0',1)[0].decode('latin1','replace'))
            vals.append(parsed.get('link_file',''))
            # Prefer exact paths; if not found, resolve against config parent.
            for v in vals:
                if not v:continue
                vn=norm(v); candidates=[vn, norm(str(Path(rel).parent/vn))]
                hit=next((index[c.casefold()] for c in candidates if c.casefold() in index),None)
                if hit:
                    rr=norm(hit.relative_to(root).as_posix());required.add(rr);row['dependencies'].append(rr)
        if a.mode=='render-safe':
            # Current server can invoke ReadTextures(); retain the whole referenced asset directory.
            for child in p.parent.iterdir():
                if child.is_file():required.add(norm(child.relative_to(root).as_posix()))
        configs.append(row)
    report={'schema':'summit.server-char-manifest/v1','mode':a.mode,'server_char':str(root),'referenced_model_configs':len(configs)+len(unresolved),'resolved_configs':len(configs),'unresolved_configs':unresolved,'required_file_count':len(required),'files':sorted(required),'models':configs,
      'safety_note':'render-safe retains every file in referenced model directories because the current server may call ReadTextures(). runtime-core is only safe after the model loader is made headless.'}
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf8');print(json.dumps({k:report[k] for k in ('mode','referenced_model_configs','resolved_configs','required_file_count')},ensure_ascii=False))
if __name__=='__main__':main()

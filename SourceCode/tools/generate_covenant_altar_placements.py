#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,re
from pathlib import Path

def slug(s): return re.sub(r'[^a-z0-9]+','_',s.lower()).strip('_')
def norm(s): return '/'.join(x for x in s.replace('\\','/').split('/') if x and x!='.')

def stage_to_spc(stage):
    p=norm(stage); name=p.split('/')[-1]
    stem=re.sub(r'(?i)\.ase$','',name)
    return f'GameServer/Field/{stem}.ASE.spc'

def make_manifest(maps_json,out):
    maps=json.loads(Path(maps_json).read_text(encoding='utf8'))['maps']
    rows=[]
    for m in maps:
        starts=m.get('start_points') or []
        c=starts[0] if starts else m.get('center') or {'x':0,'z':0}
        rows.append({
            'map_id':m['id'],'legacy_field_code':m['legacy_field_code'],'stage':m['stage'],
            'target_spc':stage_to_spc(m['stage']),
            'candidate_world':{'x':c.get('x'), 'y':None, 'z':c.get('z')},
            'rotation':{'x':0,'y':0,'z':0},'validated':False,
            'note':'Set y and validate x/y/z in-game with the debug overlay before applying.'
        })
    d={'schema':'summit.covenant-altar-placement-plan/v1','npc_id':'npc.covenant_altar','coordinate_space':'world','maps':rows}
    Path(out).parent.mkdir(parents=True,exist_ok=True);Path(out).write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8')

def apply_manifest(manifest,canonical):
    root=Path(canonical); d=json.loads(Path(manifest).read_text(encoding='utf8'))
    if d.get('coordinate_space')!='world': raise ValueError('manifest must use world coordinates')
    idxp=root/'fixed_npcs/index.json'; idx=json.loads(idxp.read_text(encoding='utf8'))
    entries=idx.get('stages') or idx.get('documents') or idx.get('files')
    if entries is None: raise ValueError('unknown fixed_npcs index layout')
    by_logical={norm((e.get('logical_path') or '')).casefold():e for e in entries}
    applied=[]
    for row in d['maps']:
        if not row.get('validated'): continue
        pos=row['candidate_world']
        if any(pos.get(k) is None for k in ('x','y','z')): raise ValueError(f"{row['map_id']}: validated row missing x/y/z")
        logical=norm(row['target_spc']); existing=by_logical.get(logical.casefold())
        if existing:
            rel=existing.get('json_path') or existing.get('path')
            p=root/'fixed_npcs'/rel
            doc=json.loads(p.read_text(encoding='utf8'))
        else:
            fname=logical.split('/')[-1]+'.json'
            p=root/'fixed_npcs/npcs/GameServer/Field'/fname
            p.parent.mkdir(parents=True,exist_ok=True)
            doc={'schema':'summit.fixed-npcs/v1','id':'fixed_npcs.'+slug(logical),'logical_path':logical,'npcs':[],'provenance':{'source':'generated:covenant-altars'}}
            e={'id':doc['id'],'logical_path':logical,'json_path':norm(p.relative_to(root/'fixed_npcs').as_posix()),'npc_count':0}
            entries.append(e);by_logical[logical.casefold()]=e;existing=e
        # replace existing altar if re-applying
        doc['npcs']=[n for n in doc.get('npcs',[]) if n.get('npc_id')!='npc.covenant_altar']
        used={int(n.get('slot',-1)) for n in doc['npcs']};slot=next(i for i in range(100) if i not in used)
        rot=row.get('rotation') or {}
        doc['npcs'].append({'slot':slot,'npc_id':'npc.covenant_altar',
          'x':round(float(pos['x'])*256),'y':round(float(pos['y'])*256),'z':round(float(pos['z'])*256),
          'ax':int(rot.get('x',0)),'ay':int(rot.get('y',0)),'az':int(rot.get('z',0)),'state':0})
        p.write_text(json.dumps(doc,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
        existing['npc_count']=len(doc['npcs']);applied.append(row['map_id'])
    idxp.write_text(json.dumps(idx,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(json.dumps({'applied':len(applied),'maps':applied},ensure_ascii=False))

def main():
    ap=argparse.ArgumentParser();sp=ap.add_subparsers(dest='cmd',required=True)
    m=sp.add_parser('make-plan');m.add_argument('maps_json');m.add_argument('output')
    a=sp.add_parser('apply');a.add_argument('manifest');a.add_argument('canonical')
    ns=ap.parse_args()
    if ns.cmd=='make-plan': make_manifest(ns.maps_json,ns.output)
    else: apply_manifest(ns.manifest,ns.canonical)
if __name__=='__main__':main()

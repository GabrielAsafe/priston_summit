#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, struct
from collections import Counter
from pathlib import Path

HEADER=b'SMD Model data Ver 0.62'; SH,SI,SO=556,40,2236; SV,SF,ST,SR,SP,SS,SM=24,36,32,20,16,16,64
O_PHYS,O_NV,O_NF,O_NT,O_NAME,O_NR,O_NP,O_NS=16,84,88,92,172,684,688,692
F_TEX=32; T_U,T_V,T_NEXT=0,12,28

def i32(b,o): return struct.unpack_from('<i',b,o)[0]
def u32(b,o): return struct.unpack_from('<I',b,o)[0]
def cs(b): return b.split(b'\0',1)[0].decode('latin1','replace')
def sha(b): return hashlib.sha256(b).hexdigest()

def parse(data:bytes):
    if len(data)<SH or not data.startswith(HEADER): raise ValueError('not Model0.62')
    n=i32(data,24); mat=i32(data,28); mat_point=i32(data,32); first_obj=i32(data,36)
    infos=[]
    for i in range(n):
        o=SH+i*SI; infos.append({'index':i,'name':cs(data[o:o+32]),'length':i32(data,o+32),'start':i32(data,o+36)})
    objs={}
    for inf in infos:
        raw=data[inf['start']:inf['start']+SO]; name=cs(raw[O_NAME:O_NAME+32]);
        if name!=inf['name']: raise ValueError(f'node mismatch {inf["name"]}/{name}')
        nv,nf,nt,nr,np,ns=[i32(raw,o) for o in (O_NV,O_NF,O_NT,O_NR,O_NP,O_NS)]; phys=u32(raw,O_PHYS)!=0
        vo=inf['start']+SO; fo=vo+nv*SV; to=fo+nf*SF
        verts=[struct.unpack_from('<6i',data,vo+i*SV) for i in range(nv)]
        texbase=u32(raw,12)
        faces=[]
        for fi in range(nf):
            off=fo+fi*SF; a,b,c,mid=struct.unpack_from('<4H',data,off); ptr=u32(data,off+F_TEX); delta=ptr-texbase
            if delta<0 or delta%ST: raise ValueError(f'{name} bad tex ptr face {fi}')
            ti=delta//ST
            if not 0<=ti<nt: raise ValueError(f'{name} tex ptr OOB face {fi}')
            uv=[]; chain=[]; cur=ti; seen=set()
            while True:
                if cur in seen: raise ValueError('texlink cycle')
                seen.add(cur); oo=to+cur*ST; us=struct.unpack_from('<3f',data,oo+T_U); vs=struct.unpack_from('<3f',data,oo+T_V)
                chain.append(tuple((round(us[j],7),round(vs[j],7)) for j in range(3)))
                nxt=u32(data,oo+T_NEXT)
                if not nxt: break
                dd=nxt-texbase
                if dd<0 or dd%ST: raise ValueError('next tex ptr invalid')
                cur=dd//ST
            faces.append({'verts':(a,b,c),'material':mid,'uv_layers':chain})
        end=to+nt*ST+nr*SR+np*SP+ns*SS+nr*SM+(nv*32 if phys else 0)
        if end!=inf['start']+inf['length']: raise ValueError(f'{name} block length')
        objs[name]={'name':name,'vertices':verts,'faces':faces,'nv':nv,'nf':nf,'nt':nt,'nr':nr,'np':np,'ns':ns,'phys':phys,'block':data[inf['start']:inf['start']+inf['length']]}
    return {'n':n,'mat_count':mat,'mat_region':data[mat_point:first_obj],'infos':infos,'objects':objs}

def face_signature(obj):
    # Orientation-independent triangle signature while retaining UV bound to vertex.
    out=[]
    for f in obj['faces']:
        layer=f['uv_layers'][0] if f['uv_layers'] else ((0,0),(0,0),(0,0))
        corners=sorted((f['verts'][i],layer[i]) for i in range(3))
        out.append((f['material'],tuple(corners),len(f['uv_layers'])))
    return Counter(out)

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--source',required=True); ap.add_argument('--output',required=True); ap.add_argument('--object',required=True); ap.add_argument('--report'); a=ap.parse_args()
    sb=Path(a.source).read_bytes(); ob=Path(a.output).read_bytes(); s=parse(sb); o=parse(ob)
    errors=[]
    if s['n']!=o['n']: errors.append('OBJECT_COUNT')
    if s['mat_count']!=o['mat_count']: errors.append('MATERIAL_COUNT')
    if s['mat_region']!=o['mat_region']: errors.append('MATERIAL_REGION_BYTES')
    if set(s['objects'])!=set(o['objects']): errors.append('OBJECT_NAMES')
    if a.object not in s['objects'] or a.object not in o['objects']: errors.append('TARGET_MISSING')
    non_target=[]
    for name in set(s['objects']) & set(o['objects']):
        if name==a.object: continue
        if s['objects'][name]['block']!=o['objects'][name]['block']: non_target.append(name)
    if non_target: errors.append('NON_TARGET_BLOCK_CHANGED')
    details={}
    if a.object in s['objects'] and a.object in o['objects']:
        x,y=s['objects'][a.object],o['objects'][a.object]
        pos_s=[v[:3] for v in x['vertices']]; pos_o=[v[:3] for v in y['vertices']]
        details={'source_counts':[x['nv'],x['nf'],x['nt']],'output_counts':[y['nv'],y['nf'],y['nt']], 'positions_exact':pos_s==pos_o,'face_uv_material_semantic_exact':face_signature(x)==face_signature(y),'source_animation_keys':[x['nr'],x['np'],x['ns']],'output_animation_keys':[y['nr'],y['np'],y['ns']],'source_physique':x['phys'],'output_physique':y['phys']}
        if not details['positions_exact']: errors.append('POSITIONS')
        if not details['face_uv_material_semantic_exact']: errors.append('FACE_UV_MATERIAL')
        if details['source_animation_keys']!=details['output_animation_keys']: errors.append('ANIMATION_KEY_COUNTS')
        if x['phys']!=y['phys']: errors.append('PHYSIQUE_MODE')
    result={'tool':'PT_SMD_STATIC_SEMANTIC_COMPARE_V1','source':a.source,'output':a.output,'source_sha256':sha(sb),'output_sha256':sha(ob),'target':a.object,'details':details,'non_target_changed':non_target,'errors':errors,'byte_identical':sb==ob,'client_modified':False,'check':'PASS' if not errors else 'FAIL'}
    if a.report: Path(a.report).write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    print('PT_SMD_STATIC_SEMANTIC_COMPARE_V1'); print(f"target={a.object}"); print(f"byte_identical={result['byte_identical']}"); print(f"errors={errors}"); print('client_modified=False'); print(f"static_semantic_compare_check={result['check']}")
    if errors: raise SystemExit(2)
if __name__=='__main__': main()

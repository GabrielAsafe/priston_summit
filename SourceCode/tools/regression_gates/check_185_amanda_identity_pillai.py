#!/usr/bin/env python3
from pathlib import Path
import json, sys
root=Path(__file__).resolve().parents[2]
errors=[]
def resource(cid):
  for p in (root/'Content/canonical/text/resources').rglob('*.json'):
    try:d=json.loads(p.read_text(encoding='utf-8'))
    except Exception: continue
    if d.get('id')==cid:return d
  errors.append(f'missing resource {cid}'); return {}
for cid in ('npc.ji_woo','npc.amanda'):
 d=resource(cid); f=d.get('semantic',{}).get('fields',{})
 if f.get('class')!=1014: errors.append(f'{cid} class={f.get("class")}')
 if str(f.get('model_file','')).casefold()!=r'char\npc\ji_woo\jiwoo.ini'.casefold(): errors.append(f'{cid} model={f.get("model_file")}')
for cid in ('text.jiwoo','text.amanda'):
 d=resource(cid); names=[e.get('text') for e in d.get('semantic',{}).get('entries',[]) if e.get('kind')=='name']
 if names!=['Instrutora Amanda']: errors.append(f'{cid} name={names}')
server=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
for token in ('pilai.ase.spc','GameServer/NPC/ji_woo.npc','260757','amandaPresent'):
 if token not in server: errors.append(f'missing Pillai overlay token: {token}')
if errors:
 print('\n'.join('ERROR: '+x for x in errors)); sys.exit(1)
print('PATCH 185 gate: OK')

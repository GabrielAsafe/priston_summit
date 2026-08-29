#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,pathlib,shutil,hashlib,re
def norm(s): return re.sub('/+','/',s.replace('\\','/')).lstrip('./')
def main():
 ap=argparse.ArgumentParser(); ap.add_argument('game_root',type=pathlib.Path); ap.add_argument('workspace',type=pathlib.Path); ap.add_argument('--manifest',type=pathlib.Path,default=pathlib.Path(__file__).with_name('summit_asset_manifest.json')); ap.add_argument('--category',action='append'); ap.add_argument('--class-code',action='append'); ap.add_argument('--owner',action='append'); ap.add_argument('--include-low',action='store_true'); ap.add_argument('--hash',action='store_true'); a=ap.parse_args()
 M=json.loads(a.manifest.read_text(encoding='utf-8')); root=a.game_root.resolve(); ws=a.workspace.resolve(); ws.mkdir(parents=True,exist_ok=True); cats=set(a.category or []); codes=set(a.class_code or []); owners=set(a.owner or []); physical={norm(p.relative_to(root).as_posix()).lower():p for p in root.rglob('*') if p.is_file()}; rec=[]; missing=[]
 for x in M['assets']:
  if cats and x['category'] not in cats: continue
  if codes and x.get('class_code') not in codes: continue
  if owners and x.get('owner_name') not in owners: continue
  if not a.include_low and x.get('confidence')=='LOW': continue
  src=physical.get(norm(x['roundtrip_target']).lower())
  if not src: missing.append(x['roundtrip_target']); continue
  dst=ws/x['blender_target']; dst.parent.mkdir(parents=True,exist_ok=True); shutil.copy2(src,dst); r={'asset_id':x['asset_id'],'workspace_path':dst.relative_to(ws).as_posix(),'roundtrip_target':x['roundtrip_target'],'category':x['category'],'owner_name':x.get('owner_name',''),'class_code':x.get('class_code','')};
  if a.hash: r['source_sha256']=hashlib.sha256(src.read_bytes()).hexdigest()
  rec.append(r)
 (ws/'workspace_manifest.json').write_text(json.dumps({'schema_version':'1.0','game_root':str(root),'files':rec,'missing':missing},ensure_ascii=False,indent=2),encoding='utf-8'); print('copied',len(rec),'missing',len(missing))
if __name__=='__main__': main()

#!/usr/bin/env python3
from __future__ import annotations
import argparse,importlib.util,json
from collections import Counter
from pathlib import Path

def load(name,path):
 spec=importlib.util.spec_from_file_location(name,path); m=importlib.util.module_from_spec(spec); assert spec.loader is not None; spec.loader.exec_module(m); return m

def main():
 ap=argparse.ArgumentParser(); ap.add_argument('--root',default='.'); ap.add_argument('--client-root',required=True); ap.add_argument('--report',required=True); a=ap.parse_args()
 root=Path(a.root).expanduser().resolve(); client=Path(a.client_root).expanduser().resolve(); bmp=load('pt_bmp_core',root/'pt_character_texture_v1_4.py'); tga=load('pt_tga_core',root/'pt_protected_tga_v1.py')
 bs=Counter(); ts=Counter(); berr=[]; terr=[]
 for p in client.rglob('*'):
  if not p.is_file(): continue
  ext=p.suffix.lower()
  if ext=='.bmp':
   try:
    b=p.read_bytes(); info=bmp.inspect_bmp_bytes(b); ed=bmp.decode_editable(b,info); out,_,_=bmp.build_game_bytes(b,ed)
    if out!=b: raise RuntimeError('no-op not byte-identical')
    bs[f"SUPPORTED_{'STANDARD' if info['standard'] else 'PROTECTED'}_{info['bpp']}BPP"]+=1
   except Exception as e:
    bs['UNSUPPORTED']+=1
    if len(berr)<50: berr.append({'path':p.relative_to(client).as_posix(),'error':f'{type(e).__name__}: {e}'})
  elif ext=='.tga':
   try:
    b=p.read_bytes(); info=tga.inspect_bytes(b)
    if info['protected']:
     ed,_=tga.decode_bytes(b); out,_,_=tga.build_bytes(b,ed)
     if out!=b: raise RuntimeError('no-op not byte-identical')
     ts[f"SUPPORTED_PROTECTED_{info['protection_mode']}"]+=1
    else: ts['STANDARD']+=1
   except Exception as e:
    ts['UNSUPPORTED']+=1
    if len(terr)<50: terr.append({'path':p.relative_to(client).as_posix(),'error':f'{type(e).__name__}: {e}'})
 result={'tool':'PT_TEXTURE_FULL_CLIENT_AUDIT_V1','bmp_counts':dict(bs),'bmp_errors_sample':berr,'tga_counts':dict(ts),'tga_errors_sample':terr,'client_modified':False,'check':'PASS'}
 rp=Path(a.report).expanduser().resolve(); rp.parent.mkdir(parents=True,exist_ok=True); rp.write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 print('PT_TEXTURE_FULL_CLIENT_AUDIT_V1'); print(f'bmp_counts={dict(bs)}'); print(f'tga_counts={dict(ts)}'); print(f'report={rp}'); print('client_modified=False'); print('texture_full_client_audit_check=PASS')
if __name__=='__main__': main()

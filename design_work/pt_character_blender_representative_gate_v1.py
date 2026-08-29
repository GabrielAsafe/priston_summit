#!/usr/bin/env python3
from __future__ import annotations
import argparse,csv,json,os,shlex,subprocess
from pathlib import Path

TOOL='PT_CHARACTER_BLENDER_REPRESENTATIVE_GATE_V1'
DEFAULT_INX=[
 'char/monster/Monbagon/Monbagon-C.inx',
 'char/monster/Monshadow/Monshadow-01.inx',
 'char/npc/SN-002/sn-002.inx',
]

def norm(x): return str(x or '').replace('\\','/').lower()
def index(root): return {norm(p.relative_to(root).as_posix()):p for p in root.rglob('*') if p.is_file()}
def blender_cmd():
 raw=os.environ.get('PT_BLENDER_COMMAND'); return shlex.split(raw) if raw else ['flatpak','run','--filesystem=home','org.blender.Blender']
def run(cmd,log):
 print('COMMAND='+' '.join(shlex.quote(str(x)) for x in cmd),flush=True); cp=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT); Path(log).parent.mkdir(parents=True,exist_ok=True); Path(log).write_text(cp.stdout,encoding='utf-8')
 if cp.returncode: raise RuntimeError(f'command failed rc={cp.returncode}; log={log}')
 return cp.stdout

def main():
 ap=argparse.ArgumentParser(); ap.add_argument('--root',default='.'); ap.add_argument('--client-root',required=True); ap.add_argument('--pairing',default='logs/asset_family_refine_v2/inx_physical_pairing.csv'); ap.add_argument('--inx',action='append'); ap.add_argument('--out-dir',default='logs/character_blender_representative_gate_v1'); a=ap.parse_args()
 root=Path(a.root).expanduser().resolve(); client=Path(a.client_root).expanduser().resolve(); pair=Path(a.pairing); pair=pair if pair.is_absolute() else root/pair; out=Path(a.out_dir); out=out if out.is_absolute() else root/out; out=out.resolve(); out.mkdir(parents=True,exist_ok=True)
 rows=list(csv.DictReader(pair.open(encoding='utf-8',newline=''))); byinx={norm(r['inx']):r for r in rows}; idx=index(client); selected=a.inx or DEFAULT_INX; summary=[]
 for logical in selected:
  r=byinx.get(norm(logical));
  if not r: raise RuntimeError(f'pairing missing {logical}')
  ip=idx.get(norm(r['inx'])); mp=idx.get(norm(r['model_smd'])); sp=idx.get(norm(r['motion_smb']))
  if not all((ip,mp,sp)): raise FileNotFoundError(f'physical chain missing for {logical}: inx={ip} model={mp} smb={sp}')
  tag=ip.stem; wd=out/tag; wd.mkdir(parents=True,exist_ok=True)
  run(['python3',str(root/'pt_character_structural_gate_v2.py'),'--smd',str(mp),'--smb',str(sp),'--report',str(wd/'structural.json')],wd/'00_structural.log')
  lods=[]
  for lod in ('high','default','low'):
   blend=wd/f'{tag}_{lod}.blend'
   run(blender_cmd()+['--background','--python-exit-code','1','--python',str(root/'history_py/pt_smd_blender_import_v3_static_armature.py'),'--','--smd',str(mp),'--skeleton',str(sp),'--model-info',str(ip),'--lod',lod,'--client-root',str(client),'--out',str(blend),'--no-pack'],wd/f'01_import_{lod}.log')
   lods.append({'lod':lod,'blend':str(blend),'check':'PASS'})
  summary.append({'inx':ip.relative_to(client).as_posix(),'model':mp.relative_to(client).as_posix(),'smb':sp.relative_to(client).as_posix(),'lods':lods,'check':'PASS'})
 result={'tool':TOOL,'representatives':summary,'client_modified':False,'check':'PASS'}; (out/'summary.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 print(TOOL); print(f'representatives={len(summary)}'); print('client_modified=False'); print('character_blender_representative_gate_check=PASS')
if __name__=='__main__':
 try: main()
 except Exception as exc: print(f'character_blender_representative_gate_check=FAIL: {type(exc).__name__}: {exc}'); raise

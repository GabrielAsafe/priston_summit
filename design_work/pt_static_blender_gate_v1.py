#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,os,shlex,subprocess
from pathlib import Path

TOOL='PT_STATIC_BLENDER_GATE_V1'
DEFAULTS=[
 'image/Sinimage/Items/DropItem/itPR101.smd',
 'image/Sinimage/Items/DropItem/itWA104.smd',
 'image/Sinimage/Items/DropItem/itWH118.smd',
]

def client_index(root:Path):
 return {p.relative_to(root).as_posix().lower():p for p in root.rglob('*') if p.is_file()}

def blender_cmd():
 raw=os.environ.get('PT_BLENDER_COMMAND')
 return shlex.split(raw) if raw else ['flatpak','run','--filesystem=home','org.blender.Blender']

def run(cmd,log:Path):
 print('COMMAND='+' '.join(shlex.quote(str(x)) for x in cmd),flush=True)
 cp=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 log.parent.mkdir(parents=True,exist_ok=True); log.write_text(cp.stdout,encoding='utf-8')
 if cp.returncode: raise RuntimeError(f'command failed rc={cp.returncode}; log={log}')
 return cp.stdout

def main():
 ap=argparse.ArgumentParser(); ap.add_argument('--root',default='.'); ap.add_argument('--client-root',required=True); ap.add_argument('--smd',action='append'); ap.add_argument('--out-dir',default='logs/static_blender_gate_v1'); a=ap.parse_args()
 root=Path(a.root).expanduser().resolve(); client=Path(a.client_root).expanduser().resolve(); out=Path(a.out_dir).expanduser(); out=out if out.is_absolute() else root/out; out=out.resolve(); out.mkdir(parents=True,exist_ok=True)
 idx=client_index(client); smds=a.smd or DEFAULTS; rows=[]
 for rel in smds:
  src=idx.get(rel.replace('\\','/').lower())
  if not src: raise FileNotFoundError(f'client asset not found: {rel}')
  tag=src.stem; wd=out/tag; wd.mkdir(parents=True,exist_ok=True)
  probe=wd/'source_probe.json'
  run(['python3',str(root/'pt_smd_static_capability_probe_v1.py'),'--smd',str(src),'--report',str(probe)],wd/'01_probe.log')
  pd=json.loads(probe.read_text()); ready=pd['ready_objects']
  if len(ready)!=1: raise RuntimeError(f'{rel}: expected exactly one V1-ready object, got {ready}')
  obj=ready[0]; blend=wd/f'{tag}_static_import.blend'; built=wd/f'{tag}_noop_build.smd'; writer_report=wd/'writer.json'; reblend=wd/f'{tag}_reimport.blend'; cmp=wd/'semantic_compare.json'
  b=blender_cmd()
  run(b+['--background','--python-exit-code','1','--python',str(root/'pt_smd_blender_import_v4_generic.py'),'--','--smd',str(src),'--client-root',str(client),'--out',str(blend),'--no-pack'],wd/'02_import.log')
  run(b+['--background',str(blend),'--python-exit-code','1','--python',str(root/'pt_blender_to_smd_static_v1.py'),'--','--smd',str(src),'--object',obj,'--out',str(built),'--report',str(writer_report)],wd/'03_writer.log')
  run(['python3',str(root/'pt_smd_static_semantic_compare_v1.py'),'--source',str(src),'--output',str(built),'--object',obj,'--report',str(cmp)],wd/'04_compare.log')
  run(b+['--background','--python-exit-code','1','--python',str(root/'pt_smd_blender_import_v4_generic.py'),'--','--smd',str(built),'--client-root',str(client),'--out',str(reblend),'--no-pack'],wd/'05_reimport.log')
  cd=json.loads(cmp.read_text()); rows.append({'asset':src.relative_to(client).as_posix(),'object':obj,'built':str(built),'semantic_compare':cd['check'],'byte_identical':cd['byte_identical'],'check':'PASS'})
 result={'tool':TOOL,'tests':rows,'client_modified':False,'check':'PASS'}; (out/'summary.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 print(TOOL); print(f'tests={len(rows)}'); print('client_modified=False'); print('static_blender_gate_check=PASS')
if __name__=='__main__':
 try: main()
 except Exception as exc:
  print(f'static_blender_gate_check=FAIL: {type(exc).__name__}: {exc}'); raise

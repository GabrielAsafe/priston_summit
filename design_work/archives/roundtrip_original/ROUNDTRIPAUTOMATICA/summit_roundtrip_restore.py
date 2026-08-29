#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,pathlib,shutil,datetime
def main():
 ap=argparse.ArgumentParser(); ap.add_argument('workspace',type=pathlib.Path); ap.add_argument('--game-root',type=pathlib.Path); ap.add_argument('--apply',action='store_true'); a=ap.parse_args(); ws=a.workspace.resolve(); M=json.loads((ws/'workspace_manifest.json').read_text(encoding='utf-8')); root=(a.game_root or pathlib.Path(M['game_root'])).resolve(); backup=ws/('_roundtrip_backup_'+datetime.datetime.now().strftime('%Y%m%d_%H%M%S'))
 for r in M['files']:
  src=ws/r['workspace_path']; dst=root/r['roundtrip_target'];
  if not src.exists(): continue
  print('WRITE' if a.apply else 'DRY-RUN',src,'->',dst)
  if a.apply:
   dst.parent.mkdir(parents=True,exist_ok=True)
   if dst.exists(): b=backup/dst.relative_to(root); b.parent.mkdir(parents=True,exist_ok=True); shutil.copy2(dst,b)
   shutil.copy2(src,dst)
 if not a.apply: print('No files changed. Use --apply after review.')
 elif backup.exists(): print('Backup:',backup)
if __name__=='__main__': main()

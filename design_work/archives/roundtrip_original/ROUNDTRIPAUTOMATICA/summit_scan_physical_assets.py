#!/usr/bin/env python3
from __future__ import annotations
import argparse,csv,hashlib,json,pathlib,re
ASSET_EXTS={'.smd','.smb','.ase','.inx','.ini','.inf','.tga','.bmp','.jpg','.jpeg','.png','.dds','.sta','.stg','.wav','.ogg','.mp3','.lua','.cam','.dat','.bin'}
def norm(s): return re.sub('/+','/',s.replace('\\','/')).lstrip('./')
def aid(p): return 'AST-'+hashlib.sha1(p.lower().encode()).hexdigest()[:12].upper()
def fallback(p):
 q=p.lower(); e=pathlib.PurePosixPath(p).suffix.lower(); c='Unclassified'; sub='Unknown'; rep='Unknown'; owner=''; code=''; conf='LOW'; act='REVIEW'
 if q.startswith('char/tmabcd/'):
  c='Characters'; sub='Character Asset'; rep='Character Asset'; conf='MEDIUM'; act='RESOLVE_CONFIG'
  m=re.fullmatch(r'(m?)([a-e])(\d{3})\.ini',p.rsplit('/',1)[-1].lower())
  if m:
   mor,l,_=m.groups(); mp=({'a':('Knight','mA'),'b':('Atalanta','mB'),'c':('Priestess','mC'),'d':('Magician','mD'),'e':('Shaman','mE')} if mor else {'a':('Mechanician','A'),'b':('Fighter','B'),'c':('Pikeman','C'),'d':('Archer','D'),'e':('Assassin','E')}); owner,code=mp[l]; sub='Body/Armor Config'; rep='Equipped Body'
 elif q.startswith('image/'):
  c='UI'; sub='UI Texture'; rep='UI'; conf='MEDIUM'; act='COPY_TEXTURE'
  if '/items/dropitem/' in q: c='Items'; sub='Drop Item'; rep='World Drop'
  elif '/items/defense/' in q: c='Items'; sub='Defense Icon'; rep='Inventory Icon'
  elif '/effect/' in q: c='Effects'; sub='Effect Texture'; rep='Effect Texture'
 elif e in {'.wav','.ogg','.mp3'}: c='Audio'; sub='Sound'; rep='Audio'; conf='MEDIUM'; act='REFERENCE_ONLY'
 elif 'monster' in q: c='Monsters'; sub='Monster Asset'; rep='Monster Asset'; conf='MEDIUM'; act='IMPORT_3D' if e in {'.ase','.smd','.smb'} else 'RESOLVE_CONFIG'
 elif 'weapon' in q or '/item' in q: c='Weapons/Items'; sub='Weapon/Item Asset'; rep='Item'; conf='MEDIUM'; act='IMPORT_3D' if e in {'.ase','.smd','.smb'} else 'RESOLVE_CONFIG'
 elif 'effect' in q or 'particle' in q: c='Effects'; sub='Effect Asset'; rep='Effect Resource'; conf='MEDIUM'; act='IMPORT_3D' if e in {'.ase','.smd','.smb'} else 'COPY_TEXTURE'
 elif e in {'.ase','.smd','.smb'}: c='3D/Config'; sub=e[1:].upper(); rep='3D'; act='IMPORT_3D'
 elif e in {'.tga','.bmp','.jpg','.jpeg','.png','.dds','.sta','.stg'}: c='Textures'; sub='Texture'; rep='Texture'; act='COPY_TEXTURE'
 return dict(category=c,subcategory=sub,representation=rep,owner_name=owner,class_code=code,confidence=conf,import_action=act)
def main():
 ap=argparse.ArgumentParser(); ap.add_argument('game_root',type=pathlib.Path); ap.add_argument('--manifest',type=pathlib.Path,default=pathlib.Path(__file__).with_name('summit_asset_manifest.json')); ap.add_argument('--out-prefix',default='Summit_Physical_Asset_Inventory'); ap.add_argument('--hash',action='store_true'); a=ap.parse_args()
 root=a.game_root.resolve(); M=json.loads(a.manifest.read_text(encoding='utf-8')); known={norm(x['roundtrip_target']).lower():x for x in M['assets'] if '%' not in x['roundtrip_target'] and '*' not in x['roundtrip_target']}; rows=[]
 for f in root.rglob('*'):
  if not f.is_file() or f.suffix.lower() not in ASSET_EXTS: continue
  rel=norm(f.relative_to(root).as_posix()); k=rel.lower(); x=known.get(k); meta=({z:x.get(z,'') for z in ['category','subcategory','representation','owner_name','class_code','confidence','action']} if x else fallback(rel)); st=f.stat(); row={'asset_id':x['asset_id'] if x else aid(rel),'relative_path':rel,'physical_path':str(f),'source_referenced':'YES' if x else 'NO','size_bytes':st.st_size,**meta};
  if a.hash: row['sha256']=hashlib.sha256(f.read_bytes()).hexdigest()
  rows.append(row)
 rows.sort(key=lambda z:(z['category'],z.get('owner_name',''),z['relative_path'].lower())); jp=pathlib.Path(a.out_prefix+'.json'); cp=pathlib.Path(a.out_prefix+'.csv'); jp.write_text(json.dumps({'game_root':str(root),'count':len(rows),'assets':rows},ensure_ascii=False,indent=2),encoding='utf-8'); fields=list(rows[0]) if rows else ['asset_id','relative_path'];
 with cp.open('w',newline='',encoding='utf-8') as h:
  w=csv.DictWriter(h,fieldnames=fields); w.writeheader(); w.writerows(rows)
 print('assets',len(rows)); print(cp); print(jp)
if __name__=='__main__': main()

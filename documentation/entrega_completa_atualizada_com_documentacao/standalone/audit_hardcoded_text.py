#!/usr/bin/env python3
import argparse,json,re,pathlib
ap=argparse.ArgumentParser();ap.add_argument('--source',required=True);ap.add_argument('--catalog',required=True);a=ap.parse_args()
d=json.load(open(a.catalog,encoding='utf8'));known={e.get('text','') for e in d.get('entries',[])}
pat=re.compile(r'(?<![A-Za-z0-9_])"((?:\\.|[^"\\]){4,})"')
skip=('systems/localization/locales/','systems/localization/legacy_catalog/','legacy_oracle/','before_')
out=[]
for p in pathlib.Path(a.source).rglob('*'):
 if p.suffix.lower() not in {'.cpp','.h','.inl','.c'} or any(s in str(p).replace('\\','/') for s in skip): continue
 try: txt=p.read_text(encoding='utf8',errors='ignore')
 except: continue
 for n,line in enumerate(txt.splitlines(),1):
  for m in pat.finditer(line):
   s=m.group(1)
   if not re.search(r'[A-Za-zÀ-ÿ]{3}',s) or s in known: continue
   if re.fullmatch(r'[A-Za-z0-9_./\\:%+\-*?=<>\[\](){} ,]+',s) and (' ' not in s or len(s.split())<2): continue
   out.append({'file':str(p),'line':n,'text':s[:240]})
print(json.dumps({'candidates':out,'count':len(out)},ensure_ascii=False,indent=2))

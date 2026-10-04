#!/usr/bin/env python3
import argparse,json,re,pathlib
ap=argparse.ArgumentParser();ap.add_argument('--drops',required=True);ap.add_argument('--items',required=True);a=ap.parse_args()
known=set()
for p in pathlib.Path(a.items).glob('*.items.json'):
 d=json.load(open(p,encoding='utf8'));known.update(x.upper() for x in d.get('codes',[]))
rows=[]
for p in pathlib.Path(a.drops).rglob('*.inf'):
 raw=p.read_bytes().decode('latin1','replace')
 for n,line in enumerate(raw.splitlines(),1):
  if line.lstrip().startswith('//') or '*아이템' not in line: continue
  toks=re.findall(r'\b[A-Za-z][A-Za-z0-9_]{2,15}\b',line)
  # first token after probability may be item codes; exclude Korean directive artifacts by known-code check/report
  for t in toks:
   if any(c.isdigit() for c in t) and t.upper() not in known: rows.append((str(p),n,t))
print(json.dumps({'known_item_codes':len(known),'missing_references':[{'file':p,'line':n,'code':c} for p,n,c in rows]},ensure_ascii=False,indent=2))
raise SystemExit(1 if rows else 0)

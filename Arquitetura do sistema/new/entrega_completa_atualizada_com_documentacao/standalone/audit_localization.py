#!/usr/bin/env python3
import argparse,json,re,pathlib
ap=argparse.ArgumentParser();ap.add_argument('catalog');a=ap.parse_args();d=json.load(open(a.catalog,encoding='utf8'));loc=d.get('locale','');bad=[]
foreign=re.compile(r'[\uac00-\ud7af\u3040-\u30ff\u4e00-\u9fff]')
moji=re.compile(r'(Ã.|Â.|#U[0-9A-Fa-f]{4}|\ufffd)')
for e in d.get('entries',[]):
 t=e.get('text','')
 reasons=[]
 if loc.startswith(('pt','es')) and foreign.search(t): reasons.append('foreign-script')
 if moji.search(t): reasons.append('encoding/mojibake')
 if reasons: bad.append({'key':e.get('key'),'text':t,'reasons':reasons})
print(json.dumps({'locale':loc,'issues':bad,'count':len(bad)},ensure_ascii=False,indent=2));raise SystemExit(1 if bad else 0)

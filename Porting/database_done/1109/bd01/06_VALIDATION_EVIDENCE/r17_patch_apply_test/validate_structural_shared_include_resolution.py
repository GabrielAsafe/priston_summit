#!/usr/bin/env python3
from pathlib import Path
import csv,re,sys
ROOT=Path(__file__).resolve().parent
rows=list(csv.DictReader((ROOT/'STRUCTURAL_SHARED_PROOF.tsv').open(encoding='utf-8'),delimiter='\t'))
INC_RE=re.compile(r'^\s*#\s*include\s*"([^"]+)"',re.M)
CLIENT=[ROOT,ROOT/'Shared',ROOT/'Shared2',ROOT/'SrcGame/src',ROOT/'SrcGame/src/systems/rendering/legacy_3d',ROOT/'SrcGame/src/third_party/lua/legacy_src',ROOT/'SrcGame/src/third_party/lua/legacy_src/Lua',ROOT/'modules',ROOT/'modules/File',ROOT/'modules/MD5',ROOT/'modules/Helper',ROOT/'Dependency/DirectX9/Include']
SERVER=[ROOT,ROOT/'SrcServer/src',ROOT/'SrcGame/src',ROOT/'Shared',ROOT/'Shared/Lua',ROOT/'Shared2',ROOT/'modules',ROOT/'modules/File',ROOT/'Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include']
def resolve(base,name,dirs):
    for p in [base/name]+[d/name for d in dirs]:
        if p.is_file():return p.resolve()
    return None
def rel(p):
    if not p:return ''
    try:return p.relative_to(ROOT).as_posix()
    except:return str(p)
checked=0;both_unresolved=0;errors=[]
for r in rows:
    if r['status']!='token-contiguous':continue
    helper=ROOT/r['path']; names=INC_RE.findall(helper.read_text(encoding='utf-8',errors='ignore'))
    if not names:continue
    matches=[x for x in r.get('matches','').split(';') if x]
    users=[x for x in r.get('structured_users','').split(';') if x]
    for user in users:
        dirs=CLIENT if user.startswith('SrcGame/') else SERVER
        prefix='SrcGame/' if user.startswith('SrcGame/') else 'SrcServer/'
        candidates=[m for m in matches if m.startswith(prefix)] or matches
        if not candidates:continue
        original=ROOT/candidates[0]
        for name in names:
            a=resolve(original.parent,name,dirs); b=resolve(helper.parent,name,dirs);checked+=1
            if not a and not b:both_unresolved+=1;continue
            if rel(a)!=rel(b):errors.append((r['path'],user,candidates[0],name,rel(a),rel(b)))
print('Quoted include contexts checked:',checked)
print('Both unresolved                :',both_unresolved)
print('Resolution changes             :',len(errors))
if errors:
    for e in errors[:100]:print(' -',' | '.join(e))
    sys.exit(1)
print('RESULT                         : PASS')

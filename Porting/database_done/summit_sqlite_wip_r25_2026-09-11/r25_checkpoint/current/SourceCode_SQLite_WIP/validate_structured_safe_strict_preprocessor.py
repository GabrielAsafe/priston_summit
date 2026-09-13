from pathlib import Path
import csv,re,sys
ROOT=Path(__file__).resolve().parent; PRE=None
manifest=list(csv.DictReader(open(ROOT/'ORACLE_PARITY_MANIFEST.tsv',encoding='utf-8'),delimiter='\t'))
safe_rows=[r for r in manifest if r['mode']=='structured-safe']
targets=[r['path'] for r in safe_rows]
oracles={r['path']:ROOT/r['oracle'] for r in safe_rows}
# include resolution dirs
CLIENT_INC=[ROOT,ROOT/'Shared',ROOT/'Shared2',ROOT/'SrcGame/src',ROOT/'SrcGame/src/systems/rendering/legacy_3d',ROOT/'SrcGame/src/third_party/lua/legacy_src',ROOT/'SrcGame/src/third_party/lua/legacy_src/Lua',ROOT/'modules',ROOT/'modules/File',ROOT/'modules/MD5',ROOT/'modules/Helper',ROOT/'Dependency/DirectX9/Include']
SERVER_INC=[ROOT,ROOT/'SrcServer/src',ROOT/'SrcGame/src',ROOT/'Shared',ROOT/'Shared/Lua',ROOT/'Shared2',ROOT/'modules',ROOT/'modules/File',ROOT/'Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include']
inc_line=re.compile(r'^(\s*)#\s*include\s*([<"])([^">]+)[">].*$',re.M)

def resolve(cur,name,q,dirs):
    c=[]
    if q=='"': c.append(cur.parent/name)
    c += [d/name for d in dirs]
    for p in c:
        if p.is_file(): return p.resolve()
    return None

# New structural Shared files are frozen in ACTIVE_SHARED_TOKEN_PROOF.tsv.
approved_new_shared=set()
proof_file=ROOT/'STRUCTURAL_SHARED_PROOF.tsv'
if proof_file.exists():
    with proof_file.open(encoding='utf-8') as f:
        approved_new_shared={r['path'] for r in csv.DictReader(f,delimiter='\t') if r['status']=='token-contiguous'}
def is_new_shared(p):
    try: rel=p.relative_to(ROOT).as_posix()
    except: return False
    return rel in approved_new_shared

def expand_file(p,dirs,stack=()):
    if p in stack: raise RuntimeError('include cycle '+str(p))
    txt=p.read_text(encoding='utf-8',errors='ignore')
    out=[]; pos=0
    for m in inc_line.finditer(txt):
        out.append(txt[pos:m.start()])
        r=resolve(p,m.group(3),m.group(2),dirs)
        if r and is_new_shared(r):
            out.append(expand_file(r,dirs,stack+(p,)))
        else:
            out.append(m.group(0))
        pos=m.end()
    out.append(txt[pos:])
    return ''.join(out)

def canon(s):
    out=[];i=0;n=len(s)
    while i<n:
        c=s[i]
        if c.isspace(): i+=1;continue
        if c=='/' and i+1<n and s[i+1]=='/':
            i+=2
            while i<n and s[i]!='\n':i+=1
            continue
        if c=='/' and i+1<n and s[i+1]=='*':
            i+=2
            while i+1<n and not(s[i]=='*' and s[i+1]=='/'):i+=1
            i=min(n,i+2);continue
        if c in ('"',"'"):
            q=c;j=i+1
            while j<n:
                if s[j]=='\\': j+=2; continue
                if s[j]==q: j+=1;break
                j+=1
            out.append(s[i:j]);i=j;continue
        if c.isalnum() or c=='_':
            j=i+1
            while j<n and (s[j].isalnum() or s[j]=='_'):j+=1
            out.append(s[i:j]);i=j;continue
        out.append(c);i+=1
    return '\x1f'.join(out)
results=[]
for rel in targets:
    cur=ROOT/rel; pre=oracles[rel]
    dirs=CLIENT_INC if rel.startswith('SrcGame/') else SERVER_INC
    try:
        flat=expand_file(cur,dirs)
        ok=canon(flat)==canon(pre.read_text(encoding='utf-8',errors='ignore'))
        results.append((rel,'PASS' if ok else 'FAIL',len(canon(flat)),len(canon(pre.read_text(encoding='utf-8',errors='ignore')))))
    except Exception as e:
        results.append((rel,'ERROR:'+str(e),0,0))
with open(ROOT/'STRUCTURED_SAFE_STRICT_PREPROCESSOR.tsv','w',newline='',encoding='utf-8') as f:
    w=csv.writer(f,delimiter='\t');w.writerow(['path','status','flatten_tokens','pre_tokens']);w.writerows(results)
from collections import Counter
print(Counter(r[1] for r in results))
for r in results:
    if r[1]!='PASS': print(*r)
if any(r[1]!='PASS' for r in results): sys.exit(1)

#!/usr/bin/env python3
from pathlib import Path
import csv,hashlib,re,sys
ROOT=Path(__file__).resolve().parent
PRE=ROOT/'Shared/legacy_oracle'  # oracle paths are taken from ORACLE_PARITY_MANIFEST
BASELINE_PATHS={r['path'] for r in csv.DictReader((ROOT/'PRE_REFACTOR_SOURCE_MANIFEST.tsv').open(encoding='utf-8'),delimiter='\t')}
manifest={r['path']:r for r in csv.DictReader((ROOT/'ORACLE_PARITY_MANIFEST.tsv').open(encoding='utf-8'),delimiter='\t')}
proof_path=ROOT/'STRUCTURAL_MACRO_EQUIVALENCE.tsv'
helper_path=ROOT/'STRUCTURAL_MACRO_HELPERS.tsv'
rows=list(csv.DictReader(proof_path.open(encoding='utf-8'),delimiter='\t')) if proof_path.exists() else []
helpers=list(csv.DictReader(helper_path.open(encoding='utf-8'),delimiter='\t')) if helper_path.exists() else []
INC_RE=re.compile(r'^(\s*)#\s*include\s*([<"])([^">]+)[">].*$',re.M)
CLIENT=[ROOT,ROOT/'Shared',ROOT/'Shared2',ROOT/'SrcGame/src',ROOT/'SrcGame/src/systems/rendering/legacy_3d',ROOT/'SrcGame/src/third_party/lua/legacy_src',ROOT/'SrcGame/src/third_party/lua/legacy_src/Lua',ROOT/'modules',ROOT/'modules/File',ROOT/'modules/MD5',ROOT/'modules/Helper',ROOT/'Dependency/DirectX9/Include']
SERVER=[ROOT,ROOT/'SrcServer/src',ROOT/'SrcGame/src',ROOT/'Shared',ROOT/'Shared/Lua',ROOT/'Shared2',ROOT/'modules',ROOT/'modules/File',ROOT/'Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include']

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def resolve(cur,name,q,dirs):
    cand=[]
    if q=='"': cand.append(cur.parent/name)
    cand += [d/name for d in dirs]
    return next((p.resolve() for p in cand if p.is_file()),None)
def is_new_shared(p):
    try: rel=p.relative_to(ROOT).as_posix()
    except: return False
    # For macro-equivalence proof expand any non-baseline Shared helper. Its exact hash is frozen separately.
    return rel.startswith('Shared/') and rel not in BASELINE_PATHS
def expand(p,dirs,stack=()):
    p=p.resolve()
    if p in stack: raise RuntimeError('include cycle: '+str(p))
    txt=p.read_text(encoding='utf-8',errors='ignore');out=[];pos=0
    for m in INC_RE.finditer(txt):
        out.append(txt[pos:m.start()]); r=resolve(p,m.group(3),m.group(2),dirs)
        if r and is_new_shared(r): out.append(expand(r,dirs,stack+(p,)))
        else: out.append(m.group(0))
        pos=m.end()
    out.append(txt[pos:]);return ''.join(out)
def expand_aliases(text,names):
    names=set(filter(None,names.split(';'))); defs={}; out=[]
    for line in text.splitlines(keepends=True):
        md=re.match(r'^\s*#\s*define\s+([A-Za-z_]\w*)\s+(.+?)\s*(?:\r?\n)?$',line)
        if md and md.group(1) in names:
            defs[md.group(1)]=md.group(2).strip(); continue
        mu=re.match(r'^\s*#\s*undef\s+([A-Za-z_]\w*)\s*(?:\r?\n)?$',line)
        if mu and mu.group(1) in names: continue
        out.append(line)
    missing=names-set(defs)
    if missing: raise RuntimeError('missing alias definitions: '+','.join(sorted(missing)))
    text=''.join(out)
    for name in sorted(names,key=len,reverse=True):
        text=re.sub(r'\b'+re.escape(name)+r'\b',defs[name],text)
    return text

def strip_relocated_defines(text,names):
    names=set(filter(None,names.split(';'))); values={}
    if not names: return text,values
    out=[]; consumed=set(); prefix=''
    for line in text.splitlines(keepends=True):
        md=re.match(r'^\s*#\s*define\s+([A-Za-z_]\w*)\s+(.+?)\s*(?:\r?\n)?$',line)
        if md and md.group(1) in names:
            name=md.group(1)
            if re.search(r'\b'+re.escape(name)+r'\b',prefix):
                raise RuntimeError('relocated macro used before original definition: '+name)
            values[name]=md.group(2).strip();consumed.add(name)
            continue
        out.append(line);prefix+=line
    missing=names-consumed
    if missing: raise RuntimeError('missing relocated macro definitions: '+','.join(sorted(missing)))
    return ''.join(out),values

def canon(s):
    out=[];i=0;n=len(s)
    while i<n:
        c=s[i]
        if c.isspace():i+=1;continue
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
                if s[j]=='\\':j+=2;continue
                if s[j]==q:j+=1;break
                j+=1
            out.append(s[i:j]);i=j;continue
        if c=='#':
            j=i+1
            while j<n and s[j] in ' \t':j+=1
            k=j
            while k<n and (s[k].isalnum() or s[k]=='_'):k+=1
            if s[j:k]=='pragma':
                z=k
                while z<n and s[z] in ' \t':z+=1
                q=z
                while q<n and (s[q].isalnum() or s[q]=='_'):q+=1
                if s[z:q]=='once':
                    while q<n and s[q]!='\n':q+=1
                    i=q;continue
        if c.isalnum() or c=='_':
            j=i+1
            while j<n and (s[j].isalnum() or s[j]=='_'):j+=1
            out.append(s[i:j]);i=j;continue
        out.append(c);i+=1
    # C/C++ concatenates adjacent ordinary string literals during translation.
    # Treat "A" "032" ".ini" as the same token value as "A032.ini"
    # for structural macro proofs. This is language semantics, not a source fix.
    merged=[]
    for tok in out:
        if (merged and len(tok)>=2 and tok[0]=='"' and tok[-1]=='"'
                and len(merged[-1])>=2 and merged[-1][0]=='"' and merged[-1][-1]=='"'):
            merged[-1]=merged[-1][:-1]+tok[1:]
        else:
            merged.append(tok)
    return '\x1f'.join(merged)
errors=[]
for h in helpers:
    p=ROOT/h['path']
    if not p.is_file() or sha(p)!=h['sha256']: errors.append('macro helper hash changed: '+h['path'])
for r in rows:
    rel=r['path']; m=manifest.get(rel)
    if not m or m['mode']!='structured-macro-equivalent': errors.append('manifest mode mismatch: '+rel);continue
    p=ROOT/rel; oracle=ROOT/m['oracle']
    if not p.is_file() or sha(p)!=r['structured_sha256']: errors.append('structured macro target hash changed: '+rel);continue
    if not oracle.is_file() or sha(oracle)!=r['oracle_sha256'] or r['oracle_sha256']!=m['sha256']: errors.append('macro oracle mismatch: '+rel);continue
    dirs=CLIENT if rel.startswith('SrcGame/') else SERVER
    try:
        normalized=expand_aliases(expand(p,dirs),r['macros'])
        oracle_text=oracle.read_text(encoding='utf-8',errors='ignore')
        relocated=r.get('relocated_macros','')
        normalized,current_defs=strip_relocated_defines(normalized,relocated)
        oracle_text,oracle_defs=strip_relocated_defines(oracle_text,relocated)
        if current_defs!=oracle_defs:
            raise RuntimeError('relocated macro values differ: '+repr((current_defs,oracle_defs)))
    except Exception as e: errors.append(rel+': '+str(e));continue
    if canon(normalized)!=canon(oracle_text):
        errors.append('macro-expanded source differs from pre_refactor: '+rel)
print('Macro-equivalent targets:',len(rows))
print('Frozen macro helpers    :',len(helpers))
if errors:
    print('FAIL:',len(errors))
    for e in errors: print(' -',e)
    sys.exit(1)
print('RESULT                  : PASS')

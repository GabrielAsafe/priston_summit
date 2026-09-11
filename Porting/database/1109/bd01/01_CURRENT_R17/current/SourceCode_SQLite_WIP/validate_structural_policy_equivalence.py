#!/usr/bin/env python3
from pathlib import Path
import csv,hashlib,re,sys
ROOT=Path(__file__).resolve().parent
BASELINE_PATHS={r['path'] for r in csv.DictReader((ROOT/'PRE_REFACTOR_SOURCE_MANIFEST.tsv').open(encoding='utf-8'),delimiter='\t')}
manifest={r['path']:r for r in csv.DictReader((ROOT/'ORACLE_PARITY_MANIFEST.tsv').open(encoding='utf-8'),delimiter='\t')}
rows=list(csv.DictReader((ROOT/'STRUCTURAL_POLICY_EQUIVALENCE.tsv').open(encoding='utf-8'),delimiter='\t')) if (ROOT/'STRUCTURAL_POLICY_EQUIVALENCE.tsv').exists() else []
helpers=list(csv.DictReader((ROOT/'STRUCTURAL_POLICY_HELPERS.tsv').open(encoding='utf-8'),delimiter='\t')) if (ROOT/'STRUCTURAL_POLICY_HELPERS.tsv').exists() else []
INC_RE=re.compile(r'^(\s*)#\s*include\s*([<"])([^">]+)[">].*$',re.M)
CLIENT=[ROOT,ROOT/'Shared',ROOT/'Shared2',ROOT/'SrcGame/src',ROOT/'SrcGame/src/systems/rendering/legacy_3d',ROOT/'SrcGame/src/third_party/lua/legacy_src',ROOT/'SrcGame/src/third_party/lua/legacy_src/Lua',ROOT/'modules',ROOT/'modules/File',ROOT/'modules/MD5',ROOT/'modules/Helper',ROOT/'Dependency/DirectX9/Include']
SERVER=[ROOT,ROOT/'SrcServer/src',ROOT/'SrcGame/src',ROOT/'Shared',ROOT/'Shared/Lua',ROOT/'Shared2',ROOT/'modules',ROOT/'modules/File',ROOT/'Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def resolve(cur,name,q,dirs):
    cand=([cur.parent/name] if q=='"' else [])+[d/name for d in dirs]
    return next((p.resolve() for p in cand if p.is_file()),None)
def new_shared(p):
    try:rel=p.relative_to(ROOT).as_posix()
    except:return False
    return rel.startswith('Shared/') and rel not in BASELINE_PATHS
def expand(p,dirs,stack=()):
    p=p.resolve()
    if p in stack:raise RuntimeError('include cycle '+str(p))
    txt=p.read_text(encoding='utf-8',errors='ignore');out=[];pos=0
    for m in INC_RE.finditer(txt):
        out.append(txt[pos:m.start()]);r=resolve(p,m.group(3),m.group(2),dirs)
        if r and new_shared(r):out.append(expand(r,dirs,stack+(p,)))
        else:out.append(m.group(0))
        pos=m.end()
    out.append(txt[pos:]);return ''.join(out)
def tokens(s):
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
            # Preserve directives as normal tokens except pragma once.
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
    return out
def match(t,start,open_t='{',close_t='}'):
    if start>=len(t) or t[start]!=open_t:raise RuntimeError('expected '+open_t)
    d=1;i=start+1
    while i<len(t):
        if t[i]==open_t:d+=1
        elif t[i]==close_t:
            d-=1
            if d==0:return i
        i+=1
    raise RuntimeError('unbalanced '+open_t)
def split_args(seq):
    if not seq:return []
    out=[];cur=[];d=0
    pairs={'(':')','[':']','{':'}','<':'>'}; opens=set(pairs); closes=set(pairs.values())
    for x in seq:
        if x in opens:d+=1
        elif x in closes and d>0:d-=1
        if x==',' and d==0:out.append(cur);cur=[]
        else:cur.append(x)
    out.append(cur);return out
def parse_policy(t,policy):
    try:i=next(i for i in range(len(t)-2) if t[i]=='struct' and t[i+1]==policy and t[i+2]=='{')
    except StopIteration:raise RuntimeError('policy struct not found: '+policy)
    close=match(t,i+2);end=close+1
    if end<len(t) and t[end]==';':end+=1
    body=t[i+3:close]; funcs={}; aliases={};p=0
    while p<len(body):
        if body[p]=='using' and p+3<len(body):
            name=body[p+1]
            if body[p+2]!='=': raise RuntimeError('policy using alias missing =: '+name)
            try: semi=body.index(';',p+3)
            except ValueError: raise RuntimeError('policy using alias missing semicolon: '+name)
            aliases[name]=body[p+3:semi]; p=semi+1; continue
        if body[p]!='static':p+=1;continue
        # find function opening paren, name is token before it
        try:op=body.index('(',p+1)
        except ValueError:break
        name=body[op-1];cp=match(body,op,'(',')')
        params=[]
        for seg in split_args(body[op+1:cp]):
            ids=[x for x in seg if re.match(r'^[A-Za-z_]\w*$',x)]
            if ids:params.append(ids[-1])
        if cp+1>=len(body) or body[cp+1]!='{':raise RuntimeError('policy function body missing: '+name)
        cb=match(body,cp+1); fb=body[cp+2:cb]
        if 'return' in fb:
            ri=fb.index('return')
            try:semi=fb.index(';',ri+1)
            except ValueError:raise RuntimeError('policy return missing semicolon: '+name)
            expr=fb[ri+1:semi]
            funcs[name]=(params,'expr',expr)
        else:
            # Empty/void policy hooks and one-or-more statement adapters are
            # valid structural policies. At a statement call site their body
            # can be inlined exactly (with parameter substitution).
            funcs[name]=(params,'stmt',fb)
        p=cb+1
    return t[:i]+t[end:],funcs,aliases
def inline_aliases(t,policy,aliases):
    out=[];i=0
    while i<len(t):
        if i+3<len(t) and t[i]==policy and t[i+1]==':' and t[i+2]==':' and t[i+3] in aliases:
            out.extend(aliases[t[i+3]]); i+=4; continue
        out.append(t[i]); i+=1
    return out
def inline_calls(t,policy,funcs):
    out=[];i=0
    while i<len(t):
        if i+4<len(t) and t[i]==policy and t[i+1]==':' and t[i+2]==':' and t[i+3] in funcs and t[i+4]=='(':
            cp=match(t,i+4,'(',')'); args=split_args(t[i+5:cp])
            params,kind,expr=funcs[t[i+3]]
            if len(args)!=len(params):raise RuntimeError('argument count mismatch '+t[i+3])
            amap=dict(zip(params,args)); repl=[]
            for x in expr:
                repl.extend(amap[x] if x in amap else [x])
            out.extend(repl)
            i=cp+1
            if kind=='stmt' and i<len(t) and t[i]==';': i+=1
            continue
        out.append(t[i]);i+=1
    if policy in out:raise RuntimeError('policy identifier remains after inline: '+policy)
    return out
errors=[]
for h in helpers:
    p=ROOT/h['path']
    if not p.is_file() or sha(p)!=h['sha256']:errors.append('policy helper hash changed: '+h['path'])
for r in rows:
    rel=r['path'];m=manifest.get(rel);p=ROOT/rel
    if not m or m['mode']!='structured-policy-equivalent':errors.append('manifest mode mismatch: '+rel);continue
    oracle=ROOT/m['oracle']
    if not p.is_file() or sha(p)!=r['structured_sha256']:errors.append('policy target hash changed: '+rel);continue
    if not oracle.is_file() or sha(oracle)!=r['oracle_sha256'] or m['sha256']!=r['oracle_sha256']:errors.append('policy oracle mismatch: '+rel);continue
    try:
        dirs=CLIENT if rel.startswith('SrcGame/') else SERVER
        cur=tokens(expand(p,dirs));cur,funcs,aliases=parse_policy(cur,r['policy']);cur=inline_aliases(cur,r['policy'],aliases);cur=inline_calls(cur,r['policy'],funcs)
        pre=tokens(oracle.read_text(encoding='utf-8',errors='ignore'))
        if cur!=pre:errors.append('policy-inlined source differs from pre_refactor: '+rel)
    except Exception as e:errors.append(rel+': '+str(e))
print('Policy-equivalent targets:',len(rows));print('Frozen policy helpers    :',len(helpers))
if errors:
    print('FAIL:',len(errors));[print(' -',e) for e in errors];sys.exit(1)
print('RESULT                   : PASS')

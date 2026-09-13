#!/usr/bin/env python3
from pathlib import Path
import re,csv,hashlib,sys,os
ROOT=Path(__file__).resolve().parent
INC_RE=re.compile(r'^\s*#\s*include\s*([<"])([^">]+)[">]',re.M)
CLIENT_INC=[ROOT,ROOT/'Shared',ROOT/'Shared2',ROOT/'SrcGame/src',ROOT/'SrcGame/src/systems/rendering/legacy_3d',ROOT/'SrcGame/src/third_party/lua/legacy_src',ROOT/'SrcGame/src/third_party/lua/legacy_src/Lua',ROOT/'modules',ROOT/'modules/File',ROOT/'modules/MD5',ROOT/'modules/Helper',ROOT/'Dependency/DirectX9/Include']
SERVER_INC=[ROOT,ROOT/'SrcServer/src',ROOT/'SrcGame/src',ROOT/'Shared',ROOT/'Shared/Lua',ROOT/'Shared2',ROOT/'modules',ROOT/'modules/File',ROOT/'Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include']

def client_tus():
    out=[]
    for p in (ROOT/'SrcGame/src').rglob('*'):
        if not p.is_file() or p.suffix.lower() not in {'.cpp','.cc','.cxx','.c'}: continue
        rel=p.relative_to(ROOT).as_posix(); low=rel.lower()
        if re.search(r'gameguard|xtrap|hackshield|lua\.bak|\.bak|bak_|corrupt|/luac/|ltests\.c|/test/|/tests/',low,re.I): continue
        if re.search(r'hoskyold\.cpp|hoopeningoff\.cpp|filtereffect\.cpp|cchatbox\.cpp|/third_party/lua/legacy_src/lua/|/compat/client_server/server_legacy/(clientfuncpos|onsever|partysever|svr_damge|svreventspawn)\.cpp',low,re.I): continue
        out.append(p)
    md=ROOT/'modules/MD5'
    if md.exists(): out += [p for p in md.rglob('*') if p.is_file() and p.suffix.lower() in {'.cpp','.cc','.cxx','.c'}]
    return sorted(set(out))

def server_tus():
    out=[]
    for base in [ROOT/'SrcServer/src',ROOT/'modules']:
        if not base.exists(): continue
        for p in base.rglob('*'):
            if not p.is_file() or p.suffix.lower() not in {'.cpp','.c'}: continue
            low=p.relative_to(ROOT).as_posix().lower()
            if re.search(r'anticheat|lua\.bak|\.bak|bak_|corrupt|/lua/lua\.c|/luac/|ltests\.c',low,re.I): continue
            out.append(p)
    return sorted(set(out))

def resolve(cur,name,q,incs):
    c=[]
    if q=='"': c.append(cur.parent/name)
    c.extend(d/name for d in incs)
    for p in c:
        if p.is_file(): return p.resolve()
    return None

def walk(tus,incs):
    seen=set(); stack=list(tus)
    while stack:
        p=stack.pop().resolve()
        if p in seen or not p.is_file(): continue
        seen.add(p)
        txt=p.read_text(encoding='utf-8',errors='ignore')
        for q,n in INC_RE.findall(txt):
            r=resolve(p,n,q,incs)
            if r and r not in seen: stack.append(r)
    return seen
active=walk(client_tus(),CLIENT_INC)|walk(server_tus(),SERVER_INC)
shared=sorted(p.relative_to(ROOT).as_posix() for p in active if str(p).startswith(str((ROOT/'Shared').resolve())+os.sep))
proof={}
for fn in ('ACTIVE_SHARED_CURRENT.tsv','ACTIVE_SHARED_TOKEN_PROOF.tsv'):
    p=ROOT/fn
    if not p.is_file():
        print('FAIL: missing',fn);sys.exit(1)
with (ROOT/'ACTIVE_SHARED_CURRENT.tsv').open(encoding='utf-8') as f:
    frozen={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
with (ROOT/'ACTIVE_SHARED_TOKEN_PROOF.tsv').open(encoding='utf-8') as f:
    tokens={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
policy_helpers={}
ph=ROOT/'STRUCTURAL_POLICY_HELPERS.tsv'
if ph.exists():
    with ph.open(encoding='utf-8') as f:
        policy_helpers={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
macro_helpers={}
mh=ROOT/'STRUCTURAL_MACRO_HELPERS.tsv'
if mh.exists():
    with mh.open(encoding='utf-8') as f:
        macro_helpers={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
errors=[]
if set(shared)!=set(frozen):
    for x in sorted(set(shared)-set(frozen)): errors.append('new active Shared dependency: '+x)
    for x in sorted(set(frozen)-set(shared)): errors.append('frozen Shared dependency disappeared: '+x)
for rel in shared:
    p=ROOT/rel; h=hashlib.sha256(p.read_bytes()).hexdigest(); r=frozen[rel]
    if h!=r['sha256']: errors.append('active Shared hash changed: '+rel)
    if r['classification']=='new-shared':
        pr=tokens.get(rel)
        mh=macro_helpers.get(rel)
        token_ok=bool(pr and pr['status']=='token-contiguous')
        macro_ok=bool(mh and mh['sha256']==h and mh.get('status')=='macro-alias-equivalent')
        ph=policy_helpers.get(rel)
        policy_ok=bool(ph and ph['sha256']==h and ph.get('status')=='inline-policy-equivalent')
        if not token_ok and not macro_ok and not policy_ok: errors.append('new Shared helper lacks frozen extraction/macro/policy proof: '+rel)
print('Active Shared files :',len(shared))
print('Frozen exact/moved  :',sum(frozen[x]['classification']!='new-shared' for x in shared))
print('Token extractions   :',sum(frozen[x]['classification']=='new-shared' for x in shared))
if errors:
    print('FAIL:',len(errors))
    for e in errors[:100]: print(' -',e)
    sys.exit(1)
print('RESULT              : PASS')

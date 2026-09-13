#!/usr/bin/env python3
from pathlib import Path
import csv,re,sys
ROOT=Path(__file__).resolve().parent
with (ROOT/'PRE_REFACTOR_SOURCE_MANIFEST.tsv').open(encoding='utf-8') as f:
    baseline={r['path'] for r in csv.DictReader(f,delimiter='\t')}
INC_RE=re.compile(r'^\s*#\s*include\s*([<"])([^">]+)[">]',re.M)
CLIENT=[ROOT,ROOT/'Shared',ROOT/'Shared2',ROOT/'SrcGame/src',ROOT/'SrcGame/src/systems/rendering/legacy_3d',ROOT/'SrcGame/src/third_party/lua/legacy_src',ROOT/'SrcGame/src/third_party/lua/legacy_src/Lua',ROOT/'modules',ROOT/'modules/File',ROOT/'modules/MD5',ROOT/'modules/Helper',ROOT/'Dependency/DirectX9/Include']
SERVER=[ROOT,ROOT/'SrcServer/src',ROOT/'SrcGame/src',ROOT/'Shared',ROOT/'Shared/Lua',ROOT/'Shared2',ROOT/'modules',ROOT/'modules/File',ROOT/'Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include']

def sources(base,target):
    out=[]
    for p in (ROOT/base).rglob('*'):
        if not p.is_file() or p.suffix.lower() not in {'.cpp','.cc','.cxx','.c'}:continue
        rel=p.relative_to(ROOT).as_posix()
        if target=='client':
            if re.search(r'GameGuard|XTrap|HackShield|Lua\.bak|\.bak|bak_|corrupt|/luac/|ltests\.c|/test/|/tests/',rel,re.I):continue
            if re.search(r'HoSkyOld\.cpp|HoOpeningOFF\.cpp|FilterEffect\.cpp|CChatBox\.cpp|/third_party/lua/legacy_src/Lua/|/compat/client_server/server_legacy/(ClientFuncPos|OnSever|PartySever|Svr_Damge|svrEventSpawn)\.cpp',rel,re.I):continue
        else:
            if re.search(r'AntiCheat|Lua\.bak|\.bak|bak_|corrupt|/lua/lua\.c|/luac/|ltests\.c',rel,re.I):continue
        out.append(p)
    return out

def resolve(cur,name,q,dirs):
    cand=([cur.parent/name] if q=='"' else [])+[d/name for d in dirs]
    return next((p.resolve() for p in cand if p.is_file()),None)
def walk(tus,dirs):
    seen=set();stack=[p.resolve() for p in tus]
    while stack:
        p=stack.pop()
        if p in seen or not p.is_file():continue
        seen.add(p)
        txt=p.read_text(encoding='utf-8',errors='ignore')
        for q,n in INC_RE.findall(txt):
            r=resolve(p,n,q,dirs)
            if r and r not in seen:stack.append(r)
    return seen
active=walk(sources('SrcGame/src','client'),CLIENT)|walk(sources('SrcServer/src','server'),SERVER)
new=[]
for p in active:
    try:rel=p.relative_to(ROOT).as_posix()
    except ValueError:continue
    if rel not in baseline:new.append(rel)
outside=sorted(x for x in new if not x.startswith('Shared/'))
allowed={
 'SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp',
 'SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.h',
 'SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.h',
 'SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.inl',
}
errors=[]
for x in outside:
    if x not in allowed:errors.append('new active non-Shared file: '+x)
for x in allowed:
    if x not in outside:errors.append('authorized SQLite active file missing: '+x)
print('Active non-baseline files:',len(new))
print('New active outside Shared:',len(outside))
for x in outside:print('  +',x,'(authorized)' if x in allowed else '(UNAPPROVED)')
if errors:
    print('FAIL:',len(errors));[print(' -',e) for e in errors];sys.exit(1)
print('RESULT: PASS')

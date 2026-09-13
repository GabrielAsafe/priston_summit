#!/usr/bin/env python3
from pathlib import Path
import csv,re,sys
ROOT=Path(__file__).resolve().parent
BASE=ROOT/'BASELINE_BUILD_SOURCE_GRAPH.tsv'
errors=[]

def client_sources(root):
    out=[]
    for p in (root/'SrcGame/src').rglob('*'):
        if not p.is_file() or p.suffix.lower() not in {'.cpp','.cc','.cxx','.c'}: continue
        rel=p.relative_to(root).as_posix(); low=rel.lower()
        if re.search(r'GameGuard|XTrap|HackShield|Lua\.bak|\.bak|bak_|corrupt|/luac/|ltests\.c|/test/|/tests/',rel,re.I): continue
        if re.search(r'HoSkyOld\.cpp|HoOpeningOFF\.cpp|FilterEffect\.cpp|CChatBox\.cpp|/third_party/lua/legacy_src/Lua/|/compat/client_server/server_legacy/(ClientFuncPos|OnSever|PartySever|Svr_Damge|svrEventSpawn)\.cpp',rel,re.I): continue
        out.append(rel)
    return sorted(set(out))

def server_sources(root):
    out=[]
    for p in (root/'SrcServer/src').rglob('*'):
        if not p.is_file() or p.suffix.lower() not in {'.cpp','.c'}: continue
        rel=p.relative_to(root).as_posix()
        if re.search(r'AntiCheat|Lua\.bak|\.bak|bak_|corrupt|/lua/lua\.c|/luac/|ltests\.c',rel,re.I): continue
        out.append(rel)
    return sorted(set(out))

with BASE.open(encoding='utf-8') as f:
    rows=list(csv.DictReader(f,delimiter='\t'))
expected_client={r['path'] for r in rows if r['target']=='client'}
expected_server={r['path'] for r in rows if r['target']=='server'}
cur_client=set(client_sources(ROOT)); cur_server=set(server_sources(ROOT))
allowed_server={'SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp'}
for p in sorted(expected_client-cur_client): errors.append('client source removed: '+p)
for p in sorted(cur_client-expected_client): errors.append('client source added: '+p)
for p in sorted(expected_server-cur_server): errors.append('server source removed: '+p)
for p in sorted(cur_server-expected_server-allowed_server): errors.append('server source added without authorization: '+p)
if not allowed_server <= cur_server: errors.append('authorized SQLite source missing')
print('Client baseline/current (repo-local, excl. modules):',len(expected_client),'/',len(cur_client))
print('Server baseline/current (repo-local, excl. modules):',len(expected_server),'/',len(cur_server))
print('Authorized server additions:',len(cur_server-expected_server))
for p in sorted(cur_server-expected_server): print('  +',p,'(authorized)' if p in allowed_server else '(UNAPPROVED)')
if errors:
    print('FAIL:',len(errors)); [print(' -',e) for e in errors]; sys.exit(1)
print('RESULT: PASS')

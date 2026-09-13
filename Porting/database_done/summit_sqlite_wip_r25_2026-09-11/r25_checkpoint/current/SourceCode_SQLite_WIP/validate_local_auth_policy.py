#!/usr/bin/env python3
"""Validate the R23 explicit local/legacy authentication policy."""
from __future__ import annotations
import sqlite3, tempfile
from pathlib import Path

ROOT=Path(__file__).resolve().parent
MIG=ROOT/'Database'/'migrations'
DLL=ROOT/'SrcServer/src/core/server/legacy_runtime/DllServer.cpp'

def apply(db: sqlite3.Connection, upto: int) -> None:
    for p in sorted(MIG.glob('[0-9][0-9][0-9]_*.sql')):
        v=int(p.name[:3])
        if v>upto: break
        db.executescript(p.read_text(encoding='utf-8'))
        db.execute(f'PRAGMA user_version={v}')

def defaults(db, table):
    out={}
    for _,name,typ,notnull,default,pk in db.execute(f'PRAGMA table_info("{table}")'):
        if notnull and default is None and not pk:
            out[name]=0 if 'INT' in (typ or '').upper() else '0'
    return out

def insert_account(db,user='PolicyUser'):
    table=f'AccountDB__{user[0].upper()}GameUser'; v=defaults(db,table)
    v.update(userid=user,Passwd='Secret',GPCode='OLDPAID',UsePeriod=123,inuse='0',BlockChk='0',DelChk='0',RNo=88,BNum=99,Channel='legacy',ServerName='',SNo=None)
    cols=list(v); db.execute(f'INSERT INTO "{table}" ({",".join("["+c+"]" for c in cols)}) VALUES ({",".join("?" for _ in cols)})',[v[c] for c in cols])
    return table

def main():
    sql32=(MIG/'032_local_auth_policy.sql').read_text(encoding='utf-8')
    assert 'UserID TEXT COLLATE NOCASE' in sql32
    assert "lower(AuthMode) IN ('local','legacy')" in sql32
    assert "legacy_billing_fallback', '1'" in sql32
    with tempfile.TemporaryDirectory(prefix='r23_auth_') as td:
        path=Path(td)/'db.sqlite3'; db=sqlite3.connect(path)
        apply(db,31); table=insert_account(db)
        db.execute("INSERT INTO LocalAuth__GameplayEntitlement(UserID,Entitlement,Enabled,ExpiresAt,UpdatedAt) VALUES ('PolicyUser','pcbang_gameplay',1,NULL,datetime('now'))")
        before=db.execute(f'SELECT GPCode,UsePeriod,RNo,BNum,Channel FROM "{table}" WHERE userid="PolicyUser"').fetchone()
        apply(db,32)
        assert db.execute('PRAGMA user_version').fetchone()[0]==32
        assert db.execute("SELECT PolicyValue FROM LocalAuth__RuntimePolicy WHERE PolicyKey='legacy_billing_fallback'").fetchone()[0]=='1'
        assert db.execute(f'SELECT GPCode,UsePeriod,RNo,BNum,Channel FROM "{table}" WHERE userid="PolicyUser"').fetchone()==before
        assert db.execute("SELECT Enabled FROM LocalAuth__GameplayEntitlement WHERE UserID='policyuser' AND Entitlement='pcbang_gameplay'").fetchone()==(1,)
        db.execute("INSERT INTO LocalAuth__AccountPolicy(UserID,AuthMode,Enabled) VALUES ('PolicyUser','local',1)")
        try:
            db.execute("INSERT INTO LocalAuth__AccountPolicy(UserID,AuthMode,Enabled) VALUES ('policyuser','legacy',1)")
            raise AssertionError('NOCASE primary key did not reject duplicate')
        except sqlite3.IntegrityError:
            pass
        assert db.execute("SELECT AuthMode FROM LocalAuth__AccountPolicy WHERE UserID='POLICYUSER'").fetchone()==('local',)
        db.close()
    src=DLL.read_text(encoding='latin1')
    required=[
        'SQLITE_LOCAL_AUTH_LOCAL','SQLITE_LOCAL_AUTH_LEGACY','ReadSQLiteLocalAuthMode',
        'if(authMode == SQLITE_LOCAL_AUTH_LEGACY)','const BOOL explicitlyLocal = authMode == SQLITE_LOCAL_AUTH_LOCAL',
        'return explicitlyLocal ? TRUE : FALSE','LocalAuth__AccountPolicy',
        "WHERE Enabled=1 AND lower(AuthMode)='local'",'lpszGPCode[0] = 0',
        '*((int *)lpszUsePeriod) = 0','*((int *)lpszPCRNo) = 0'
    ]
    missing=[x for x in required if x not in src]
    assert not missing, missing
    print('R23 local auth policy PASS: explicit local/legacy ownership, local-only Database.dll switch, NOCASE constraints, stale billing isolation, and v31->v32 preservation verified')
    return 0
if __name__=='__main__': raise SystemExit(main())

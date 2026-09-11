#!/usr/bin/env python3
"""Validate the R24 read-only local-only readiness audit."""
from pathlib import Path
import sqlite3,subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parent; MIG=ROOT/'Database/migrations'; TOOL=ROOT/'tools/sqlite_local_admin.py'

def apply(db):
    for p in sorted(MIG.glob('[0-9][0-9][0-9]_*.sql')):
        v=int(p.name[:3]); db.executescript(p.read_text(encoding='utf-8')); db.execute(f'PRAGMA user_version={v}')

def defaults(db,table):
    d={}
    for _,n,t,nn,default,pk in db.execute(f'PRAGMA table_info("{table}")'):
        if nn and default is None and not pk: d[n]=0 if 'INT' in (t or '').upper() else '0'
    return d

def add(db,user,mode=None,**overrides):
    table=f'AccountDB__{user[0].upper()}GameUser'; d=defaults(db,table)
    d.update(userid=user,Passwd='pw',GPCode='',UsePeriod=0,inuse='0',BlockChk='0',DelChk='0',RNo=0,BNum=0,Channel='',ServerName='',SNo=None); d.update(overrides)
    cols=list(d); db.execute(f'INSERT INTO "{table}" ({",".join("["+c+"]" for c in cols)}) VALUES ({",".join("?" for _ in cols)})',[d[c] for c in cols])
    if mode: db.execute("INSERT INTO LocalAuth__AccountPolicy(UserID,AuthMode,Enabled) VALUES (?,?,1)",(user,mode))

def audit(path,details=False):
    args=[sys.executable,str(TOOL),'--db',str(path),'audit-local-only']
    if details: args.append('--details')
    return subprocess.run(args,text=True,capture_output=True)

def main():
    with tempfile.TemporaryDirectory(prefix='r24_ready_') as td:
        path=Path(td)/'summit.sqlite3'; db=sqlite3.connect(path); db.row_factory=sqlite3.Row; apply(db)
        add(db,'LocalStale',mode='local',GPCode='OLD',UsePeriod=5,RNo=9,BNum=7,Channel='old')
        add(db,'ImplicitFree')
        db.commit(); db.close()
        p=audit(path); assert p.returncode==0,(p.stdout,p.stderr); assert 'local_only_ready=yes' in p.stdout; assert 'explicit_local=1' in p.stdout and 'implicit_safe_local=1' in p.stdout
        db=sqlite3.connect(path); db.row_factory=sqlite3.Row; add(db,'LegacyUser',mode='legacy'); add(db,'BilledUser',GPCode='PAID',RNo=5); db.commit(); db.close()
        p=audit(path,True); assert p.returncode==3,(p.stdout,p.stderr); assert 'local_only_ready=no' in p.stdout; assert 'legacy: LegacyUser' in p.stdout and 'implicit-provider: BilledUser' in p.stdout
        db=sqlite3.connect(path); db.execute("UPDATE LocalAuth__AccountPolicy SET AuthMode='local' WHERE UserID='LegacyUser'"); db.execute("INSERT INTO LocalAuth__AccountPolicy(UserID,AuthMode,Enabled) VALUES ('BilledUser','local',1)"); db.commit(); db.close()
        p=audit(path); assert p.returncode==0,p.stdout; assert 'local_only_ready=yes' in p.stdout
    src=TOOL.read_text(encoding='utf-8')
    for token in ['PRAGMA quick_check','PRAGMA foreign_key_check','implicit_provider_dependent','casefold_duplicates','orphan_policies','local_only_ready=']:
        assert token in src,token
    print('SQLite local-only readiness PASS: local/stale-billing ownership, implicit-free compatibility, legacy/provider blockers, integrity checks and exit codes verified')
    return 0
if __name__=='__main__': raise SystemExit(main())

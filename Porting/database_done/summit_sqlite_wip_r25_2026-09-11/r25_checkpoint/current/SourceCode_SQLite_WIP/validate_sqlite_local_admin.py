#!/usr/bin/env python3
"""Exercise the R23 local SQLite administration CLI against a disposable DB."""
from pathlib import Path
import sqlite3,tempfile,subprocess,sys
ROOT=Path(__file__).resolve().parent; MIG=ROOT/'Database'/'migrations'; TOOL=ROOT/'tools/sqlite_local_admin.py'

def apply(db):
    for p in sorted(MIG.glob('[0-9][0-9][0-9]_*.sql')):
        v=int(p.name[:3]); db.executescript(p.read_text(encoding='utf-8')); db.execute(f'PRAGMA user_version={v}')

def run(db,*args,ok=True):
    p=subprocess.run([sys.executable,str(TOOL),'--db',str(db),*args],text=True,capture_output=True)
    if ok and p.returncode!=0: raise AssertionError((args,p.stdout,p.stderr))
    if not ok and p.returncode==0: raise AssertionError(('expected failure',args,p.stdout))
    return p

def main():
    with tempfile.TemporaryDirectory(prefix='r23_admin_') as td:
        path=Path(td)/'summit.sqlite3'; db=sqlite3.connect(path); apply(db); db.commit(); db.close()
        run(path,'create-account','LocalUser','--password','Secret123')
        run(path,'create-account','localuser','--password','Other',ok=False)
        run(path,'set-password','LocalUser','--password','NewSecret')
        run(path,'set-auth','LocalUser','legacy'); run(path,'set-auth','LocalUser','local')
        run(path,'grant-pcbang','LocalUser','--expires','2026-12-31 23:59:59')
        run(path,'set-gm','LocalUser','--password','gmsecret','--security','4')
        run(path,'set-billing-fallback','off')
        out=run(path,'show','LocalUser').stdout
        assert "auth_policy={'AuthMode': 'local'" in out and 'pcbang_gameplay=' in out and 'gm=' in out
        db=sqlite3.connect(path)
        assert db.execute("SELECT Passwd FROM AccountDB__LGameUser WHERE lower(userid)=lower('LocalUser')").fetchone()==('NewSecret',)
        assert db.execute("SELECT AuthMode FROM LocalAuth__AccountPolicy WHERE UserID='localuser'").fetchone()==('local',)
        assert db.execute("SELECT Enabled FROM LocalAuth__GameplayEntitlement WHERE UserID='LOCALUSER' AND Entitlement='pcbang_gameplay'").fetchone()==(1,)
        assert db.execute("SELECT security,active FROM AccountDB__GameMasters WHERE lower(accountId)=lower('LocalUser')").fetchone()==(4,1)
        assert db.execute("SELECT PolicyValue FROM LocalAuth__RuntimePolicy WHERE PolicyKey='legacy_billing_fallback'").fetchone()==('0',)
        db.close()
    print('SQLite local admin PASS: account/auth/password/PCBang/GM operations and duplicate guards verified')
    return 0
if __name__=='__main__': raise SystemExit(main())

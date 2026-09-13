#!/usr/bin/env python3
"""Validate R23's opt-in runtime path where Database.dll is not loaded."""
from pathlib import Path
import re
ROOT=Path(__file__).resolve().parent
DLL=ROOT/'SrcServer/src/core/server/legacy_runtime/DllServer.cpp'
s=DLL.read_text(encoding='latin1')
required=[
 'int BILL_SQL::Init(BOOL loadLegacyDatabase)',
 'if(!loadLegacyDatabase)',
 'hLogSqlDLL = LoadLibrary("Database")',
 'IsSQLiteLegacyBillingFallbackEnabled()',
 'BOOL legacyBillingFallback = IsSQLiteLegacyBillingFallbackEnabled();',
 'BillSql.Init(legacyBillingFallback)',
 "PolicyKey='legacy_billing_fallback'",
 'if(BillSql.SQLLogon)','if(BillSql.ServerDown)','if(BillSql.ServerExit)',
 'if(BillSql.GetGPCode)','if(BillSql.PCUse)','if(BillSql.PCSearchReserv)',
 'if(BillSql.EventSQLLogon)','if(BillSql.EventSQLLogout)','if(BillSql.PCRNoReservProduct)'
]
missing=[x for x in required if x not in s]
assert not missing, missing
# The guard must appear before the module load inside Init.
init=s[s.index('int BILL_SQL::Init(BOOL loadLegacyDatabase)'):s.index('BILL_SQL BillSql;')]
assert init.index('if(!loadLegacyDatabase)') < init.index('LoadLibrary("Database")')
# Constructor zeroing is what makes skipped imports safe.
ctor=s[s.index('BILL_SQL::BILL_SQL()'):s.index('BILL_SQL::~BILL_SQL()')]
assert 'ZeroMemory(this, sizeof(BILL_SQL));' in ctor
# bSql_LogOn must fail deterministically when neither SQLite nor provider handles it.
login=s[s.index('int\tbSql_LogOn('):s.index('LoginResult login',s.index('int\tbSql_LogOn('))]
assert 'result = 0;' in login and 'if(BillSql.SQLLogon)' in login
print('Database.dll local-only PASS: runtime policy skips LoadLibrary; auth failure is deterministic; remaining provider calls are guarded')

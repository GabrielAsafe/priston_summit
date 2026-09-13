# SQLite WIP R18 — validation report

Date: 2026-09-11

## Scope

R18 does not change schema version. It isolates gameplay persistence hidden inside the supplied `Database.dll` and migrates the proven clan/siege money responsibilities local-first to SQLite.

Changes:

- `TrySQLiteLocalClanFund` for `ClanMoney` / `SiegeMoney`;
- `TrySQLiteLocalSiegeMoneyTax` for Bless Castle tax persistence;
- event paths prefer SQLite and retain the DLL only as infrastructure-failure fallback;
- new Database.dll gameplay dependency inventory/audit;
- new regression validator `validate_database_dll_gameplay_contract.py`.

Database.dll evidence SHA-256:

`c9fd54668cba6e00fa760bda541587635f51c962ab6ef2b19179d0651e7220a7`

## Results

All validators available in this environment PASS after the R18 changes:

- backup catalog completion: 326/326 backup tables;
- BillingDB schema contract: 43 tables;
- SoD2 contract;
- migrations: `user_version=29`, 332 user tables;
- SQL query coverage: missing=0;
- clan contract;
- transaction API;
- clan mark/tag local storage;
- SQLite local/free login;
- notice contract;
- server HTTP surface: only `clanChipChange.asp` remains;
- Database.dll gameplay contract: ClanMoney/SiegeMoney/SiegeMoneyTax;
- source graph: only authorized new server TU is `SQLiteConnection.cpp`;
- build/link scripts frozen;
- port structure 1517/1517;
- compiler adaptation ledger PASS.

Raw output: `docs/R18_VALIDATION_RAW.txt`.

## Specific Database.dll contract tests

The new validator creates a DB from the real migrations and verifies:

- ClanMoney withdrawal may make the balance negative, matching the DLL;
- operation codes/signs `01/-amount`, `02/+amount`;
- recovered `DelActive` asymmetry;
- SiegeMoney withdrawal/deposit behavior;
- `SiegeMoneyTax` parses CL.IDX from the last six mark digits;
- a 10% tax with Total=1100 credits 100 SiegeMoney under recovered DLL arithmetic;
- tax and siege log rows use the recovered Server/GS/02 contract;
- malformed mark and zero-total cases do not mutate the DB.

## Not proven by these tests

R18 still has no target Windows compile/link/runtime proof. In particular, not yet proven end-to-end:

- real server startup with sqlite3.dll;
- clan fund UI/network behavior;
- Bless Castle tax cycle at runtime;
- Database.dll fallback failure behavior on the target;
- login/session paid/PC-room semantics;
- final DLL/ODBC/WinInet removal.

R6.1 remains the regression oracle until the full target runtime suite passes.

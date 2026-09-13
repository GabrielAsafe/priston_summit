# SQLite WIP R19 — validation report

Date: 2026-09-11

## Scope

R19 does not change schema version. It extends the conservative local/free AccountDB path with the session lifecycle recovered from the supplied `Database.dll`:

- atomic session claim during local/free login;
- local-first session release during logout;
- narrow stale local/free session cleanup at startup and final server exit;
- paid/PC-room/ambiguous rows remain provider-owned;
- no `Bl_RNo` gameplay changes.

Database.dll evidence SHA-256:

`c9fd54668cba6e00fa760bda541587635f51c962ab6ef2b19179d0651e7220a7`

## R19-specific result

New validator `validate_sqlite_local_session.py` PASS:

- local/free account claim writes `inuse=1`, `ServerName`, `SNo`, `StartDay`, `EditDay`, `RNo=NULL`;
- second local claim is rejected while the session is active;
- wrong GUID cannot release the session;
- matching logout records `LastDay` and clears `inuse`, `SNo`, `RNo`;
- stale reset clears only local/free rows for the current server;
- paid/billing row is preserved;
- another server's row is preserved.

## Full regression-validator results

All validators available in this environment PASS after the R19 changes:

- backup catalog completion: 326/326 backup tables;
- BillingDB schema contract: 43 tables;
- SoD2 contract;
- migrations: `user_version=29`, 332 user tables;
- SQL query coverage: missing=0;
- clan contract;
- transaction API;
- clan mark/tag local storage;
- SQLite local/free login;
- SQLite local/free session lifecycle;
- notice contract;
- server HTTP surface: only `clanChipChange.asp` remains;
- Database.dll gameplay contract: ClanMoney/SiegeMoney/SiegeMoneyTax;
- source graph: only authorized new server TU is `SQLiteConnection.cpp`;
- build/link scripts frozen;
- port structure 1517/1517;
- compiler adaptation ledger PASS.

Raw output: `docs/R19_VALIDATION_RAW.txt`.

## Not proven by these tests

R19 still has no target Windows compile/link/runtime proof. In particular, not yet proven end-to-end:

- actual server startup with the R19 code;
- local free login against a clean DB;
- simultaneous second login behavior;
- queued logout timing;
- disconnect/reconnect;
- clean shutdown session release;
- crash/kill then startup stale-session recovery;
- character create/load/save and all broader R6.1 gameplay regressions;
- paid/PC-room fallback semantics;
- final Database.dll/ODBC/WinInet removal.

R6.1 remains the regression oracle until the full Windows target suite passes.

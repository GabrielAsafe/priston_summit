# SQLite local/free session audit — R19

Date: 2026-09-11

## Purpose

R19 continues the R18 Database.dll dismantling by separating **free/local account session ownership** from billing/PC-room behavior. It does not change the schema, does not change `Bl_RNo` gameplay bonuses, and does not remove the legacy DLL fallback for paid/PC-room/ambiguous accounts.

R6.1 remains the compile/link/runtime-confirmed regression oracle.

## Binary evidence from the supplied Database.dll

Supplied DLL SHA-256:

`c9fd54668cba6e00fa760bda541587635f51c962ab6ef2b19179d0651e7220a7`

The DLL contains the proven account-session writes used by its `SetGameUser` / logout helpers.

### Login/session claim

Recovered SQL string:

```sql
update %s set StartDay = '%s', EditDay ='%s', RNo=NULL,
ServerName ='%s', inuse='1', SNo='%s' where userid = '%s'
```

A second variant is identical except that it writes a non-null `RNo` for PC-room/billing cases. R19 uses only the `RNo=NULL` form because its local path accepts only the already-proven free subset.

### Logout/session release

Recovered SQL string:

```sql
UPDATE %s SET LastDay = '%s', inuse='0', RNo=NULL, SNo=NULL
WHERE userid = '%s' and ServerName='%s'
```

This proves that `inuse`, `ServerName`, `SNo`, `StartDay/EditDay` and `LastDay` are session state, not merely commercial accounting metadata.

### ServerDown / ServerExit

The supplied DLL's `ServerDown` path inserts a row into `ServerDown(ServerName)`. Its inspected code does not perform the per-account `inuse` reset above.

The supplied DLL's `ServerExit` export closes/releases the DLL's database connection. Therefore R19 does **not** reinterpret `ServerDown` as an account-session cleanup event. Local stale-session cleanup is done at startup and final `bSql_ServerExit()` only.

### SetAllGameUser

The server bridge requests `SetAllGameUser`, but the supplied DLL does not export it. R18 had already classified this as an optional startup-maintenance function. R19 replaces only its needed local/free intent with a narrowly filtered SQLite reset.

## R19 implementation

File changed:

`SrcServer/src/core/server/legacy_runtime/DllServer.cpp`

### Atomic local/free login claim

`TrySQLiteLocalLogOn` now:

1. opens `AccountDB` and starts `BEGIN IMMEDIATE`;
2. reads and validates the same conservative free/local subset as R18;
3. creates the session GUID;
4. writes `StartDay`, `EditDay`, `ServerName`, `inuse='1'`, `SNo`, `RNo=NULL` in the same transaction;
5. commits only after the session claim succeeds.

This closes the R18 race where two local/free logins could both read `inuse=0` without either claiming the row.

### Local/free logout

`TrySQLiteLocalLogout` handles only a row that proves all of the following:

- free/local billing signature remains present;
- `inuse='1'`;
- stored `SNo` equals the queued logout GUID;
- stored `ServerName` equals the current server.

It then writes `LastDay`, `inuse='0'`, `RNo=NULL`, `SNo=NULL` transactionally. If the row does not match the local signature, the existing `Database.dll!SQLLogout` path remains the fallback.

### Startup/final-exit stale reset

`ResetSQLiteLocalSessionsForServer` scans the 26 `<initial>GameUser` tables and clears only rows that match the R19 local/free session signature:

- current `ServerName`;
- `inuse='1'`;
- non-empty `SNo`;
- zero/empty `GPCode`;
- `UsePeriod=0`;
- `BNum=0`;
- zero/empty `Channel`;
- null/zero `RNo`.

Paid/PC-room rows and rows belonging to another server are not changed.

The reset runs:

- after provider initialization during `LoadLogSqlDLL()` to recover from an unclean previous server exit;
- at final `bSql_ServerExit()` to release local/free sessions on normal shutdown.

It is intentionally **not** run from `bSql_ServerDown()`, because that function is called when the shutdown countdown starts and the supplied DLL evidence does not show account cleanup there.

## What R19 does not change

- no schema migration / `user_version` bump;
- no billing provider reconstruction;
- no changes to `Bl_RNo`, `GpType`, `GpCode`, EXP/drop/pet behavior or expiry timers;
- no removal of `SQLLogon`/`SQLLogout` DLL fallbacks for non-local accounts;
- no change to `clanChipChange.asp`;
- no optional log-sink cleanup yet;
- no ODBC/WinInet/Database.dll removal yet.

## New validator

`validate_sqlite_local_session.py` proves against the real migration set that:

- a free/local row is claimed with `inuse=1`, server, GUID and timestamps;
- a second claim is rejected while the first session is active;
- a wrong GUID cannot release the local session;
- the matching logout clears `inuse`, `SNo` and `RNo` and records `LastDay`;
- startup/final-exit reset clears stale local/free sessions only;
- paid rows and another server's rows remain unchanged.

## Runtime status

This remains static/contract evidence. Windows target compile/link and end-to-end login/disconnect/reconnect tests are still mandatory before R19 can replace R6.1 as a baseline.

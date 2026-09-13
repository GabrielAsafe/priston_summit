# RETOMADA — Summit SQLite WIP R19

Date: 2026-09-11

Read `RETOMADA_COMPLETA_R19.md` first. R6.1 remains the only compile/link/runtime-confirmed baseline.

## R19 objective

Continue dismantling `Database.dll` by responsibility, without changing gameplay: move the proven **free/local session lifecycle** behind the existing SQLite compatibility path while retaining billing/PC-room fallback.

## What changed

`SrcServer/src/core/server/legacy_runtime/DllServer.cpp` now implements:

- atomic free/local session claim during `TrySQLiteLocalLogOn`;
- `TrySQLiteLocalLogout` local-first for matching free/local sessions;
- narrow stale-session cleanup for the current server at startup and final exit.

The writes match SQL recovered from the supplied DLL:

- login: `StartDay/EditDay`, `ServerName`, `inuse='1'`, `SNo`, `RNo=NULL`;
- logout: `LastDay`, `inuse='0'`, `RNo=NULL`, `SNo=NULL`.

The login read + claim is wrapped in `BEGIN IMMEDIATE`, closing the R18 duplicate-local-session race.

## Important boundary

R19 still does **not** treat paid/PC-room accounts as local. `SQLLogon`, `SQLLogout`, `PCUse`, `PCSearchReserv`, `PCRNoReservProduct` and the gameplay meaning of `Bl_RNo` remain legacy/fallback responsibilities where applicable.

`ServerDown` is not used to clear accounts: binary evidence shows it is a server-down record path and the source calls it at the beginning of the shutdown countdown. Local session reset is performed at startup and final `ServerExit` instead.

## Validation

New validator:

`validate_sqlite_local_session.py`

It proves claim/release ownership, duplicate-claim rejection, stale cleanup, and preservation of paid/other-server rows. All R18 validators continue to pass in this environment.

## Next priority

1. Windows compile/link and runtime proof for R19 local login/logout/reconnect.
2. Decide the simplified replacement for the gameplay entitlement currently represented by `Bl_RNo` before removing PC-room functions.
3. Classify/migrate or intentionally disable optional audit sinks (`CharLog`, item/trade/post logs, `WithSOD2RecordRanks`).
4. Resolve `clanChipChange.asp` only after its exact contract is recovered.
5. Remove Database.dll/ODBC/WinInet only after zero required callers and target link/runtime tests.

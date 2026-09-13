# SQLite WIP R17 — Validation Report

Date: 2026-09-06

## Scope

R17 is a web-surface reduction checkpoint built on frozen R16. It does **not** change SQLite schema or migrations. It neutralizes server-side legacy HTTP paths that were either proven dead in the current production call graph or already replaced by SQLite/local storage, while deliberately preserving the one unresolved active clan HTTP path (`clanChipChange.asp`).

R6.1 remains the last compile/link/runtime-confirmed functional baseline.

## Source delta

R16 -> R17 source-tree changes:

- `Shared/security/integrity/isaocheck_legacy_server.inl`
- `SrcServer/src/systems/presentation/opening/HoOpening.cpp`
- new `validate_server_http_surface.py`

No migration file is added.

## Disabled legacy HTTP paths

R17 keeps the old method symbols for compatibility but removes network side effects from these server implementations:

- `ISAOCHECK::IsaoCheck` (`login.php`, `LoginCheck.php`, `InsertUser.php`) — fail-closed; real server login remains `bSql_LogOn/SQLLogon`, with the proven free subset already handled by SQLite.
- `ISAOCHECK::IsaoCheck_GameExit` (`Login_End.php`) — local no-op success; no active production caller found.
- `ISAOCHECK::read_ClanName` (`clan_readClanName.php`) — fail-closed; active clan reads already use SQLite.
- `ISAOCHECK::WarhouseRight` (`clan_WarehouseRight.php`) — fail-closed; no producer/caller found and persistence semantics are unproven.
- `ISAOCHECK::updateFlag` (`updateFlag.php`) — fail-closed for the same reason.
- `ISAOCHECK::GetWavFile` (`NPCSound/*.yul`) — disabled; queue code exists but no production producer was found.
- `ISAOCHECK::write_ClanImageFile` (`clan_insertClanImage.php`) — disabled; marks now use local storage.
- `ISAOCHECK::WEBdb_Exam` (`ktj_login.php`) — disabled diagnostic path.

`SrcServer/src/systems/presentation/opening/HoOpening.cpp` retains the global `szJoinWebUrl` symbol for linkage caution but sets it to an empty string instead of an external homepage URL.

## Deliberately preserved HTTP path

`ISAOCHECK::LeaveClan(...)` still calls `clanChipChange.asp`.

Despite the legacy name, this is the clan **leader-transfer** flow. The production clan UI calls it with the selected member. The client accepts response codes 1..6/100 and deducts 300000 after code 1.

The current backups prove relevant CL/UL/log table layouts but do not prove the exact historical write/log sequence or meaning of every response code. R17 therefore does not replace this endpoint by approximation.

## New validator

`validate_server_http_surface.py` checks:

- exactly one HTTP/HTTPS string literal remains in the active server-specific isaocheck bridge;
- that literal is `clanChipChange.asp`;
- removed login/session/clan-read/warehouse/updateFlag/NPCSound/upload/diagnostic endpoint literals are absent;
- each disabled API contains the intended local stub and no `connect_WEBdb` call;
- `clanChipChange` remains preserved in the unresolved active path;
- `SrcServer` contains no direct HTTP/HTTPS string literal after the server homepage neutralization.

Result:

`Server HTTP surface PASS: remaining_http_literals=1 (clanChipChange.asp only)`

## Schema state

Unchanged from R16/R15:

- `PRAGMA user_version = 29`
- 332 user tables
- 326/326 backup-proven tables represented
- 6 source-derived compatibility tables explicitly distinguished from backup-proven schema

## Full validation results

PASS in the available environment:

- backup catalog completion — 326/326 tables, 332 total;
- BillingDB — 43 tables, exact column order/identity translation;
- SoD2 — index1 0..6 and index3 local protocol;
- SQLite migrations — version 29 / 332 / idempotence;
- SQL query coverage — missing 0, forbidden external/positional 0;
- clan SQLite contract — create, roles, member list, sub-chief, join/kick/leave, dissolution, legacy clwon1 behavior;
- transaction API;
- clan mark local storage;
- clan tag local storage;
- local/free SQLite login;
- notice SQLite contract;
- R17 server HTTP surface;
- source graph — one authorized new server TU, `SQLiteConnection.cpp`;
- build/link scripts frozen — 4;
- port structure — 1517/1517;
- compiler adaptation ledger — 1.

Raw output: `docs/R17_VALIDATION_RAW.txt`.

## What is not proven

R17 still does not prove:

- target Windows compile/link;
- runtime behavior in the actual game/server;
- leader-transfer replacement;
- paid/PC-room billing equivalence;
- historical SQL Server data import;
- safe removal of WinInet while `clanChipChange` remains;
- safe removal of `Database.dll` or ODBC;
- end-to-end persistence regression.

R6.1 remains the functional oracle.

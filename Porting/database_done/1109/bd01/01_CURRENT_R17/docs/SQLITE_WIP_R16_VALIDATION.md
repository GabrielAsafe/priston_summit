# SQLite WIP R16 — Validation Report

Date: 2026-09-06

## Scope

R16 is a functional persistence checkpoint built on frozen R15. It removes the active clan-creation HTTP call and implements the proven clan creation database/protocol contract through SQLite. No migration/schema version is added.

## Source delta

R15 -> R16 changes exactly two source-tree paths:

- `Shared/security/integrity/isaocheck_legacy_server.inl`
- `validate_clan_sqlite_contract.py`

The implementation changes `ISAOCHECK::makeClan(...)` from `clanInsert.aspx` HTTP to a direct `ClanDB` transaction. The validator gains dynamic and static clan-creation contract tests.

## Clan creation validation

The R16 clan test verifies:

- first allocation: `1000000001`;
- `LI.ID=1` allocator state and subsequent advance;
- insertion of `CL` with leader, count 1, `New clan`, allocated mark, expected zero flags/money state and current-schema `LIDER_BC=0` compatibility initialization;
- insertion of leader membership in `UL`;
- success buffer: `Code=1\rCMoney=500000\r`;
- already-in-clan result `2`;
- duplicate clan-name result `3`;
- stale empty membership cleanup;
- second allocation: `1000000002`;
- active `makeClan` contains no HTTP/connect_WEBdb path;
- transaction and local mark-copy behavior are present.

Dynamic result:

`Clan SQLite contract PASS: create/mark-allocation=PASS, roles=0/1/2/5, member-list CR/CMem=PASS, sub-chief appoint/release=PASS, join/kick/leave transactions=PASS, dissolution/archive=PASS, first-clwon legacy behavior=PASS`

## Current-schema compatibility note

The historical clan creation script used to reconstruct the observable contract predates the `LIDER_BC` field found in the supplied modern ClanDB backup. The current backup declares that field `NOT NULL` and the legacy `makeClan` API supplies no value for it.

R16 therefore initializes `LIDER_BC` to zero on create. This is explicitly classified as compatibility with the supplied current schema rather than a claim about the historical ASP implementation.

## Full validation results

All available non-Windows validations passed:

- backup catalog completion: PASS — 176 R15 additions, 326/326 backup tables present, 332 user tables;
- BillingDB: PASS — 43 tables, exact order and identity translation;
- SoD2: PASS — index1 codes 0..6, index3 protocol/ranking, HTTP removed;
- migrations: PASS — version 29, 332 user tables, contracts/idempotence;
- SQL query coverage: PASS — qualified tables 14, missing 0, forbidden external/positional 0;
- clan SQLite contract: PASS — including R16 create/mark allocation;
- SQLite transaction API: PASS;
- clan mark local storage: PASS;
- clan tag local storage: PASS;
- SQLite local/free login: PASS;
- notice SQLite contract: PASS;
- source graph: PASS — client 201/201, server 179/180, one authorized addition `SQLiteConnection.cpp`;
- build/link scripts: PASS — 4 frozen;
- port structure: PASS — 1517/1517;
- compiler adaptation ledger: PASS.

Raw output is stored in `docs/R16_VALIDATION_RAW.txt`.

## What is not proven

R16 still does not prove:

- target Windows compile/link of the current SQLite tree;
- runtime clan creation in the actual game/server;
- paid/PC-room billing equivalence;
- leader-transfer or warehouse/updateFlag web replacement;
- removal of ODBC/Database.dll/WinInet;
- historical data import from SQL Server backups.

R6.1 remains the functional regression oracle.

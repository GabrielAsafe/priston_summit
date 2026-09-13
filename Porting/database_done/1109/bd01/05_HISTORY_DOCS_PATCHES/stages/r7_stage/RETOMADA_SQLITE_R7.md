# RETOMADA — SQLite WIP R7

Date: 2026-09-06

## Baseline rule

`port_rebuild_r6_1` remains the last user-confirmed functional baseline. `current/SourceCode_SQLite_WIP` is experimental and must not become the regression oracle until Windows compile/link/runtime validation succeeds.

## Current SQLite state

- Migrations: schema version 16.
- SQLite user tables: 111.
- SQL query coverage validator: PASS.
- Unknown bare singleton tables fail routing explicitly instead of defaulting silently to ServerDB.
- One authorized new server translation unit remains: `SQLiteConnection.cpp`.

## Clan migration completed through R7

Read paths now local SQLite:
- clan leader existence/name/member checks already migrated in R6;
- `isClanMember` metadata/role response migrated locally;
- roles preserved: 0 none, 1 member, 2 leader, 5 sub-chief;
- sub-chief represented by `ClanDB__UL.Permi='2'`;
- member `MIconCnt` synchronization and stale-UL cleanup preserved.

Mutable clan paths now local SQLite and transactional:
- join/invite member;
- kick/banish member;
- self-leave.

The legacy kick parser intentionally consumes only `clwon1`, matching the old ASP behavior.

## Transaction API

`SQLConnection`/`SQLiteConnection` now provide:
- `BeginTransaction()`;
- `CommitTransaction()`;
- `RollbackTransaction()`.

SQLite uses `BEGIN IMMEDIATE`. Closing a connection with an active transaction rolls it back.

## Still HTTP/web-backed after R7

Important remaining clan/service endpoints include:
- `GetAllMyClanMember.asp` (`read_ClanData`);
- clan creation and clan dissolution;
- sub-chief update/release;
- clan leader/chip change;
- PFlag/KFlag and related flag updates;
- warehouse rights;
- clan mark image read/write/storage;
- notice/external asset paths and other non-DB HTTP paths.

## Next recommended work

1. Reconstruct `GetAllMyClanMember` response from the client parser and migrate it locally.
2. Migrate sub-chief update/release using `UL.Permi`, with transactions.
3. Only then consider clan leader change/dissolution and flag/warehouse semantics.
4. Keep asset/notice HTTP removal separate from relational DB operations.

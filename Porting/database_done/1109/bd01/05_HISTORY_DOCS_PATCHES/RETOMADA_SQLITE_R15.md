# RETOMADA — SQLite WIP R15

Data: 2026-09-06

## 1. Baseline contract

- R6.1 remains the last compile/link/runtime-confirmed functional baseline.
- R15 is experimental SQLite migration work and must not be used as the gameplay oracle until user validation.
- R14 is the frozen predecessor for the R14 -> R15 delta.
- Do not mix gameplay/refactor fixes into this database migration.

## 2. Purpose of R15

R15 closes the **physical SQLite schema coverage of all tables proven by the supplied SQL Server backups**.

It is deliberately schema-focused. It does not widen paid/PC-room login behavior and does not claim that SQL Server row data has been imported.

## 3. Catalog recovered from the supplied backups

Direct catalog recovery from 21 `.bak` files produced:

- 326 user tables;
- 5,568 columns.

The recovered metadata used for R15 includes table name, exact column order, SQL type, nullable state and `IDENTITY` state.

R14 already represented 150 of those 326 tables. R15 adds the missing 176.

## 4. New migrations

- `019_billinglogdb_catalog.sql` — 6 tables
- `020_gamelogdb_catalog.sql` — 17
- `021_itemlogdb_catalog.sql` — 3
- `022_logdb_catalog.sql` — 5
- `023_pcroom_catalog.sql` — 3
- `024_pcroomlog_catalog.sql` — 19
- `025_rptdb_catalog.sql` — 1
- `026_sod2db_catalog.sql` — 9
- `027_soddb_catalog.sql` — 69
- `028_clandb_catalog.sql` — 24
- `029_remaining_backup_catalogs.sql` — 20

Total: 176 new backup-proven tables.

## 5. Current SQLite schema

- `PRAGMA user_version = 29`
- 332 user tables total
- 326/326 backup-proven tables represented
- 6 additional tables are explicitly source-derived compatibility contracts from earlier checkpoints:
  - `QuestDB__RepeatedQuest`
  - `SecurityDB__AgingRecord`
  - `SecurityDB__BannedIP`
  - `SecurityDB__BannedMac`
  - `SecurityDB__ClassCheatList`
  - `StartUserDB__ExtraItems`

Do not mislabel these six as backup-proven.

## 6. New audit/validation artifacts

- `migration/BACKUP_CATALOG_R15.tsv` — recovered catalog audit with 5,568 column rows.
- `migration/BACKUP_SCHEMA_COMPLETION_R15.tsv` — before/add/after table coverage by logical database.
- `validate_backup_catalog_completion.py` — verifies all 326 backup table names and exact layout/affinity/nullability/identity for the 176 R15 additions.
- `migration/SQLITE_WIP_R15_CHANGED_FILES.tsv` — hashes for the 15 changed paths.
- `migration/sqlite_wip_r14_to_r15.patch` — clean R14 -> R15 delta.

## 7. Validation status

PASS:

- backup catalog completion — 326/326;
- new catalog layouts — column order/affinity/nullability/identity;
- migrations — v29 / 332 / idempotence;
- BillingDB 43/43 contract;
- SoD2 local SQLite contract;
- SQL query coverage;
- clan contract and transactions;
- clan marks/tag local storage;
- local/free SQLite login;
- notice SQLite contract;
- build source graph;
- frozen build/link scripts;
- 1517/1517 port structure;
- compiler adaptation ledger;
- clean R14 -> R15 patch application and SHA-256 comparison: 2032/2032 files.

See `docs/SQLITE_WIP_R15_VALIDATION.md`.

## 8. Login/billing status

Do **not** treat the presence of BillingDB/PCRoom schemas as proof of the historical billing state machine.

Current policy remains:

- the proven simple/free account path can authenticate directly through SQLite;
- billing/paid, PC-room, blocked, expired, ambiguous or unsupported account states remain fallback-only to the historical `Database.dll` path when available;
- without that legacy path, unproven states fail closed.

The supplied BillingDB backup proves schema but did not provide enough populated provider/channel state to reconstruct charging semantics safely.

## 9. Data migration status

R15 reconstructs schema. It does **not** mean that all historical rows from SQL Server `.bak` files have been imported into `summit.sqlite3`.

Before a production replacement, decide whether the target is:

1. a clean SQLite database populated by normal server activity/seeds; or
2. preservation/import of historical accounts, characters, clans, logs and billing data.

If historical preservation is required, build a separate deterministic data migration instead of mixing it into schema bootstrap.

## 10. Remaining functional blockers

Primary remaining work after this checkpoint:

1. decide/reconstruct paid billing and PC-room semantics before removing `Database.dll`;
2. finish remaining active clan/web paths only where behavior is proven, especially clan creation/default mark, leadership transfer, warehouse rights/updateFlag;
3. classify/remove remaining dead legacy HTTP helpers;
4. prove no active ODBC/WinInet/DLL callers remain before removing linker/runtime dependencies;
5. perform Windows compile/link/runtime validation and the full persistence regression sequence.

## 11. Runtime promotion rule

Do not promote R15 to baseline merely because the static validators pass.

The required target-environment sequence still includes at least:

- compile server;
- link server;
- start from clean DB and apply migrations;
- login;
- create/load/save/reconnect character;
- inventory/warehouse;
- quests;
- coin shop;
- clan;
- SoD/event paths;
- restart and confirm persistence;
- concurrent SQLite/WAL access where applicable.

Until that succeeds, R6.1 remains the functional oracle.

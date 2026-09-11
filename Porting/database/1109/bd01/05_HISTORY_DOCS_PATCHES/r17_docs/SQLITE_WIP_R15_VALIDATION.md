# SQLite WIP R15 — Validation Report

Date: 2026-09-06

## Scope

R15 is a schema-completion checkpoint built on frozen R14. It adds only SQL Server backup-proven table catalogs that were still absent from the SQLite physical schema. It does not widen paid/PC-room login semantics, change gameplay, or promote the SQLite tree to the functional baseline.

## Backup catalog recovery

The supplied SQL Server `.bak` files were parsed directly to recover catalog metadata used by this checkpoint. Across 21 logical databases the recovered catalog contains:

- 326 user tables;
- 5,568 columns;
- table names and physical column order;
- SQL Server system type information used for SQLite affinity translation;
- recovered nullable flags;
- recovered `IDENTITY` flags.

R14 already contained 150 of those backup-proven tables. R15 adds the remaining 176 with migrations 019 through 029. The resulting SQLite schema contains 332 user tables: 326 backup-proven tables plus 6 explicitly source-derived compatibility tables from earlier checkpoints.

The six source-derived tables are:

- `QuestDB__RepeatedQuest`;
- `SecurityDB__AgingRecord`;
- `SecurityDB__BannedIP`;
- `SecurityDB__BannedMac`;
- `SecurityDB__ClassCheatList`;
- `StartUserDB__ExtraItems`.

## Migrations added

- `019_billinglogdb_catalog.sql` — 6 tables
- `020_gamelogdb_catalog.sql` — 17 tables
- `021_itemlogdb_catalog.sql` — 3 tables
- `022_logdb_catalog.sql` — 5 tables
- `023_pcroom_catalog.sql` — 3 tables
- `024_pcroomlog_catalog.sql` — 19 tables
- `025_rptdb_catalog.sql` — 1 table
- `026_sod2db_catalog.sql` — 9 tables
- `027_soddb_catalog.sql` — 69 tables
- `028_clandb_catalog.sql` — 24 tables
- `029_remaining_backup_catalogs.sql` — 20 tables from BattlePass, CoinShopDB, PristonDB and SecurityDB

Total R15 additions: 176 tables.

## Schema result

- `PRAGMA user_version = 29`
- user tables = 332
- backup-proven table coverage = 326 / 326
- backup catalog columns = 5,568

The new catalog validator checks the 176 newly materialized tables for exact recovered column order, SQLite affinity translation, recovered nullability and `IDENTITY` translation. It also asserts that all 326 backup-proven physical tables are present in a clean database built only from migrations.

## Validation results

All available non-Windows validations passed:

- backup catalog completion: PASS — 176 newly materialized tables, 326/326 backup tables present;
- exact new column order / affinity / nullability / identity: PASS;
- BillingDB catalog: PASS — 43/43 tables, exact order and identity translation;
- SoD2 SQLite contract: PASS — index 1 codes 0..6 and index 3 protocol/ranking;
- migrations: PASS — version 29 / 332 user tables / idempotence;
- SQL query coverage: PASS — missing 0, forbidden external/positional 0;
- clan SQLite contract: PASS;
- SQLite transaction API: PASS;
- clan mark local storage: PASS;
- clan tag local storage: PASS;
- SQLite local/free login: PASS;
- notice SQLite contract: PASS;
- source graph: PASS — only authorized new server TU remains `SQLiteConnection.cpp`;
- frozen build/link scripts: PASS;
- port structure: PASS — 1517/1517;
- compiler adaptation ledger: PASS.

Raw validator output is stored in `docs/R15_VALIDATION_RAW.txt`.

## Patch reproducibility

`migration/sqlite_wip_r14_to_r15.patch` was applied to a clean extraction of R14 and the resulting source was compared to the R15 tree by path and SHA-256.

Result:

- expected files: 2032
- actual files: 2032
- missing: 0
- extra: 0
- hash mismatches: 0
- result: PASS

See `docs/R15_PATCH_APPLY.txt` and `migration/SQLITE_WIP_R15_CHANGED_FILES.tsv`.

## What R15 intentionally does not prove

R15 is schema completion, not runtime promotion. In particular it does not prove:

- Windows compile/link of the current SQLite server tree;
- startup/runtime against a clean SQLite file;
- import of historical row data from the SQL Server backups;
- equivalence of paid billing / PC-room / charging behavior from `Database.dll`;
- removal of the remaining legacy DLL/ODBC/WinInet dependencies;
- full end-to-end gameplay persistence regression.

R6.1 therefore remains the functional regression oracle until the current tree passes compile/link/runtime validation in the target Windows environment.

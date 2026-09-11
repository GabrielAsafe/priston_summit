# RETOMADA — SQLite WIP R14

Data: 2026-09-06

## Baseline contract

- R6.1 remains the last compile/link/runtime-confirmed functional baseline.
- R14 is experimental SQLite migration work; do not use it as gameplay oracle until user validation.
- R13 is the frozen predecessor for this delta.

## R14 delta from R13

R14 is deliberately a **BillingDB schema-only** checkpoint. It does not widen paid/PC-room authentication semantics.

The SQL Server catalog was recovered directly from `BillingDb202405120913.bak`. Migration `018_billingdb_catalog.sql` materializes all 43 recovered BillingDB tables under the physical `BillingDB__*` namespace.

Recovered catalog scope:

- 28 routing tables: `0CHANNEL`, `9CHANNEL`, `ACHANNEL` through `ZCHANNEL`, each with the exact recovered order `PRISTONTALE_ID, TABLE_NAME, INSERT_DATE`;
- `BillingPointCheck20060329`;
- `Excite` plus `Excite_Amount`, `_Bak`, `_Error`, `_Free`, `_NoUse`;
- `NetMarble` plus the same five history/side tables;
- `NTTBilling`;
- `Table1`.

Column order and SQLite affinities are generated from the recovered SQL Server catalog. SQL Server `IDENTITY` columns are represented as SQLite `INTEGER PRIMARY KEY AUTOINCREMENT`, consistent with the existing migration strategy. No provider/login state machine has been inferred from this schema.

## Schema

- `PRAGMA user_version = 18`
- 156 user tables
- BillingDB contribution: 43 tables

`SQLiteConnection.cpp` bootstrap now includes `018_billingdb_catalog.sql`.

## Login policy remains conservative

The SQLite-first login path introduced earlier is unchanged in R14:

- only the proven simple/free-account case is accepted directly from SQLite;
- billing/paid, PC-room, blocked, expired, missing, password-mismatch or otherwise ambiguous rows remain fallback-only to the historical `Database.dll` path when available;
- without the legacy DLL, ambiguous/billing cases fail closed.

Having the BillingDB schema available does **not** by itself prove the `SQLLogon` billing return-code/state semantics, so R14 intentionally stops at schema reconstruction.

## Validation

New validator: `validate_billingdb_sqlite_contract.py`.

It validates all 43 BillingDB physical tables against a self-contained column-order oracle generated from the recovered `.bak` catalog, and checks translation of recovered identity columns.

Results:

- BillingDB catalog: 43/43 tables exact column order PASS;
- migrations: version 18 / 156 user tables PASS;
- SoD2 contract PASS;
- SQL query coverage PASS;
- clan SQLite contract PASS;
- transaction API PASS;
- clan mark/tag local storage PASS;
- SQLite local/free login PASS;
- notice contract PASS;
- source graph PASS, only authorized new server TU remains `SQLiteConnection.cpp`;
- build/link scripts frozen PASS;
- port structure 1517/1517 PASS;
- compiler adaptation PASS;
- R13 -> R14 patch clean-apply/hash comparison PASS (2020/2020 files).

See `docs/SQLITE_WIP_R14_VALIDATION.md`, `docs/R14_VALIDATION_RAW.txt`, and `docs/R14_PATCH_APPLY.txt`.

## Audit artifact

`migration/BILLINGDB_BACKUP_CATALOG_R14.tsv` records the table/column catalog recovered from the supplied BillingDb backup, including source SQL type, translated SQLite affinity, identity flag, and recovered nullable flag.

## Next priorities

1. Reconstruct the BillingDB `<initial>CHANNEL -> provider table` lookup behavior from legacy code/data before widening local login.
2. Do not guess `Database.dll` return codes or paid/PC-room expiration/charging semantics.
3. Continue web removal only where parser/source/backups prove the observable contract.
4. Keep R6.1 as the regression oracle until Windows compile/link/runtime validation succeeds.

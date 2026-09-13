# SQLite WIP R14 — validation report

Date: 2026-09-06

R14 adds the backup-proven BillingDB catalog as migration 018 without changing paid/PC-room login behavior.

## Schema/catalog

- `validate_billingdb_sqlite_contract.py`: PASS
  - 43 BillingDB tables present;
  - exact recovered column order checked for every table;
  - SQL Server IDENTITY -> SQLite integer primary-key/autoincrement translation checked.
- `validate_sqlite_migrations.py`: PASS
  - `PRAGMA user_version = 18`;
  - 156 user tables;
  - existing migration contracts and idempotence remain PASS.

## Existing functional/structural validators

- SoD2 SQLite contract: PASS
- SQL query coverage: PASS (`missing=0`, `forbidden_external_or_positional=0`)
- clan SQLite contract: PASS
- SQLite transaction API: PASS
- clan mark local storage: PASS
- clan tag local storage: PASS
- SQLite local/free login: PASS
- notice SQLite contract: PASS
- source graph: PASS; only authorized new server TU is `SQLiteConnection.cpp`
- build/link scripts frozen: PASS
- port structure: PASS (`1517/1517`)
- compiler adaptation: PASS

Raw output is in `R14_VALIDATION_RAW.txt`.

## Delta reproducibility

The generated R13 -> R14 patch was applied to a clean R13 copy. The patched tree was compared byte-for-byte by SHA-256 with R14:

- expected files: 2020
- actual files: 2020
- missing: 0
- extra: 0
- hash mismatches: 0
- RESULT: PASS

See `R14_PATCH_APPLY.txt`.

## Not validated here

No native Windows/i686 MinGW compile, link, server startup, login runtime, or persistence runtime was performed in this environment. R6.1 remains the last user-confirmed functional baseline.

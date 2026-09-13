# SQLite WIP R25 — validation

Date: 2026-09-11

R25 is an operational-safety revision on top of R24. It does not change C++
runtime behavior or the SQLite schema. The database remains at migrations
`001..032`, `user_version=32`, with 338 user tables.

Controlled suite: **40 / 40 PASS**. Raw output:
`docs/R25_VALIDATION_RAW.txt`.

New R25 proof covers:

- consistent SQLite online backup using `sqlite3_backup` rather than raw
  database/WAL file copying;
- source and backup integrity checks, equal `user_version` and table count;
- overwrite and same-source guards;
- `prepare-local-only` refuses catalog blockers and active sessions before
  creating the rollback snapshot;
- canonical cross-platform controlled-suite runner, including parallel mode;
- a non-mutating Windows preflight contract;
- a two-phase Windows runtime checklist ending with a strong test where
  `Database.dll` is temporarily unavailable while `legacy_billing_fallback=0`.

All 38 R24 validators continue to pass, plus the R25 backup/cutover validator
and Windows preflight-kit validator.

The PowerShell preflight is statically validated here but cannot be executed as
Windows proof in this environment. The actual Windows compile/link/startup and
runtime smoke remain mandatory before promotion over R6.1.

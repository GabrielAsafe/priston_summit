# SQLite WIP R24 — validation

Date: 2026-09-11

R24 is an operational-hardening revision on top of R23. It does not change the
schema or C++ runtime behavior. SQLite remains `user_version=32` with 338 user
tables.

Controlled suite: **38 / 38 PASS**. Raw output: `docs/R24_VALIDATION_RAW.txt`.

New R24 proof:

- `sqlite_local_admin.py audit-local-only` is read-only;
- explicit-local accounts with stale historical billing data are accepted as
  locally owned;
- explicit-legacy and implicit billing-dependent accounts are reported as
  blockers;
- implicit zero-billing R19 accounts remain compatible;
- duplicate case-insensitive account identities and orphan policy/entitlement/
  GM rows are detected;
- `PRAGMA quick_check` and `foreign_key_check` are included;
- command returns exit code 0 only when the catalog is ready for local-only use.

All 37 R23 validators continue to pass.

Windows compile/link/runtime proof is still mandatory before promoting the WIP
over the R6.1 functional baseline.

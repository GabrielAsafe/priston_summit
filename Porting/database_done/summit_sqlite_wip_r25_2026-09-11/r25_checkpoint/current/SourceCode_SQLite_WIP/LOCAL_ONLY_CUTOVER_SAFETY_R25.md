# Local-only cutover safety — R25

Date: 2026-09-11

R25 does not change schema or C++ runtime behavior. It hardens the operational
boundary around the R23 local-only mode.

## Consistent backup

`tools/sqlite_local_admin.py backup [destination]` uses Python SQLite's online
backup API instead of copying `summit.sqlite3`/`-wal` files independently. The
result is checked before success is reported:

- source `PRAGMA quick_check`;
- source `PRAGMA foreign_key_check`;
- backup `quick_check` and `foreign_key_check`;
- equal `user_version`;
- equal user-table count.

Existing backup files are not overwritten unless `--overwrite` is explicit.

## Guarded preparation

`prepare-local-only [destination]` combines the R24 readiness audit with two
additional cutover requirements:

- zero catalog/integrity blockers;
- zero active account sessions.

Only then is the verified rollback snapshot created. The command is deliberately
**non-mutating with respect to runtime policy**: it does not switch
`legacy_billing_fallback` off. That remains an explicit separate administrator
action followed by server restart.

## Reproducible validation

`tools/run_controlled_validation.py` is the canonical cross-platform runner for
the controlled WIP validator set. `--jobs N` allows parallel execution while
preserving deterministic report order; `--output FILE` records the full result.

`tools/windows_sqlite_preflight.ps1` executes the suite plus SQLite readiness and
backup checks before the Windows runtime smoke. It does not start the server or
change provider policy.

Schema remains `user_version=32` with 338 user tables.

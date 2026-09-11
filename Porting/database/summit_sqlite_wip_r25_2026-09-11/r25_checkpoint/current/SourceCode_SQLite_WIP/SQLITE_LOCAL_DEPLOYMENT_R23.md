# SQLite local deployment — R23

Date: 2026-09-11

R23 includes `tools/sqlite_local_admin.py` so the local auth layer can be
operated without hand-editing the 26 per-letter AccountDB tables.

## Preconditions

1. Keep a backup of `Database/summit.sqlite3`.
2. Run an R23 server build once so migrations reach `user_version=32`, or apply
   the exact migration set through `032_local_auth_policy.sql` by the existing
   migration runner.
3. Prefer doing account/policy maintenance while the server is stopped. SQLite
   WAL permits concurrent access, but operational account changes are easier to
   reason about at a maintenance boundary.

The tool refuses databases older than v32 and never runs migrations itself.

## Typical commands

From the repository/server root:

```text
python tools/sqlite_local_admin.py status
python tools/sqlite_local_admin.py create-account LocalUser
python tools/sqlite_local_admin.py show LocalUser
python tools/sqlite_local_admin.py set-auth LocalUser local
python tools/sqlite_local_admin.py grant-pcbang LocalUser
python tools/sqlite_local_admin.py set-gm LocalUser --security 4
```

Passwords are prompted twice when `--password` is omitted. `--password` exists
for controlled automation but exposes the value to shell history/process
inspection and is not recommended for normal administration.

Security levels used by the server are:

- `1` VIP;
- `2` moderator;
- `3` game master;
- `4` administrator.

An optional entitlement expiry can be supplied with, for example:

```text
python tools/sqlite_local_admin.py grant-pcbang LocalUser --expires "2026-12-31 23:59:59"
```

To remove that gameplay entitlement:

```text
python tools/sqlite_local_admin.py revoke-pcbang LocalUser
```

## Converting an old account to local ownership

An account may contain historical `GPCode`, `RNo`, `BNum`, `Channel`,
`UsePeriod` or `DisuseDay`. Do **not** fabricate zero billing rows just to make
it local. Instead:

```text
python tools/sqlite_local_admin.py set-auth ExistingUser local
```

The stored historical fields are retained, while login runtime outputs are
neutralized for the local session.

To hand the account back to the provider:

```text
python tools/sqlite_local_admin.py set-auth ExistingUser legacy
```

To remove explicit ownership and return to the conservative R19 auto-detection:

```text
python tools/sqlite_local_admin.py set-auth ExistingUser implicit
```

## Disabling Database.dll

Only after all accounts that must work locally are created/converted and tested:

```text
python tools/sqlite_local_admin.py set-billing-fallback off
```

Then restart the server. `Database.dll` will not be loaded. A legacy-owned or
ambiguous account will fail authentication instead of silently bypassing local
policy.

To restore compatibility:

```text
python tools/sqlite_local_admin.py set-billing-fallback on
```

Restart again after changing this runtime switch.

## GameMasters compatibility fix

R23 also fixes `GameMasters::readFromDatabase()`: `active` is stored as INTEGER
in SQLite, so the server now binds integer `1` instead of the SQL Server-style
string `"True"`. The query also projects the five expected columns explicitly
instead of relying on `SELECT *` column order.

## Still required before promotion

R6.1 remains the functional oracle. R23 still needs a real Windows build/link
and runtime smoke suite before it can replace that baseline: startup, account
creation, local login/logout/reconnect, duplicate-session rejection,
crash/restart recovery, GM login, character/inventory/warehouse/quest/coin-shop/
clan persistence, and the four PCBang gameplay benefits.

## R24 readiness gate

Before switching the provider off, run:

```text
python tools/sqlite_local_admin.py audit-local-only --details
```

A zero exit status means no explicit-legacy or implicit billing-dependent
account remains and the basic SQLite integrity/orphan checks are clean. The
audit is read-only; it never bulk-converts account ownership.

## R25 backup and Windows preflight

Before a local-only cutover, create a consistent verified snapshot rather than
copying SQLite/WAL files independently:

```text
python tools/sqlite_local_admin.py backup Database/backups/manual.sqlite3
```

For the guarded pre-cutover check (readiness + no active sessions + backup):

```text
python tools/sqlite_local_admin.py prepare-local-only Database/backups/pre_local_only.sqlite3
```

This command does not change `legacy_billing_fallback`. On Windows, the bundled
`tools/windows_sqlite_preflight.ps1` runs the full controlled validator suite,
status/readiness checks and a rollback backup before runtime testing. See
`WINDOWS_RUNTIME_SMOKE_R25.md` for the two-phase fallback-on/local-only smoke.

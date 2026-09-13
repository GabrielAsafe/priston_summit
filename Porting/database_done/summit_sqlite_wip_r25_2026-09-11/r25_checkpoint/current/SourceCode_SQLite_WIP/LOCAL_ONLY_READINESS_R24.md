# Local-only readiness audit — R24

Date: 2026-09-11

R24 adds a **read-only** operational gate before disabling `Database.dll`:

```text
python tools/sqlite_local_admin.py audit-local-only --details
```

Exit code `0` means the current account catalog is ready for the R23
SQLite-only runtime. Exit code `3` means migration blockers remain.

The audit distinguishes:

- explicit `local` accounts (safe even if historical billing columns remain);
- explicit `legacy` accounts (block local-only deployment);
- implicit accounts with the proven R19 zero-billing signature (safe);
- implicit accounts carrying billing/PC-room identity (blockers);
- active sessions (reported for maintenance planning);
- case-insensitive duplicate accounts;
- orphan auth policies, gameplay entitlements and active GM rows;
- SQLite `PRAGMA quick_check` and `foreign_key_check` results.

The command never changes account ownership. Resolve each blocker explicitly
with `set-auth <user> local`, retain it as `legacy`, or correct the underlying
catalog before disabling `legacy_billing_fallback`.

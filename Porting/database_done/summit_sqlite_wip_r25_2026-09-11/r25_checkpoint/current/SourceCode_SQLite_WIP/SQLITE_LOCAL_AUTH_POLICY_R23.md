# SQLite local authentication policy — R23

Date: 2026-09-11

## Purpose

R23 closes the ownership gap between the conservative R19 local/free login and
the historical `Database.dll` billing provider.  It does **not** reconstruct the
old commercial billing engine.  It adds an explicit, reversible policy saying
which provider owns authentication for a given account and whether the legacy
provider may be loaded at all.

## Schema

Migration `032_local_auth_policy.sql` adds:

- `LocalAuth__AccountPolicy(UserID, AuthMode, Enabled, UpdatedAt)`;
- `LocalAuth__RuntimePolicy(PolicyKey, PolicyValue, UpdatedAt)`.

`UserID` is `NOCASE`. `AuthMode` is constrained to `local` or `legacy`.
`legacy_billing_fallback` is seeded to `1`, so upgrading from R22 preserves the
old compatibility behavior until an administrator explicitly disables it.

R23 schema: `user_version=32`, 338 user tables.

## Authentication ownership

There are three effective states:

| Account state | Login owner | Historical billing columns |
| --- | --- | --- |
| no enabled policy | conservative R19 auto-local only when the proven zero-billing signature matches; otherwise fallback | still authoritative for deciding whether implicit local auth is safe |
| `AuthMode=local` | SQLite is authoritative | retained for history but ignored for runtime GPCode/RNo/period/reservation outputs |
| `AuthMode=legacy` | legacy provider | unchanged |

For an explicit local account, wrong password, blocked/deleted state and an
already-active session are **local rejections**. They no longer fall through to
a provider that could override the local decision.

## Session ownership

Explicit local ownership also applies to logout. Once the account policy is
`local`, a stale/wrong session GUID or local write error is handled locally and
does not fall through to `SQLLogout`. This prevents `Database.dll` from
mutating a session it no longer owns.

Startup/exit stale-session cleanup includes explicit local accounts even if
historical GPCode/RNo/BNum/Channel values remain in the AccountDB row.

## SQLite-only runtime

`LocalAuth__RuntimePolicy` contains:

```text
legacy_billing_fallback = 1
```

Set it to `0` and restart the server to enter local-only mode. In that mode:

- `Clan.dll` may still be loaded independently;
- `Database.dll` is **not loaded**;
- local/explicit-local login and logout continue through SQLite;
- rows that are not accepted/owned by SQLite fail login deterministically;
- provider-specific calls remain pointer-gated and therefore cannot dereference
  absent `Database.dll` exports;
- local audit/SoD/Clan/Siege paths migrated in earlier revisions continue to use
  SQLite.

This switch is read when the SQL/log worker starts, so changing it requires a
server restart.

## What R23 deliberately does not recreate

The supplied audited `Database.dll` did not export several optional historical
features (`UserStar`, `Contribution`, `PollRecord`, `EventJoin`, `IPRecord`,
etc.). R23 does not invent their unknown commercial/gameplay semantics merely
to obtain a no-DLL deployment. They stay compatibility-only/dead in the
observed supplied runtime.

The paid/PC-room provider path is still available when
`legacy_billing_fallback=1` and an account is not explicitly local. This keeps
R23 reversible until Windows runtime verification is complete.

# Windows runtime smoke — R25

Date: 2026-09-11

This is the remaining promotion gate for the SQLite WIP. Static/Linux-hosted
validation does **not** replace this run. R6.1 remains the functional oracle
until the steps below pass on the real Windows server environment.

## 0. Stop and preserve

Stop the server before the cutover phase. Keep the R6.1 binary/configuration and
do not overwrite the only copy of the live database.

Run the non-mutating preflight from PowerShell at the repository/server root:

```powershell
powershell -ExecutionPolicy Bypass -File tools\windows_sqlite_preflight.ps1
```

The script runs the controlled validator suite, `status`, the R24 readiness
audit, and creates a verified SQLite online-backup snapshot. It does **not**
change `legacy_billing_fallback`.

## 1. Build/link proof

Build the R25 source with the actual Windows target/toolchain and link the real
server executable. Archive:

- compiler/linker command or IDE configuration;
- complete build log;
- SHA-256 of `Server.exe`;
- SHA-256 of `Database/summit.sqlite3` before runtime testing.

A non-Windows `clang-cl` invocation without the Windows SDK is not acceptable as
this proof.

## 2. Provider-compatible smoke (`legacy_billing_fallback=1`)

Keep the default fallback enabled for the first run. Start the server and prove:

1. startup/migrations reach `user_version=32` without schema errors;
2. explicit-local account login succeeds;
3. wrong local password fails locally and cannot be accepted by the provider;
4. duplicate simultaneous login is rejected;
5. logout then reconnect succeeds;
6. forced process termination followed by restart clears the stale local session;
7. active GM row is loaded with the expected security level;
8. character, inventory, warehouse, quest, coin-shop, clan and siege persistence
   survive reconnect/restart;
9. local audit sinks receive character/item/trade/post and SoD events;
10. the four PCBang effects remain identical to the R6.1 oracle when entitlement
    is enabled: +50 drop accumulator, +20% solo EXP, +20% party EXP and PCBang Pet;
11. disabling/revoking the entitlement removes those effects without creating a
    billing/PC-room identity.

If legacy/paid compatibility is still required, also test one explicitly
`legacy` account in this phase.

## 3. Prepare local-only cutover

With no active sessions:

```powershell
python tools\sqlite_local_admin.py audit-local-only --details
python tools\sqlite_local_admin.py prepare-local-only Database\backups\pre_local_only.sqlite3
python tools\sqlite_local_admin.py set-billing-fallback off
```

`prepare-local-only` refuses catalog blockers or active sessions and creates a
verified rollback snapshot. The snapshot is taken **before** the runtime policy
is changed, so restoring it also restores the pre-cutover policy state.

## 4. Strong Database.dll independence proof

Stop the server. Keep a reversible copy/rename of the provider DLL, for example:

```powershell
Rename-Item Database.dll Database.dll.disabled
```

Restart the R25 server with `legacy_billing_fallback=0`. The server must start
without `Database.dll` present. Repeat local login/logout/reconnect, duplicate
session, GM, persistence and entitlement smoke checks.

A local-only server that fails merely because `Database.dll` was renamed does
not pass R25 promotion.

After the test, restore the filename if legacy fallback will remain available:

```powershell
Rename-Item Database.dll.disabled Database.dll
```

## 5. Crash/recovery proof in local-only mode

While a local test account is connected, terminate the server process without a
normal logout. Restart it and verify the account can reconnect after the R19
stale-session cleanup. Confirm another server's/session's ownership is not
incorrectly cleared if a multi-server setup is being tested.

## 6. Acceptance and rollback

Promote the WIP only if all runtime observations match R6.1 except for the
explicitly intended SQLite/local-only changes.

On any unexpected behavior:

1. stop the server;
2. preserve the failed-run database/logs for diagnosis;
3. restore the verified pre-cutover SQLite backup;
4. restore `Database.dll` if it was renamed;
5. return to the R6.1 functional baseline or R25 with fallback enabled.

Do not repair a failed smoke test by inventing fake `Bl_RNo`, billing rows, or
provider identities. Those would defeat the local entitlement/auth boundaries
established in R20-R23.

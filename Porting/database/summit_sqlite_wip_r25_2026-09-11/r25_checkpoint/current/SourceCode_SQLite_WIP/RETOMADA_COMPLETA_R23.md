# RETOMADA COMPLETA — Summit / SQLite WIP R23

Data: 2026-09-11

R23 continues R22. **R6.1 remains the proven functional baseline** until a real
Windows build/runtime checkpoint is completed.

## State now

- shared SQLite file: `Database/summit.sqlite3`;
- migrations `001..032`, `user_version=32`, 338 user tables;
- free/implicit R19 login/session path retained;
- explicit per-account auth ownership: `local` or `legacy`;
- explicit local accounts ignore stale historical billing identity at runtime;
- local logout is authoritative and cannot fall through to Database.dll;
- local PCBang gameplay entitlement remains independent from billing identity;
- character/item/SoD/Clan/Siege migrated local-first as in R22;
- `legacy_billing_fallback` is seeded to `1` for compatibility;
- when set to `0` and the server is restarted, `Database.dll` is not loaded;
- `tools/sqlite_local_admin.py` manages local accounts, ownership, passwords,
  PCBang entitlement, GM access and the runtime fallback switch;
- GameMasters now binds `active` as INTEGER `1` under SQLite.

## Validation

Controlled suite: **37/37 PASS**. See `SQLITE_WIP_R23_VALIDATION.md` and
`docs/R23_VALIDATION_RAW.txt`.

## Remaining promotion blocker

Compile/link/startup and runtime smoke testing on the real Windows target:
local login/logout/reconnect, duplicate login, crash/restart recovery, GM,
character/inventory/warehouse/quest/coin-shop/clan persistence and the four
PCBang gameplay benefits.

Once Windows validation is green, the compatibility default may be flipped to
SQLite-only and the unreachable billing/provider code can be deleted in a
separate cleanup revision.

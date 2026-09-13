# SQLite WIP R21 — validation

Date: 2026-09-11

## Result

R21 adds migration 030 and a real server-local gameplay entitlement source.
SQLite is now `user_version=30` with 333 user tables: the 332 reconstructed
legacy/catalog tables plus one server-owned local entitlement table.

Controlled regression suite: **29/29 PASS**.

Key proofs:

- fresh migration set 001..030 is idempotent;
- v29 -> v30 upgrade preserves an existing AccountDB row;
- entitlement UserID/key identity is case-insensitive and unique;
- Enabled accepts only 0/1;
- disabled and expired grants do not activate;
- non-expiring and future-expiring grants activate;
- login loads the grant once into a per-slot local cache;
- slot allocation and both disconnect paths clear the cache;
- gameplay uses `Bl_RNo > 0 OR local entitlement`;
- raw billing/reservation paths still use legacy `Bl_RNo` semantics;
- R19 local login/session gates remain green;
- R18 Clan/Siege local-first gates remain green;
- query coverage and transaction API remain green.

Raw output: `docs/R21_VALIDATION_RAW.txt`.
Inherited frozen-validator notes: `docs/R21_PREEXISTING_VALIDATOR_LIMITATIONS.txt`.

## Mandatory before promotion

Static proof is not a Windows target proof. R21 still requires the real Windows
compile/link and runtime smoke tests, especially:

- server startup and migration 29->30 against a copied real database;
- free SQLite login/logout/reconnect and duplicate-session rejection;
- local entitlement disabled/enabled/expired/future-expiry behavior;
- solo/party EXP, drop and PCBang Pet behavior;
- paid/PC-room legacy fallback if that provider will remain supported;
- character/inventory/warehouse/quest/coin-shop/clan smoke tests.

R6.1 remains the functional oracle until those checks pass.

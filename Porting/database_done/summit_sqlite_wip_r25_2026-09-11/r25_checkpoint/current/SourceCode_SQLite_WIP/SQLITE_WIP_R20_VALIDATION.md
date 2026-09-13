# SQLite WIP R20 — validation

Date: 2026-09-11

## Result

R20 is a behavior-preserving semantic refactor around the PC-room gameplay
benefit. No schema migration was added; SQLite remains `user_version=29` with
332 user tables.

The scope-controlled R20 suite passes 28 validators. It includes all R19 gates
used for SQLite/login/session/gameplay-persistence/build structure plus the new
`validate_gameplay_entitlement_boundary.py`.

The new validator proves:

- the entitlement facade still means exactly `Bl_RNo > 0`;
- drop `+50`, solo EXP `+20`, party EXP `+20`, and PCBang Pet use the facade;
- billing/session/reservation paths retain raw `Bl_RNo` semantics;
- no raw `Bl_RNo > 0` check remains in `PartySever.cpp` gameplay logic.

Raw output: `docs/R20_VALIDATION_RAW.txt`.

## Inherited validator-harness limitations

Two older structural validators fail on the untouched R19 ZIP as well and are
not R20 regression gates:

- `validate_active_nonbaseline_graph.py` has a stale allowlist for existing
  `SqliteQueryCompat.h/.inl`;
- `validate_active_shared_graph.py` has a stale frozen-map entry and raises a
  `KeyError` for existing `Database_sqlite_server.h`.

Reproduction note: `docs/R20_PREEXISTING_VALIDATOR_LIMITATIONS.txt`.

## Still mandatory before promotion

Static validation is not a substitute for the Windows target. R20 still needs:

- compile and link with the real Windows toolchain;
- server startup;
- free SQLite login/logout/reconnect and duplicate-session test;
- crash/restart stale-session recovery;
- character/inventory/warehouse/quest/coin-shop/clan runtime smoke tests;
- verification that solo/party EXP, drop and PCBang Pet behavior is unchanged;
- verification of the remaining paid/PC-room fallback if that provider is kept.

R6.1 remains the functional oracle until those checks are completed.

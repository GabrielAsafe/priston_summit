# SQLite WIP R22 — validation

Date: 2026-09-11

## Result

Controlled R22 suite: **31/31 PASS**.

R22 adds two focused validators:

- `validate_sod_record_local_sink.py`
- `validate_local_audit_sinks.py`

The suite also re-runs all R21 gates for reconstructed backup catalogs,
BillingDB, build source graph, clan storage, Database.dll gameplay persistence,
gameplay entitlement, local login/session, migrations, query coverage,
transactions and structural parity helpers.

## R22-specific proof

The validators prove:

- SoD result writes are SQLite-first from the existing async event worker;
- `Database.dll!WithSOD2RecordRanks` remains fallback-only;
- migration 031 is registered in the runtime migration runner;
- schema reaches `user_version=31` and 336 user tables;
- character new/delete/logout audit preserves the local event payload,
  including 64-bit EXP;
- item/trade/gift events persist header + entries transactionally;
- item logging no longer requires DLL exports merely to enqueue events;
- v30 -> v31 upgrade preserves pre-existing local entitlement and CoinShop
  data;
- R21 login/session/entitlement and R18 Clan/Siege contracts remain green.

Raw controlled output: `docs/R22_VALIDATION_RAW.txt`.

## Inherited broad-validator limitations

The same four broad/frozen validators remain unsuitable as WIP promotion gates:
`validate_active_nonbaseline_graph.py`, `validate_active_shared_graph.py`,
`validate_full_source_parity.py`, and `validate_oracle_parity.py`. Their stale
allowlists/frozen parity assumptions were already documented before R22; R22
continues to use the scope-controlled suite for intentional WIP changes.

## Still mandatory before promotion

Static validation is not a Windows runtime proof. R22 still requires the real
Windows target to verify compile/link, server startup, login/logout/reconnect,
crash recovery, SoD result persistence, character audit worker, item/trade/gift
audit worker, and the existing gameplay/persistence smoke tests.

R6.1 remains the functional oracle until that proof is complete.

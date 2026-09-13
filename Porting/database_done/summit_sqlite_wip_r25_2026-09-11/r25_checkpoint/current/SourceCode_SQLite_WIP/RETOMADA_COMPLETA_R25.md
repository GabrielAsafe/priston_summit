# RETOMADA COMPLETA — Summit / SQLite WIP R25

Data: 2026-09-11

R25 continues the verified R24 checkpoint. Runtime/schema contracts remain the
same: migrations `001..032`, `user_version=32`, 338 user tables, explicit
`local`/`legacy` account ownership, local PCBang gameplay entitlement and
opt-in `legacy_billing_fallback=0` mode.

## What R25 closes

R25 closes the main **operational gap** before a Windows local-only test:

1. `sqlite_local_admin.py backup` creates a consistent SQLite snapshot with the
   online backup API and verifies it before reporting success;
2. `prepare-local-only` combines the R24 readiness audit with a zero-active-
   session requirement and creates the verified rollback snapshot;
3. the policy switch remains deliberately separate: preparation does not turn
   `legacy_billing_fallback` off by itself;
4. `tools/run_controlled_validation.py` is now the canonical cross-platform
   controlled-suite runner;
5. `tools/windows_sqlite_preflight.ps1` packages validation, readiness and
   rollback backup into one pre-runtime Windows preflight;
6. `WINDOWS_RUNTIME_SMOKE_R25.md` defines the remaining evidence needed to
   promote the WIP, including startup with `Database.dll` temporarily renamed
   after the provider fallback is disabled.

## Validation

Controlled suite: **40/40 PASS**. See `SQLITE_WIP_R25_VALIDATION.md` and
`docs/R25_VALIDATION_RAW.txt`.

## What is deliberately not changed

- no schema migration after 032;
- no gameplay formula change;
- no change to explicit-local/legacy ownership semantics;
- no default flip of `legacy_billing_fallback`;
- no deletion of optional provider exports before Windows runtime proof.

## Remaining hard blocker

A real Windows compile/link/startup and runtime smoke. R6.1 remains the
functional oracle until that proof passes. R25 now contains the tools and exact
rollback procedure needed to perform that gate reproducibly.

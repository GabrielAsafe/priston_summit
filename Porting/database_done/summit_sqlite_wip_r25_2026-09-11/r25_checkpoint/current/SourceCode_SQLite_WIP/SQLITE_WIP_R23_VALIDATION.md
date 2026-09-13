# SQLite WIP R23 — validation

Date: 2026-09-11

## Result

R23 adds explicit SQLite authentication ownership and an opt-in SQLite-only
runtime switch while preserving the R22 compatibility default.

Controlled suite: **37 / 37 PASS**. Raw output: `docs/R23_VALIDATION_RAW.txt`.

Validated in particular:

- migrations through `032`, `user_version=32`, 338 user tables;
- v31 -> v32 data-preserving upgrade;
- explicit `local` / `legacy` auth ownership and implicit R19 compatibility;
- authoritative local rejection (wrong password, blocked/deleted/in-use);
- authoritative local logout and stale-session recovery;
- `legacy_billing_fallback=0` skips `LoadLibrary("Database")`;
- all remaining legacy provider calls stay pointer-gated;
- GameMasters INTEGER `active=1` query compatibility;
- local admin CLI account/auth/password/entitlement/GM operations;
- previously stale active graph validators now pass.

## Not proven here

This environment does not contain a Windows SDK/toolchain capable of compiling
the target (`windows.h` is unavailable to the installed `clang-cl`). R6.1
therefore remains the functional oracle pending real Windows compile/link,
startup and runtime smoke tests.

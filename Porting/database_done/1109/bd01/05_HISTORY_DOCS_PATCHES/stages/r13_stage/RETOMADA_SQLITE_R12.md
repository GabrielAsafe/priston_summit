# RETOMADA — SQLite WIP R12

Data: 2026-09-06

## Baseline contract

- R6.1 remains the last compile/link/runtime-confirmed functional baseline.
- R12 is experimental SQLite migration work; do not use it as gameplay oracle until user validation.
- Do not repair unrelated legacy gameplay/UB while DB/web migration is in progress.

## R12 delta from R11

R12 starts replacing the active `Database.dll!SQLLogon` dependency without inventing the historical billing engine.

### Local/free AccountDB login

`bSql_LogOn()` now tries SQLite first using the proven per-letter AccountDB layout:

`AccountDB__<A..Z>GameUser`

The query uses named columns:

`Passwd, GPCode, DisuseDay, UsePeriod, inuse, BlockChk, DelChk, RNo, BNum, Channel`

The SQLite path returns success only for a deliberately narrow local/free contract: exact password; no GP code, period, active/block/delete marker, PC-room number, billing number, or channel; and a parseable non-expired disuse date.

Everything outside that contract remains fallback-only to `Database.dll!SQLLogon` when available. If the DLL is absent, unsupported cases fail rather than being approximated.

A successful local login generates a fresh session GUID with `CoCreateGuid`. Empty/zero GPCode is resolved locally by `bSql_GetGPCode()` as GP type 0.

`LoadLogSqlDLL()` now keeps the historical `LOAD_DLL_SQL` dispatch active even when `Database.dll` is missing, allowing the local/free SQLite login path to operate. Other DLL-backed operations remain protected by their existing function-pointer checks.

### Notice

The active server notice path was already SQLite-backed. R12 removes its positional dependency:

`SELECT * FROM NoticeSystem WHERE Activated=1`

became:

`SELECT Message, Type FROM NoticeSystem WHERE Activated=1`

with `GetData` positions updated from 2/3 to 1/2.

The old `ISAOCHECK::ReadNotice` HTTP function has no active server caller found and is retained only for compatibility.

### HTTP caller audit

The R11 inventory incorrectly treated some compiled legacy shims as active. The R12 caller audit distinguishes definitions/queue processors from reachable callers:

- `ISAOCHECK::IsaoCheck` (`login.php/LoginCheck.php/InsertUser.php`): no active server caller found.
- `ISAOCHECK::IsaoCheck_GameExit` (`Login_End.php`): no active server caller found.
- `ISAOCHECK::ReadNotice`: no active server caller found; active notices are `News::msgNotice`/SQLite.
- `GetWavFile` NPCSound HTTP: queue/function exists, but no production caller into `WebDB_Thread::GetWavFile` was found.
- `read_sod2` remains active: server `sinSOD2` calls `sod2INFOindex(...,1/3)`.

See `migration/HTTP_REMAINING_R12.tsv`.

## Schema

No schema change in R12.

- `PRAGMA user_version = 17`
- 113 user tables

## Validation

All available R12 validators PASS, including the new local-login and notice-contract validators. Patch R11 -> R12 was applied to a clean R11 tree and the resulting 2017 files were hash-compared against R12 with zero differences.

See `docs/SQLITE_WIP_R12_VALIDATION.md`.

## Remaining auth limitation

Do not remove the optional `Database.dll` fallback yet. Paid GP codes, channel/PC-room billing, exact password-collation behavior, expiry rejection codes and provider-specific accounting still need reconstruction/proof before they can be represented locally.

## Next safe work

1. Reconstruct the BillingDB `<initial>CHANNEL` routing + billing-table contract used by `SQLLogon`, using the supplied `.bak` schemas and historical behavior, then expand the local login subset incrementally.
2. Prove and migrate `sod2info.asp` index 1/3 serialization using the existing client parser and `SoD2DB` schema.
3. Find/prove the initial clan-mark asset required by `makeClan`; do not synthesize it.
4. Keep leader transfer and warehouse/updateFlag blocked until their exact write/return contracts are proven.
5. Only remove the DLL/WinInet linker/runtime dependencies after the remaining active callers have been eliminated or classified as intentionally optional.

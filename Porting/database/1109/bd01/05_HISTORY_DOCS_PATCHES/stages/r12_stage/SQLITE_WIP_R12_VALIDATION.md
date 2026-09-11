# SQLite WIP R12 — validation report

Date: 2026-09-06

## Scope

R12 is based on the frozen R11 checkpoint. It does not change gameplay and does not change the SQLite schema version. The main change is a conservative local/free authentication path that removes `Database.dll` as an obligatory login dependency for the subset of AccountDB rows whose non-billing semantics can be proven from the current schema/source. Complex billing behavior deliberately remains fallback-only.

## Functional changes in R12

1. `bSql_LogOn` first tries `AccountDB` through `SQLConnection`/SQLite using `<first-letter>GameUser`.
2. The local path is deliberately restricted to rows with:
   - exact password match;
   - empty/zero `GPCode`;
   - `UsePeriod=0`;
   - empty/zero `inuse`, `BlockChk`, and `DelChk`;
   - `RNo=0` and `BNum=0`;
   - empty/zero `Channel`;
   - parseable, non-expired `DisuseDay` (or zero/empty).
3. Any password/collation ambiguity, missing row, billing state, block state, PC-room state, expired date, query failure, or unsupported account-table case is not accepted locally. It falls back to `Database.dll!SQLLogon` when that provider exists.
4. If `Database.dll` is absent, the legacy `LOAD_DLL_SQL` dispatch remains active so proven simple/free accounts can still reach the SQLite login path; unsupported cases fail closed.
5. Local/free successful login generates a per-session GUID with `CoCreateGuid`, returns zero GP/billing fields, and `bSql_GetGPCode` resolves empty/zero GPCode locally as type 0.
6. `News::msgNotice` no longer uses `SELECT *`; it projects `Message, Type` explicitly from `ServerDB__NoticeSystem`.
7. Caller audit reclassified legacy HTTP `IsaoCheck` login/GameExit, `ReadNotice`, and NPCSound `GetWavFile` as having no active/production caller found in the server roots. They are retained for compatibility rather than deleted.

## Schema

No schema migration was added in R12.

- `PRAGMA user_version = 17`
- 113 user tables

## Validation results

- SQLite local/free login contract: PASS
- active notice named-column contract: PASS
- clan mark local storage: PASS
- clan tag local storage / IIS removal: PASS
- SQLite migrations: PASS — `user_version=17`, 113 user tables
- SQLite query coverage: PASS — missing=0, forbidden external/positional=0
- clan SQLite contract: PASS
- SQLite transaction API: PASS
- source graph: PASS — only authorized new server TU remains `SQLiteConnection.cpp`
- build/link scripts frozen: PASS
- port structure: PASS — 1517/1517
- compiler adaptations: PASS — one authorized MinGW adaptation
- R11 -> R12 patch application followed by full-file SHA-256 comparison: PASS (`2017/2017`, zero missing/extra/hash mismatches)

Raw validator output is retained in `docs/R12_VALIDATION_RAW.txt` in the packaged checkpoint.

## Important limitations

R12 is not a full replacement for the historical billing DLL. The exact `SQLLogon` behavior for paid GP codes, PC-room/channel billing, password collation edge cases, expiry return codes, and other provider-specific states is not sufficiently proven. R12 therefore refuses to synthesize those semantics.

The current environment still does not provide the Windows/i686 MinGW dependency setup used by the project, so compile/link/runtime acceptance has not been performed here. R6.1 remains the functional oracle.

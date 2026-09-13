# Database.dll gameplay audit — R18

Date: 2026-09-11

## Purpose

The supplied `Database.dll` is not one feature. The server uses it as a mixed provider for authentication/session state, PC-room/billing accounting, gameplay persistence and audit logging. The simplification target is therefore **not** “delete billing/MMO code”; it is to extract any state that changes gameplay, replace required persistence locally, then delete commercial/accounting machinery that no longer serves the simplified multiplayer target.

Historical SQL Server data migration is explicitly out of scope. The target can start from a clean SQLite DB. User/GM/admin creation queries will be delivered separately after the server path is stable.

## Supplied DLL vs server loader

- `Database.dll` exports 101 symbols.
- The server bridge requests 37 `Database.dll` symbols with `GetProcAddress`.
- Only 21 of those 37 requested symbols exist in the supplied DLL.
- The other 16 are already optional/pointer-guarded in the legacy architecture and cannot be assumed to have been functional in this supplied runtime.

See `migration/DATABASE_DLL_DEPENDENCIES_R18.tsv` for the per-symbol classification.

## Proven gameplay coupling from billing/PC-room state

The source proves that `Bl_RNo` is not merely accounting metadata:

- `Shared/server/legacy_runtime/OnSever_legacy_server.inl:10848`: `Bl_RNo > 0` adds `+50` to the drop-roll accumulator.
- `...:11121`: `Bl_RNo > 0` adds `+20` EXP for solo kill reward calculations.
- `SrcServer/src/core/server/legacy_runtime/PartySever.cpp:567`: the same `+20` EXP is applied to party reward calculations.
- `OnSever_legacy_server.inl:19810`: `Bl_RNo > 0` enables the PCBang Pet path.
- `OnSever_legacy_server.inl:13120+`: `PCUse`, `PCSearchReserv`, `DisuseDay`, `dwPeriodTime` and `dwPeriodTime2` participate in live expiry/warning/disconnect behavior.
- `/time` calls `bSql_PCRNoReservProduct` when `Bl_RNo > 0`.

Therefore PC-room/billing cannot be removed by simply forcing all fields to zero without an explicit gameplay decision. The safe simplification is to model the desired gameplay entitlement separately from the historical charging/accounting system.

`GpType`/`GpCode` were also inspected. In the observed active source, `GpType == 1` and `GpCode` are used by a PC-room IP monitoring thread. No direct EXP/drop bonus was found from `GpType`; the direct gameplay switch is `Bl_RNo`.

## R18 gameplay persistence extracted from Database.dll

### ClanMoney / SiegeMoney

The supplied DLL contains the exact SQL and its x86 control flow was inspected. R18 now performs these operations directly through `SQLConnection`/SQLite first, with the DLL retained only as an infrastructure-failure fallback.

Legacy quirks deliberately preserved:

- clan fund withdrawal can make the balance negative; the DLL does not enforce sufficient funds here;
- `ClanMoney` withdrawal updates by `UserID` without `DelActive`;
- deposit and both `SiegeMoney` mutations use the active-clan predicate;
- operation 1 logs `OperCode='01'` with negative amount;
- operation 2 logs `OperCode='02'` with positive amount;
- the initial balance/IDX lookup is leader-qualified by `UserID + ClanZang`, but mutation is historically by `UserID`.

### SiegeMoneyTax

R18 also migrates `Database.dll!SiegeMoneyTax` local-first. Reverse engineering proves the following contract:

1. total `0` returns failure;
2. clan-mark string must have at least 9 characters;
3. `CL.IDX` is parsed from the last six decimal digits of the clan-mark number;
4. if Tax is zero, siege credit is zero;
5. otherwise the DLL calculates approximately:
   `credit = Total - round(Total / (1 + Tax * 0.01f))`;
6. `CL.SiegeMoney += credit WHERE IDX=<parsed idx>`;
7. a `SiegeMoneyTax` row records the category totals, Total, Tax and server;
8. a `SiegeMoneyLog` row records `UserID='Server'`, `chName='Server'`, `Operator='GS'`, `OperCode='02'`, and the credited amount.

The old SQL schema supplied `RegistDay` through database behavior/defaults not present in the catalog-only reconstruction, so the SQLite adapter explicitly uses `datetime('now','localtime')` for the two log rows. The mutation + logs are transactional in SQLite.

## What still blocks removal of Database.dll

### Required/behavioral

1. **Auth/session fallback**: paid/PC-room/ambiguous accounts still use `SQLLogon` and related functions. The current SQLite local login intentionally accepts only the proven free/local subset.
2. **PC-room gameplay signal**: `Bl_RNo` drives EXP/drop/pet behavior and expiry paths. Before deleting PC functions, decide whether this becomes a normal local entitlement/perk, is globally enabled, or is removed as an intentional gameplay change.
3. **Session cleanup**: `SQLLogout`, `ServerDown`, `ServerExit` still own some legacy inuse/session cleanup behavior.

### Non-blocking for core gameplay if simplified intentionally

- `CharLog`, `CharRecordLog`, item/trade/post logs are audit/history, not authoritative character/item persistence in the paths inspected.
- `WithSOD2RecordRanks` is called as a record sink; its return value is ignored by the SOD event path. The active SoD2 UI path was already migrated locally in R13.
- Several requested exports do not exist in the supplied DLL at all; those paths are already optional/provider-dependent.

## R18 policy going forward

- Do not recreate historical commercial billing unless gameplay truly needs its result.
- Preserve gameplay effects separately and explicitly.
- Prefer a future small local entitlement/session model over reproducing provider/channel charging tables.
- Do not change `Bl_RNo` bonuses yet; first complete the dependency extraction and get Windows runtime parity.
- Historical data import remains out of scope.
- Final simplified DB documentation and user/GM/admin insertion queries come after runtime stabilization.

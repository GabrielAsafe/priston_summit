# RETOMADA — Summit SQLite WIP R18

Date: 2026-09-11

Read this file first after `RETOMADA_COMPLETA_R17.md` / `CONHECIMENTO_CONSOLIDADO_CHAT_R17.md`.

## Baseline rule

R6.1 remains the only compile/link/runtime-confirmed baseline. R18 is WIP and must not be promoted before Windows build/runtime regression.

## User scope decisions after R17

- Historical SQL Server **data import is out of scope**. A clean DB is acceptable.
- After the server works, deliver simple SQL/query recipes to create `user`, `GM` and `admin` accounts.
- Later deliver a practical map/documentation of how to use the SQLite DB.
- Long-term product target is simpler than the old MMO: multiplayer is sufficient; character export may be added later.
- However, do **not** delete billing/MMO subsystems merely by name. First identify any game logic that consumes their state and preserve/replace those effects intentionally.

## R18 objective

Separate responsibilities hidden in `Database.dll` so the DLL can eventually be removed without deleting gameplay by accident.

New evidence source: the user supplied `/mnt/data/Database.dll` (PE export name internally reports SQL.dll). Its exports, embedded SQL and selected x86 paths were inspected directly.

## Database.dll audit result

- DLL exports: 101.
- Server requests from this provider: 37.
- Requested exports actually present: 21.
- Requested exports absent: 16; these are already pointer/provider-optional in the supplied runtime.

Detailed files:

- `docs/DATABASE_DLL_GAMEPLAY_AUDIT_R18.md`
- `migration/DATABASE_DLL_DEPENDENCIES_R18.tsv`

### Billing state that changes gameplay

`Bl_RNo > 0` is proven to:

- add +20% EXP in solo;
- add +20% EXP in party;
- add +50 to a drop-roll accumulator;
- enable PCBang Pet;
- participate in PC-room/time-reservation flows.

Expiry fields `DisuseDay`, `dwPeriodTime`, `dwPeriodTime2` can warn and disconnect users, and `PCUse`/`PCSearchReserv` can refresh/change runtime state.

Do not zero/remove these mechanically. The future simplified design can replace them with explicit local entitlement/session flags after Windows parity is reached or with explicit user approval of gameplay changes.

## Gameplay persistence migrated in R18

File changed:

`SrcServer/src/core/server/legacy_runtime/DllServer.cpp`

### ClanMoney / SiegeMoney

Added `TrySQLiteLocalClanFund` and changed `GAMELOG_SQL_CLANMONEY` to local-first SQLite with Database.dll fallback only on local infrastructure failure.

Behavior recovered from the DLL and deliberately preserved:

- no insufficient-funds guard in this layer;
- leader-qualified initial lookup;
- UserID mutation;
- odd `DelActive` asymmetry;
- exact `01/02` logging sign conventions.

### SiegeMoneyTax

Added `TrySQLiteLocalSiegeMoneyTax` and changed `GAMELOG_SQL_SIEGETAX` to local-first SQLite. `bSql_RecordCurrency` is no longer hard-gated on `BillSql.SiegeMoneyTax`.

Recovered DLL behavior:

- total zero -> failure;
- mark length <9 -> failure;
- clan IDX = last 6 decimal digits of mark;
- tax credit = `Total - round(Total/(1+Tax*0.01f))` for nonzero tax;
- update `CL.SiegeMoney` by IDX;
- insert `SiegeMoneyTax`;
- insert `SiegeMoneyLog` as Server/GS/02.

The SQLite adapter supplies `RegistDay=datetime('now','localtime')` because the modern backup marks it NOT NULL while the historical DLL insert omitted the column.

## New validator

`validate_database_dll_gameplay_contract.py`

It builds the real migrations and verifies:

- ClanMoney can go negative like the DLL;
- `01/02` log signs;
- `DelActive` asymmetry;
- SiegeMoney operations;
- SiegeMoneyTax mark->IDX mapping;
- tax-credit calculation;
- tax + siege audit rows;
- invalid mark/zero-total no-mutation cases.

## R18 validation status

After the latest changes the following all PASS in this environment:

- backup catalog completion: 326/326;
- BillingDB contract: 43 tables;
- SoD2 contract;
- migrations v29 / 332 user tables;
- SQL query coverage missing=0;
- clan SQLite contract;
- SQLite transactions;
- clan mark/tag local storage;
- local/free login;
- notice;
- HTTP surface: only `clanChipChange.asp` remains;
- Database.dll gameplay contract including ClanMoney/SiegeMoney/SiegeMoneyTax;
- source graph: only `SQLiteConnection.cpp` new server TU;
- frozen build scripts;
- port structure 1517/1517;
- compiler adaptation ledger.

These are static/contract tests, not Windows executable proof.

## Remaining priority work

1. Freeze R18 as a new checkpoint; do not edit it in place afterward.
2. Continue Database.dll dismantling by responsibility:
   - session/logout/startup cleanup;
   - PC-room gameplay entitlement vs commercial accounting;
   - optional logging sinks.
3. Do not remove `SQLLogon` fallback until local auth/session policy is fully defined.
4. Resolve `clanChipChange.asp` separately; still the only active server HTTP route and its exact codes/writes remain unproven.
5. Only after zero active callers: remove WinInet / Database.dll / ODBC in that order with Windows link tests.
6. Run complete Windows regression against R6.1.

## Out of scope

- importing historical SQL Server data;
- fixing unrelated gameplay/loader/UB bugs;
- character export implementation (future);
- DB user guide/map (after runtime stabilization, as requested);
- changing PC-room bonus balance before explicit decision/parity proof.

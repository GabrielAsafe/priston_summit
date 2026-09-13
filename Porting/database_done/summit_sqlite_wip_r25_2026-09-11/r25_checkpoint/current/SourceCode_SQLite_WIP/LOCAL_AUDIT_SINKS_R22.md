# Local audit sinks — R22

Date: 2026-09-11

## Goal

Reduce non-authoritative `Database.dll` dependencies without changing gameplay,
authentication policy, PC-room billing rules, or the existing local entitlement.

## SoD result sink

`WithSOD2RecordRanks` was only an asynchronous statistics sink: the return value
is ignored by the game event path. R22 writes the server-owned SoD result first
to the already reconstructed `SoD2DB__SOD2RecBySandurr` table from the same log
event worker. The DLL is attempted only if the SQLite write fails.

Provider-only PC-room/business metadata is not fabricated. R22 records account,
character, score totals, server, legacy PCRNo/GPCode and IP; unknown commercial
fields receive neutral values.

## Character audit

`CharLog` / `CharRecordLog` are also audit/history sinks. Rather than guessing
historical provider procedures, R22 adds an append-only local table:

- `GameLogDB__LocalCharacterEvent`

Each new/delete/logout event records the exact server-owned payload, including
64-bit EXP. Legacy event type values are retained: 1=new, 2=delete, 3=logout.
If the local write fails, the original `CharRecordLog`/`CharLog` fallback remains.

## Item/trade/post audit

R22 adds:

- `ItemLogDB__LocalItemEvent`
- `ItemLogDB__LocalItemEventEntry`

The existing asynchronous item queue now works even when the Database.dll item
log exports are absent. Event header + item entries are written in one SQLite
transaction. The local representation preserves raw server payloads for normal
item events, trades and gift/post events, including target identity, money,
gift number, telephone field and every `_LogITEM` code/serial tuple.

If the local transaction fails, the corresponding legacy DLL export is still
used when available.

## Schema

R22 adds migration `031_local_audit_events.sql`.

- previous: `user_version=30`, 333 user tables
- R22: `user_version=31`, 336 user tables

The 326 backup-reconstructed tables remain unchanged. The three new tables are
explicit server-local audit tables, not claims about historical SQL Server
stored-procedure behavior.

## What R22 deliberately does not change

- paid/PC-room authentication fallback;
- `Bl_RNo`, GPCode/GpType provider identity;
- `PCUse`, `PCSearchReserv`, `PCRNoReservProduct`, `/time`;
- expiry/disconnect billing timers;
- local gameplay entitlement semantics introduced in R21;
- ClanMoney/SiegeMoney/SiegeMoneyTax behavior;
- any EXP/drop/Pet formula.

R22 therefore removes low-risk logging dependencies first and leaves the
remaining Database.dll dependency concentrated around auth/PC-room policy and
runtime fallback.

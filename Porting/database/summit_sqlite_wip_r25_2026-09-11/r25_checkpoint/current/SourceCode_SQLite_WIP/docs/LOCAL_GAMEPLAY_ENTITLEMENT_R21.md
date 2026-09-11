# Local gameplay entitlement — R21

Date: 2026-09-11

## Goal

Provide a real SQLite-owned source for the historical PC-room gameplay benefit
without fabricating `Bl_RNo`, reusing `Bl_Meter`, or changing the layout of
`rsPLAYINFO`.

## Schema

Migration `030_local_gameplay_entitlement.sql` adds:

`LocalAuth__GameplayEntitlement(UserID, Entitlement, Enabled, ExpiresAt, UpdatedAt)`

The primary key is `(UserID, Entitlement)` with NOCASE identity semantics.
`Enabled` is constrained to 0/1. `ExpiresAt` is optional; NULL/empty means no
expiry.

The supported R21 entitlement key is:

`pcbang_gameplay`

Example grant:

```sql
INSERT INTO LocalAuth__GameplayEntitlement
    (UserID, Entitlement, Enabled, ExpiresAt)
VALUES
    ('SomeAccount', 'pcbang_gameplay', 1, NULL)
ON CONFLICT(UserID, Entitlement) DO UPDATE SET
    Enabled=1,
    ExpiresAt=NULL,
    UpdatedAt=datetime('now','localtime');
```

Example revoke:

```sql
UPDATE LocalAuth__GameplayEntitlement
SET Enabled=0, UpdatedAt=datetime('now','localtime')
WHERE UserID='SomeAccount' AND Entitlement='pcbang_gameplay';
```

## Runtime boundary

`bSql_GetLocalPCBangGameplayEntitlement()` reads the SQLite row once after a
successful account login. The result is stored in a server-local per-connection
slot cache outside `rsPLAYINFO`.

The gameplay facade now means:

`legacy Bl_RNo > 0 OR active local pcbang_gameplay entitlement`

Therefore existing paid/PC-room behavior remains compatible while a local/free
server can grant the perk without entering billing/reservation paths.

The cache is explicitly cleared when a connection slot is allocated and in both
active disconnect paths. Reads/writes use Windows interlocked operations.

## Gameplay effects

The same four R20 call sites remain behind the facade:

- +50 drop accumulator;
- +20% solo EXP;
- +20% party EXP;
- PCBang Pet activation.

No bonus magnitude or formula changed.

## What R21 deliberately does not do

- it does not make local entitlement populate `Bl_RNo`;
- it does not make `PCUse`, `PCSearchReserv`, `PCRNoReservProduct` or `/time`
  treat a local entitlement as a PC-room billing identity;
- it does not remove Database.dll paid/PC-room fallback;
- it does not alter `rsPLAYINFO` packet/shared structure layout.

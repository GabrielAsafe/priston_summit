# Gameplay entitlement boundary — R20

Date: 2026-09-11

## Goal

Separate the **meaning** of the historical PC-room gameplay benefit from the
commercial/accounting identifier `Bl_RNo` without changing gameplay behavior.

R20 does **not** remove PC-room billing, does not invent a new entitlement
schema, and does not change any bonus value. It creates a semantic seam only.

## Proven gameplay effects

The active server has four gameplay decisions driven by `Bl_RNo > 0`:

- drop-roll accumulator: `+50`;
- solo EXP reward: `+20%`;
- party EXP reward: `+20%`;
- PCBang Pet activation.

R20 routes those four decisions through:

```cpp
rsHasPCBangGameplayEntitlement(const rsPLAYINFO *lpPlayInfo)
```

The R20 implementation intentionally remains:

```cpp
return lpPlayInfo && lpPlayInfo->Bl_RNo > 0;
```

Therefore behavior is unchanged while the gameplay formulas no longer need to
know that the entitlement currently comes from a billing/PC-room identifier.

## What remains raw legacy billing state

R20 deliberately leaves `Bl_RNo` untouched in paths whose semantics are still
provider/accounting/session-specific:

- `PCUse` / periodic PC-room updates;
- `PCSearchReserv` / `PCRNoReservProduct`;
- `/time` billing/reservation flow;
- expiry/warning/disconnect accounting;
- billing/event logs;
- the legacy PCBang inter-server token payload and receive path;
- login/logout provider parameters.

Those paths must not be switched to a gameplay entitlement predicate because
that would conflate entitlement with provider identity again.

## Why no new field/table yet

A real local entitlement state needs an explicit ownership/lifetime model. The
current `rsPLAYINFO` type is shared legacy state and changing its layout before
a Windows compile/runtime checkpoint would widen regression risk. Reusing
`Bl_Meter` or manufacturing a fake `Bl_RNo` would also leak the local perk back
into billing/reservation code.

R20 therefore stops at the safe semantic boundary. After Windows verification,
a later revision can introduce a server-local entitlement state and change only
the facade implementation, leaving the four gameplay call sites untouched.

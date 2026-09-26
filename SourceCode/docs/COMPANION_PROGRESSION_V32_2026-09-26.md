# Companion Progression Runtime V32

The offline EXP persistence path already performs the authoritative character-record work: EXP is capped/gated like the legacy player path, level is advanced, `ReformCharStatePoint()` grants the level state points, `Next_Exp`/character integrity fields are rebuilt and the DAT is backed up/written.

V32 closes the live-runtime gap after that write. A companion that levels while still summoned now immediately mirrors the progression header committed to its DAT instead of keeping stale level-state until dismiss/resummon.

## Live sync after a successful EXP flush

- cache and runtime level are updated;
- live `smCHAR_INFO` EXP is updated through `SetExp64()`;
- on an actual level transition, the freshly written record is reloaded and only progression fields are copied back: `StatePoint`, `Next_Exp`, `ChangeJob`, `JobBitMask`;
- current HP/MP/Stamina, combat stats and companion equipment overrides are deliberately left untouched;
- `LEVEL_UP` debug telemetry records the transition.

This avoids copying the entire DAT `smCharInfo` over the live companion, which would incorrectly replace runtime resources and the companion-specific equipment layer.

## Persistence semantics

Dismiss, DismissAll, periodic flush and owner disconnect continue using the existing retry-minded EXP flush path. V32 does not claim runtime/Wine validation of logout/restart; it makes the live state consistent with the same persisted record those flows already use.

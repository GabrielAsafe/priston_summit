# Companion Combat HUD V39 — Incoming threat indicator

V39 extends the existing runtime HUD snapshot without changing packet size or protocol version.

`CompanionRuntimeStatusFlags` gains `COMPANION_RUNTIME_STATUS_THREAT` (`1<<4`). The server sets it when any valid hostile in the companion's current stage area has `lpTargetChar` pointing at that companion. This is authoritative incoming aggro, not an inference from the companion's own selected target.

The client party row renders `AGGRO` alongside Slow/Stun/Poison/Distortion. Existing cast progress, cooldown, target and positive-effect telemetry are unchanged.

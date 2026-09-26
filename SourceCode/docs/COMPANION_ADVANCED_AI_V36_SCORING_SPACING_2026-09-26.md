# Companion Advanced AI V36 — Target Scoring and Combat Spacing

V36 builds on V35 without adding a second AI/pathfinding system.

## Stable target scoring

When a preset is allowed to choose a generic hostile, candidates inside the existing sight cap are scored from squared distance plus bounded preference bonuses:

- retain the current valid target when alternatives are only modestly better;
- prefer a hostile attacking the owner;
- prefer a hostile attacking another active account companion;
- preserve explicit owner target priority in Balanced/OwnerFocus before scoring.

The retention/threat bonuses are intentionally smaller than the sight-radius distance delta, so a stale far target can still be replaced by a clearly superior nearby threat. Defensive continues to use its stricter threat-first path from V35.

## Combat spacing

The follow formation already gives each companion a persistent formation slot around the owner. V36 reuses that slot while two or more companions converge on the same hostile. During an out-of-range chase, later companions approach one of four small 36-unit offsets around the target rather than all tracing its exact center.

The offset is safely inside the executor's minimum combat range (80 units), and only changes movement anchoring. Damage/range checks still use the real hostile position. When no other companion is targeting the hostile, legacy exact-target tracing remains unchanged.

This is spacing on top of the existing mover, not a new pathfinder or collision solver.

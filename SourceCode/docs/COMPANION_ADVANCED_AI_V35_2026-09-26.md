# Companion Advanced AI V35

V35 extends the existing Tactics V2 path rather than introducing a second AI system.

## Defensive threat selection

The Defensive preset now scans the current stage for the nearest valid hostile that is actually targeting the owner or any active account companion. A current retaliating target is retained first, then party threat, owner target, current target and finally generic nearest hostile. This reduces target churn while making the preset react to real incoming threat.

## Preferred-range kiting

`preferredRange` was already persisted/configurable, but previously only stopped forward chase. When explicitly non-zero, it now also creates an inner 55% hysteresis band. If a hostile enters that band, the companion takes a bounded 32–120 world-unit step away from the target and uses RUN (WALK fallback). A zero preferred range keeps the legacy/current behavior, so melee companions are not forced into kiting.

## Stuck recovery

While a hostile remains outside combat range, each managed slot samples movement once per second. Two consecutive probes with less than eight world units of progress reset `TargetMoveCount`, `AutoMoveStep`, trace state and deferred target movement before the normal combat trace is reissued. This is deliberately a recovery of the existing legacy mover, not a new pathfinder.

The slot's progress probe state is cleared when it has no combat target and on slot reset.

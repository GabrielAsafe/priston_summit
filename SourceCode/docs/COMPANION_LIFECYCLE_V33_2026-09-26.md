# Companion Lifecycle V33

V33 closes two lifecycle gaps that were still capable of producing destructive runtime behaviour.

## Owner map transition / warp

Account companions are live `smCHAR` entities and can outlive an owner warp. Before V33, the normal 500-unit hard leash queried the companion's old `lpStage` using the owner's new-map X/Z. An invalid height from that old stage could kill the companion.

The server character loop now detects an owner `Position.Area` whose `StageArea[].lpStage` differs from the companion stage. For account companions only it:

1. resolves the companion formation/follow X/Z;
2. switches `lpStage` to the owner's authoritative stage;
3. resolves the destination floor there, with owner-Y + 2 as safe fallback;
4. snaps position/anchor and clears stale target/movement state;
5. then runs normal Companion AI/hard-leash logic.

This does not alter generic monsters or legacy skill pets.

## Owner death

The defined policy is **hold/follow, no combat** while the controlling player is dead. The companion remains alive and visible, but clears targets and does not regenerate/use potions/cast/attack through Companion AI. Pending Raving/Avalanche multi-hit and Charging Strike delayed hits are explicitly cancelled so they cannot land after the owner dies or much later after revive.

Absolute-duration buffs/familiars are not reset; their timers continue naturally and can resume only if still active when the owner returns alive.

## Existing lifecycle retained

Companion death/corpse retention and Resurrection V26 remain unchanged. Dismiss/DismissAll and owner disconnect still flush EXP before entity cleanup. Profile/loadout persistence remains sidecar-authoritative.

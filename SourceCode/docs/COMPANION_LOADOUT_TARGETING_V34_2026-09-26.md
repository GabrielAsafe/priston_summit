# Companion Loadout Target Counting V34

V30 introduced `minimumTargets` per active-skill slot. V34 fixes the remaining geometry mismatch in that gate.

The original V30 counter understood ordinary target-centered radial skills and forward BOX skills, but several already-authoritative companion adapters deliberately use **caster-centered** legacy geometry. Those skills could therefore report only one target even when their execution would hit/control several enemies.

V34 makes the loadout precondition use the same geometry family as execution for:

- Pike Wind;
- Ground Pike;
- Roar;
- Venom Spear (including `VenomSpear_Num` target cap);
- Extinction;
- Distortion.

Glacial Spike and the other frontal skills remain on the existing BOX counter. Ordinary AoE remains target-centered. Vertical tolerance matches the status iterator (`128` world units).

This patch changes only the `minimumTargets` admission test; damage/status execution itself is unchanged.

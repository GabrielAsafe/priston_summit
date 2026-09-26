# Companion Skill Loadout V30

V30 completes the active-skill loadout policy layer without changing the 84-byte companion profile ABI.

## Per-slot policy

Active slots are now deterministic priority: slot 1 is evaluated first, then 2, 3 and 4 as fallbacks.

Each active slot persists three policies in the profile's previously reserved 12-byte tail:

- `R` — post-cast MP/RES reserve percentage, cycled 0..90 in 10% steps. It is an additional per-slot floor on top of the global V8 resource-stability policy.
- `N` — minimum number of valid targets, cycled 1..5 in the compact UI (disk/runtime accepts 1..16). Single-target skills naturally fail `N>1`; AoE/BOX skills count candidates using the same geometry/max-target rules as execution.
- `T` — target policy: `TA` AUTO (AI target), `TO` OWNER_TARGET (strict owner target), `TN` NEAREST (nearest valid hostile in the current stage area).

Changing the selected skill in a slot resets that slot's policy to `R0 / N1 / TA`. Resetting the active loadout clears all policy state.

## Persistence migration

Profile version becomes V4, while V1/V2/V3 remain accepted. V4 reuses bytes that were reserved in the original 84-byte disk struct, so existing profile file size and checksum framing remain unchanged. V3 profiles load the previous skill/support loadouts and receive permissive V30 defaults.

## Protocol/UI

Companion UI protocol becomes v5. The skill row exposes compact policy buttons next to the active skill name. Server remains authoritative; UI actions only edit the sidecar profile.

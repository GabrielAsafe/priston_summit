# Companion Combat HUD V31

V31 upgrades the compact 1 Hz companion runtime snapshot used by the party HUD. The UI still consumes network state only; it never dereferences server/legacy `smCHAR` internals.

## Runtime protocol v2

Each active companion now publishes, in addition to HP/MP/mode/tactic/target:

- authoritative action-lock/cast state;
- current skill code and canonical skill name;
- cast/action progress in permille (0..1000), derived from the executor lock start/end timestamps;
- nearest pending executor cooldown in milliseconds;
- status flags for Slow, Stun, Poison and Distortion;
- up to six positive runtime effects with skill code and remaining milliseconds.

The packet remains compact (`sizeof(CompanionRuntimeStatePacket) == 912` on the normal 32/64-bit host ABI used by the validator probe), below the existing 1024-byte guard.

## Effect sources

The snapshot includes the existing four local buff modifiers plus the separate runtimes introduced by later companion skill adapters: Force of Nature, Holy Valor, Triumph/Hall of Valhalla, Regeneration Field, Enchant Weapon, Falcon/Golden Falcon and the persistent familiar slot. Native monster status fields remain represented as negative status flags rather than duplicated pseudo-buffs.

## HUD

The party row now shows the active skill and progress while the executor is locked, the nearest cooldown rounded up to seconds, the positive-effect count and active Slow/Stun/Poison/Distortion markers.

V31 is a telemetry/HUD closure step. It does not claim icon rendering or a full tooltip/effect browser; those remain presentation work on top of the now explicit snapshot contract.

# Account Companion System V1

This build adds a server-side companion foundation that can summon another character from the same account as an allied `smCHAR` and share monster EXP with it persistently.

## Commands

- `/companion list` - list characters on the current account.
- `/companion select <name>` - select the character used by the companion crystal.
- `/companion summon [name]` - summon the selected character, or a named character.
- `/companion dismiss [name|all]` - dismiss one or all account companions.
- `/companion mode aggressive` - use legacy summon AI and prioritize the owner's current monster target.
- `/companion mode follow` - follow without an explicit target.
- `/companion mode passive` - clear explicit targets and follow the owner.
- `/companion status` - show active count, selected character, mode, and configured crystal code.

The current logged-in character cannot be summoned. A character that is already online cannot be summoned. Up to four account companions can be active, matching a five-character account with one active player plus four alts.

## Crystal

`Configs/Companion.ini` controls the companion crystal. `CrystalItemCode=0` leaves item interception disabled. Set it to the decimal `sITEMINFO.CODE` of the item you want to use. The item should use the same `PotionCount=100` convention as legacy summon crystals. On success the item is consumed by the normal throw-item flow; if summon creation fails it is returned using the same legacy pattern.

## EXP semantics

Active companions count as party members when selecting `PartyExpTable` / `[PartyRate]` from `Configs/ExpManager.ini`.

Examples:

- solo player + 1 companion = effective party size 2;
- solo player + 4 companions = effective party size 5;
- a normal multiplayer party counts the active companions of members in the killer's area as additional effective members (capped at the existing 8-entry rate table).

Each companion's EXP is calculated independently using its own saved level, the existing monster-level-difference penalty, and the existing `[ExpConfig] Nv[level]` multiplier. Its share is buffered in memory and flushed to that character's record periodically (and forcibly on dismiss/death/logout), avoiding a full record write per kill. Level and state-point progression are updated in the offline record; the visible summoned entity's level is refreshed immediately. Personal premium/VIP boosts of the owner are not copied to offline companions.

## Runtime model

The companion is deliberately not a fake `rsPLAYINFO` and has no fake socket. It is an `smCHAR` using the existing summon follow/teleport/attack loop, with `lpMasterPlayInfo` pointing at the owner. This keeps legacy party networking, chat, disconnect logic, and socket assumptions intact.

The spawn snapshot comes from the alternate character's `TRANS_RECORD_DATA` / `smCHAR_INFO`. A valid legacy crystal monster is used only as the required `smCHAR_MONSTER_INFO` AI template. Basic attacks therefore retain the character snapshot's attack stats while movement/targeting reuses the stable summon AI.

## Safety / persistence

- The account-character list is checked before loading or modifying an alternate record.
- Online characters cannot be summoned.
- Owner disconnect destroys all managed companions before normal character-save teardown.
- Companion EXP writes are performed under the existing character-save critical section and create the normal backup before replacement.
- Companion inventory is not mutated in V1, avoiding concurrent inventory/save and duplication problems.

## Not yet in V1

This is the core playable/backend pass. It does **not** yet emulate the complete player-skill executor for every class, a Dragon Age-style tactics UI, per-companion loot filters, or a combined visual party panel. Those should build on `CompanionSystem` rather than inserting companions into `lpPartyPlayers[]`, because the legacy party array assumes every member is a connected `rsPLAYINFO` with a socket.

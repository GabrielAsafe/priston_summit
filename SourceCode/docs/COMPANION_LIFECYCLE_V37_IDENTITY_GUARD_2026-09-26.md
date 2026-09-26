# Companion Lifecycle V37 — Real-player identity guard

V37 closes the remaining coexistence gap between an account companion and the same character logging in as a real player.

## Existing summon-time contract

`Summon()` already rejects a character returned by `FindUserFromName()` when that online character is not the owner. That only protects the instant of summon.

## Session-time guard

Each managed companion now performs a lightweight identity check at most once every two seconds. If the same character name appears as a live `rsPLAYINFO` after the companion was already summoned, the companion is retired before any further resource regen, skill execution or combat AI runs.

Retirement follows the existing persistence discipline:

1. flush pending companion EXP once;
2. kill/detach the companion entity immediately so duplicate gameplay cannot continue;
3. clear the slot if persistence succeeded;
4. if persistence failed, retain the reserved slot/name/pending EXP for the existing prune/retry path;
5. publish runtime/visual state immediately.

The owner character itself is never a valid companion (existing summon/select checks already reject it), so the guard only resolves a true alt identity conflict.

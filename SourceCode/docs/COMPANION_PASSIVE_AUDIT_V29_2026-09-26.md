# Companion Passive Audit V29 — T1–T4

## Result

The remaining 16 `CATALOG_ONLY` entries in the 128-skill T1–T4 catalog are all passive skills. They are intentionally **not selectable/executable** by `CompanionSkillExecutor`.

The companion spawn path copies the persisted `TRANS_RECORD_DATA::smCharInfo` snapshot into the runtime `smCHAR` before any companion-specific skill execution state is loaded. This snapshot is the authoritative character-stat image used by the existing companion bridge. The 16 T1–T4 skill points are loaded separately into `CompanionSkillRuntime::legacySkillPoints` only so active skills can resolve learned levels and cross-skill dependencies.

Therefore V29 does **not** reapply passive percentages or flat bonuses from the skill tables. Doing that would risk double-applying passive effects already materialized in the persisted character stats.

## Audited passive set

- Mechanician: Poison Resistance, Mechanic Weapon Mastery
- Fighter: Weapon Mastery, Fire Resistance, Boost Health
- Pikeman: Ice Resistance, Weapon Defence Mastery, Critical Mastery
- Archer: Shooting Mastery, Dions Eye, Evasion Mastery
- Knight: Physical Training, Sword Mastery
- Atalanta: Throwing Mastery
- Priestess: Meditation
- Magician: Mental Mastery

## Runtime contract

1. `OpenCharacterCompanionFromRecord()` copies `recordData->smCharInfo` into the companion runtime entity.
2. Companion spawn restores current HP/MP/RES to the copied maxima but does not recompute passive skill stats.
3. `LoadCompanionSkillPoints()` separately copies the 16 persisted T1–T4 points into the executor runtime.
4. Passive catalog entries remain `CATALOG_ONLY` / non-executable and never occupy active/support loadout slots.
5. Active T1–T4 coverage is therefore complete at **112 READY + 16 audited passives = 128/128 classified**.

## Future rule

Only add a companion-side passive recomputation layer if the persistence contract changes so that `smCharInfo` no longer contains the effective player stats. If that happens, passive recomputation must be done once from base stats, never incrementally on top of an already-materialized snapshot.

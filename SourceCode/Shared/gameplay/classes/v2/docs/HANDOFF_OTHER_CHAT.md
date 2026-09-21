# Handoff — srcLastBugFix + Playable Class System V2

## Base of truth

This tree uses the **literal contents of `srcLastBugFix.zip`** as its base. Do not reconstruct it from the older repository history: the archive contains 23 uncommitted fixes made today, including Tier 1-4 animation/skill behavior and Tier 5 related changes.

## What was integrated

Playable Class System V2 was merged semantically on top of that archive. The goal is to decouple playable-character/class identity from scattered `JOB_CODE` switches, fixed 8-class arrays, job-number ranges and parallel registries.

The refactor intentionally does **not** replace concrete skill handlers/effects. Tier skill fixes in `srcLastBugFix` are therefore authoritative and have been preserved.

## Current status

- 8 legacy classes represented by the V2 class domain/registry/service.
- Assassin reserved/registered but remains `enabled=false`; canonical content is still incomplete.
- Creation/selection, formulas, brood/gender, appearance, armor routing, equipment policy, animation metadata, motion masks, item specialization, ranks, generic UI class presentation, loot filtering and generic skill routing are behind the class abstraction.
- Legacy quests/events/kits remain opt-in adapters rather than class-domain dependencies.
- Concrete class-specific skills remain in the skill subsystem by design.
- Default class content is still compiled C++ through `IClassContentSource`; a JSON/Lua/DB loader is a future content-source implementation, not required for this merge.

## Important recent-fix guarantee

See `TODAY_FIXES_PRESERVATION.md`. All non-trivial lines added in the 23 files changed today are present in this integrated source.

## Tests already run here

```text
playable_class_registry_smoke: PASS
legacy_class_quest_adapter_smoke: PASS
legacy_class_commerce_adapter_smoke: PASS
legacy_class_onboarding_adapter_smoke: PASS
legacy_class_event_kit_adapter_smoke: PASS
Class architecture guard: PASS
Class system V2 integration suite: PASS
client build script syntax: PASS
server build script syntax: PASS
```

## What the next chat should do

Use this integrated source as the new base for the remaining implementation work. Before declaring release-ready, run the native MinGW i686 client/server compile + link and regress in game:

1. all eight legacy classes create/select/login/relog;
2. Tier 1-4 skills and animations fixed today;
3. Tier 5 skills touched today;
4. weapon/armor/model/face changes;
5. party/ranking/kill notification UI;
6. rank promotion and persistence;
7. death/respawn/map change/server restart.

Do not reapply an older class patch over this tree; that can overwrite today's skill/animation fixes.

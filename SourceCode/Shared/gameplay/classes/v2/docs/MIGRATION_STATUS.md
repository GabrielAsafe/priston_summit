# Playable Classes V2 — Migration Status

## Migrated to registry/service/policies

- class identity registry and validation;
- eight-class legacy parity plus disabled reserved Assassin;
- character creation base stats;
- character selection enumeration/order;
- brood/faction lookup;
- gender policy;
- formulas;
- item specialization identity and rank aliases;
- model/motion applicability identity;
- appearance/head lookup;
- armor routing with legacy and data-driven paths;
- equipment affinity/default weapon/model slots;
- generic combat critical-scaling attribute;
- generic animation anchors/restore quirks/item frame adjustments;
- skill-tree/group routing and legacy T5 opt-in;
- item requirement revision policy;
- item specialization tooltip enumeration;
- generic loot-filter specialization lookup;
- kill/party/SOD/ranking class presentation lookup;
- low-level present-item compatibility row moved out of the class domain into onboarding content;
- onboarding, quest and commerce compatibility adapters;
- historical 20/40/60 change-job quests gated by explicit legacy-content support;
- Bless Castle class-target crystal compatibility adapter;
- persistence/field brood decisions that formerly inferred faction from job number;
- skill integrity state keyed by `jobCode` instead of eight fixed slots;
- semantic HoLogin selection/face capacities separated from class count;
- class content split into composable sources (legacy-eight + reserved);
- transactional source -> registry loading with whole-registry validation;
- build-manifest guard for every Shared playable-class implementation unit;
- class-specific random-box equipment bundles (types 11/14) isolated behind an event-content adapter with full 189-grant parity verification;
- default built-in class content now fails fast at startup if composition/validation is invalid;
- generic presentation enumeration narrowed to `ClassService::EnabledJobCodes()`;
- active generic runtime paths audited free of class-as-array-index and numeric brood inference patterns.

## Intentionally not moved into ClassDefinition

- concrete skill damage/effect logic;
- concrete skill visual effects;
- old event-specific item bundles;
- localized content that is meaningful only to one old event/UI;
- protocol constants themselves.

These belong to their own subsystems/content adapters.

## Reserved / fail-closed

Assassin is present only as a reserved identity. It remains disabled until canonical contracts and runtime assets are proven.

## Release gates still external to portable tests

- MinGW i686 full client compile/link;
- MinGW i686 full server compile/link;
- Server.exe startup and restart;
- create/select/login/relog for existing eight classes;
- model/head/armor/equipment smoke;
- item specialization and OpenItem smoke;
- skill/rank smoke;
- optional legacy content smoke.


## Remaining concrete class symbols

The remaining audited `JOBCODE_*` references in active runtime code are protocol declarations or concrete skill/effect behavior. They are intentionally not migrated into `ClassDefinition`; doing so would turn the class domain into a god-object. The architecture guard contains the explicit whitelist and fails on new occurrences elsewhere.

## Content-source status

The engine/content boundary is decoupled through `IClassContentSource`, composition, transactional loading, and `ClassService`. The built-in sources are still compiled C++. A file/database-backed class-content source is a future extension point, not part of this patch.

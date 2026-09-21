# Playable Classes V2 — Merge Readiness

## Reference

- Working baseline commit: `6fb065b8c27b` (`baseline src_new_sistems`).
- This refactor is intended to be reviewed/applied as a single architectural change against that baseline, then reconciled with the current bugfix source during the later merge.

## What this patch changes

The patch changes the ownership model of playable-class knowledge. Generic engine systems query `ClassService`; class definitions come from composable `IClassContentSource` providers; old quests/events use explicit compatibility adapters; concrete skill ownership remains in the skill subsystem.

The patch deliberately preserves `smCHAR_INFO::JOB_CODE` and other wire/save contracts at the compatibility boundary rather than rewriting the protocol.

## Merge-ready at source/domain level

The following are complete enough for source review and merge preparation:

- eight-class parity content;
- reserved/fail-closed Assassin identity;
- registry/service/content composition;
- creation/selection/brood/gender/formulas;
- appearance/armor/equipment/combat/animation policies;
- model-motion job resolution;
- item specialization/rank token resolution;
- configurable rank chain and legacy `ChangeJob` translation;
- generic UI/presentation lookups by real job code;
- legacy quest/onboarding/commerce/Castle/event-kit boundaries;
- startup validation/fail-fast behavior;
- architecture guard and portable smoke tests;
- client/server build-manifest coverage for V2 implementation units.

## Deliberately not claimed complete

- Assassin gameplay enablement;
- data-file-only class authoring (current sources are compiled C++ behind `IClassContentSource`);
- rewrite of concrete skill handlers;
- rewrite of every historical event/localization table;
- native MinGW i686 compile/link;
- in-game runtime/regression acceptance.

## Merge policy

When reconciling with the newer bugfix tree:

1. keep bugfix behavior unless this patch explicitly replaces the class-ownership mechanism;
2. keep V2 class-domain/service APIs as the authority rather than reintroducing class ranges/tables;
3. resolve legacy-file conflicts behaviorally, not by choosing whole-file versions;
4. preserve CRLF/legacy encodings of existing files;
5. rerun the architecture guard after every conflict resolution;
6. rerun native client/server compile/link before runtime smoke.

## Acceptance gate

This package should be considered **source/domain merge-ready, native-runtime pending**. Native acceptance requires the MinGW and in-game checks listed in `TEST_REPORT.md`.

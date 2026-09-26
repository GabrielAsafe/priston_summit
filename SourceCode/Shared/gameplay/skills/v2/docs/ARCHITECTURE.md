# Skills V2 architecture

`gameplay/skills/v2` is the canonical skill domain for Summit.

The rule is the same one used by Classes V2:

```text
content -> SkillRegistry -> SkillService -> LegacySkillAdapter -> old runtime APIs
```

A tier is metadata (`SkillDefinition::tier`), never an execution architecture.

`SkillDefinition::id` is the logical identity. `legacy.playCode` is the old
network/runtime binding. `legacy.presentationPlayCode` is visual-only and may
reuse a proven legacy effect without changing gameplay/network identity.

Player and Companion consumers must query this domain rather than maintaining
their own skill catalogs. Legacy `sinSkill`, `SkillSub`, `ContinueSkill`,
motions, particles and packet APIs stay behind compatibility code.

The initial migration deliberately leaves damage/buff formula bodies in their
existing legacy implementations. The source of truth for identity, ownership,
slot/tier, behavior family, play code and presentation binding is no longer
Companion-specific.

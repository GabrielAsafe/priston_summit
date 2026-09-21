# Playable Classes V2 — Architecture

## Goal

The playable-class subsystem owns **class identity and reusable class policy**. Legacy client/server code consumes it through `ClassService` and small adapters instead of maintaining parallel `JOB_CODE` switches, fixed eight-row tables, or numeric job ranges.

The protocol field `smCHAR_INFO::JOB_CODE` remains a compatibility identity at the boundary. It is **not** the domain model and must not be used as an array index or as an implicit race/brood encoding.

## Layers

```text
IClassContentSource(s)
        |
CompositeClassContentSource
        |
        v
PlayableClassDefinition[]
        |
        v
PlayableClassRegistry  ---- validation / uniqueness / fail-closed
        |
        v
ClassService            ---- stable queries and policies
        |
        +-----------------------------+
        |                             |
        v                             v
legacy adapters                  V2 consumers
        |
        v
old UI / inventory / model / quest / skill gateways
```

### Domain

`domain/ClassDefinition.h` contains only class-owned data. It has no dependency on UI, inventory, quests, renderer, packets, or concrete legacy headers.

A definition may own:

- identity: `id`, `legacyJobCode`, `jobBitCode`, `key`, `displayKey`, `brood`, gender;
- base stats and character formulas;
- character creation/selectability and selection presentation;
- appearance/head/body namespace;
- armor profile (legacy row **or** data-driven models);
- equipment affinity and model slots;
- combat policy shared by generic combat code;
- animation compatibility data;
- skill-tree route and legacy skill group route;
- rank chain and content aliases;
- item requirement revision policy;
- class presentation assets used by generic UI;

### Content composition

Class content is intentionally composable. The current default runtime is not
one monolithic catalog anymore:

```text
LegacyEightClassesContentSource
             +
ReservedClassesContentSource
             |
             v
CompositeClassContentSource
             |
             v
LoadClassContent (transactional + Validate)
             |
             v
PlayableClassRegistry
```

The concrete list of installed sources lives only in
`content/DefaultClassContent.cpp`, the application composition root. The
registry implementation does not include or reference concrete catalogs.

`CompositeClassContentSource` does not own its sources; the composition root
owns their lifetime. Passing a temporary source is explicitly rejected.

Future content packs may add another `IClassContentSource` without changing
`ClassService` or existing consumers. `LoadClassContent` stages the complete
payload and only replaces the registry after registration **and validation**
succeed, so a broken pack cannot leave a partially loaded class catalog.

### Registry

`PlayableClassRegistry` is the authority for which definitions exist. Registration is transactional and validation is fail-closed.

Important invariants include:

- unique class id, legacy job code and key;
- enabled classes must have complete gameplay/creation/model/skill contracts;
- normalized item/rank content tokens cannot resolve to conflicting bits;
- model-motion aliases use their **own mask protocol** and cannot collide;
- legacy skill-group routes are unique;
- reserved/incomplete classes can exist with `enabled=false` without pretending their missing values are known.

### Service

`ClassService` is the read/query API used by the rest of the engine. Consumers should ask semantic questions such as:

- `IsInBrood(job, brood::Tempskron)`
- `HasCapability(job, ClassCapability::Caster)`
- `TryGender(job, ...)`
- `TryBaseAttributes(job, ...)`
- `ArmorModel(job, variant, index)`
- `TryMotionJobMask(job, ...)`
- `TryJobBitCodeByContentToken(token, ...)`
- `SkillTreeKey(job)`
- `EnabledJobCodes()` for presentation/UI enumeration without exposing domain storage

Consumers should **not** recreate these answers from `JOB_CODE` ranges or class-name switches. Presentation-only code should prefer narrow queries such as `EnabledJobCodes()` rather than receiving `PlayableClassDefinition` objects merely to enumerate jobs.

## Protocols that must stay separate

Several historical values look similar but are not interchangeable:

| Contract | Meaning |
|---|---|
| `legacyJobCode` | protocol/runtime class identity (`JOB_CODE`) |
| `jobBitCode` | item/class specialization bit |
| rank `legacyJobBitCode` | specialization/rank bit used by item content |
| `animation.legacyMotionJobMask` | `*APPLICABLE_JOB` model/motion mask |
| `skills.legacyGroupCode` | legacy skill-code routing group |

The item specialization bit and motion mask are especially dangerous to confuse: the historical mappings are different even for existing classes.

## What belongs outside the class domain

The class domain must not become a new god-object.

### Skill-specific behavior

A check such as “Spark Shield belongs to Mechanician” or “Golden Apple belongs to Atalanta” is behavior of that concrete skill. It stays in the skill/combat subsystem, reached through the class skill route where appropriate.

### Quest/event/shop/onboarding content

A legacy event that gives a different item bundle to each class is content, not class identity. Put the mapping in a legacy content adapter. A new class can exist without automatically participating in that old event.

Examples already isolated:

- onboarding/starter gifts;
- low-level present-item row mapping;
- quest reward compatibility;
- commerce/manufacture compatibility;
- Bless Castle class-target crystals.

### Localization

The class registry owns stable keys/aliases. Legacy localized tables can remain behind presentation adapters while they are still needed. They must not become identity authorities again.

## Compatibility capacities are not class counts

Historical constants have been renamed/reframed where possible:

- `LEGACY_PLAYABLE_JOB_SLOT_COUNT`: historical eight-slot compatibility width;
- `LEGACY_ARMOR_MODEL_ROW_COUNT`: number of rows in the old armor model matrices;
- `CHARACTER_SELECTION_SLOT_CAPACITY`: preview/UI capacity of HoLogin;
- `CHARACTER_FACE_SLOT_CAPACITY`: legacy face-card capacity.

None of these values define how many classes may exist in `PlayableClassRegistry`.

## Failure policy

Unknown, disabled, incomplete, or unsupported class content should fail closed:

- character creation does not invent stats/models;
- optional legacy event content does not guess a row;
- invalid parser tokens do not silently resolve by registration order;
- unsupported visual/profile lookups return failure/null rather than indexing by `jobCode - 1`.


## Startup validation

Default class-content composition is fail-fast: an invalid built-in catalog aborts startup instead of exposing a partially/empty initialized registry.

# Legacy Boundaries and Migration Rules

## Allowed kinds of class-specific knowledge

### 1. Class content source — allowed

The compiled legacy parity catalog necessarily names Fighter, Mechanician, Archer, Pikeman, Atalanta, Knight, Magician and Priestess because it is **content**.

### 2. Concrete skill handler — allowed

A handler for a concrete skill may verify that the actor belongs to the class that owns that skill. Moving every such check into `ClassDefinition` would only hide skill logic inside a new god-object.

### 3. Explicit legacy content adapter — allowed

Old quests/events can map existing class identity to historical rewards/items through a narrow adapter. This is opt-in compatibility content.

## Forbidden in generic character/class infrastructure

- numeric brood inference such as `JOB_CODE <= 4`;
- `JOB_CODE - 1` array indexing;
- using `MAX_JOBNUM` as the runtime class count;
- concrete eight-class switches in character selection, appearance, generic inventory, generic model loading or generic UI;
- `JobDataBase` as an identity parser;
- `ArmorJobNum` as class policy;
- `BitMaskJobNames` as the model parser authority;
- treating item-specialization bits as motion masks;
- enabling a reserved class by filling unknown values with guesses.

`class_architecture_guard.py` enforces the highest-risk boundaries automatically.

## Known intentionally legacy/content-specific areas

The following may still contain fixed historical content and are not authoritative class registries:

- concrete skill/effect handlers;
- event-specific class item kits;
- localization tables such as the eight existing Bless Castle target labels;
- historical armor model matrices, accessed through the armor adapter/profile;
- legacy localized `JOB_DATA_CODE` presentation rows, accessed through a bridge where still needed.

A new class does not become invalid because one of these old pieces of content does not support it. The integration should fail closed until that content is deliberately added.

## Merge/runtime validation boundary

Portable host tests prove domain contracts and guard architecture, but they do not prove ABI, DirectX-era headers, MinGW i686 linkage, packet compatibility, or in-game model/skill behavior. Those remain release gates in the native Windows toolchain.


## Legacy event content

- Random-box/equipment bundles (`CaixaItensRandom` types 11/14) are legacy event content behind `LegacyClassEquipmentKitAdapter`; new classes opt in explicitly.

## Concrete `JOBCODE_*` whitelist

After the migration, remaining concrete class symbols in active runtime code are restricted to protocol declarations and concrete skill/effect handlers. The architecture guard rejects new concrete class symbols outside class-content sources, explicitly named legacy content adapters, tests, and the audited skill/protocol whitelist.

This is intentional: ownership of Spark Shield, Golden Apple, Evasion Mastery, Tier-5 handlers, and similar concrete skills belongs to the skill/combat subsystem, not to `ClassDefinition`.

# Adding a Playable Class

This is the intended workflow after the V2 decoupling. A new class should be added as **content plus explicit optional integrations**, not by adding `case JOBCODE_X` throughout the engine.

## 1. Establish protocol identity

Before enabling the class, obtain canonical values for every protocol-facing contract that actually exists:

- stable class id;
- `legacyJobCode` if the current wire/save protocol still requires one;
- item specialization `jobBitCode`;
- model/motion applicability mask;
- legacy skill group route, if the class uses the existing skill runtime;
- rank specialization bits.

Do not derive one bitmask from another. Do not invent missing historical values.

A class with incomplete canonical contracts may be registered as reserved with `enabled=false`.

## 2. Create the definition in a content source

Do **not** add future classes to the eight-class parity catalog just because it
already exists. The current default runtime is composed from:

- `LegacyEightClassesContentSource` — only the proven eight legacy classes;
- `ReservedClassesContentSource` — known identities that deliberately remain
  disabled (currently Assassin);
- `CompositeClassContentSource` — composition boundary.

A new content pack can implement another `IClassContentSource` and be added to
`content/DefaultClassContent.cpp`. **Today these sources are compiled C++ content.** The
interface is intentionally ready for a future file/database loader, but this branch does
not yet provide a JSON/YAML/TOML/data-file implementation. `ClassService` and gameplay
consumers do not depend on that storage choice.

Use `LoadClassContent()` to apply a source. It is transactional and validates
the resulting registry before commit. Do not manually loop over `Register()`
from runtime composition code.

At minimum an enabled/selectable definition should provide:

```cpp
PlayableClassDefinition d;
d.id = ...;
d.legacyJobCode = ...;
d.jobBitCode = ...;
d.brood = ...;
d.key = "...";
d.displayKey = "...";
d.gender = CharacterGender::...;
d.enabled = true;

d.formula = ...;
d.creation.selectable = true;
d.creation.selectionOrder = ...;
d.creation.baseAttributes = ...;
d.creation.bodyModels = ...;
d.creation.faces = ...;
d.creation.presentation = ...;

d.appearance = ...;
d.armor = ...;
d.equipment = ...;
d.combat = ...;
d.animation = ...;
d.skills = ...;
d.ranks = ...;
```

Run registry validation before exposing the class to gameplay.

## Current content-authoring boundary

Adding a class no longer requires modifying generic character-selection, appearance, armor, inventory, model-loader, presentation, or rank infrastructure. It **does** currently require compiling the new `IClassContentSource` implementation (or extending an intentionally compiled content source). Optional historical events/quests remain explicit opt-in adapters.

A future file-backed source can remove the compile step without another engine-wide migration.

## 3. Creation and character selection

No class switch should be required in HoLogin.

Provide:

- brood;
- `creation.selectionOrder` inside that brood;
- base attributes;
- body models;
- face options and previews;
- selection label/description assets.

`CHARACTER_SELECTION_SLOT_CAPACITY` is only the capacity of the legacy selection UI. The registry itself has no eight-class limit. If one brood exceeds the UI capacity, the UI must be expanded/paged; do not redefine the registry count.

## 4. Appearance

For an existing legacy class, `appearance.legacyCatalogKey` may bridge to the historical head matrices.

For a new class, prefer data-driven appearance:

- `bodyModelRoots`;
- `headModels[face][hair]`;
- `defaultHeadModels`.

A new class should not require editing `GetFaceHairSkin`, `FindFaceSkin`, `CheckUserModel`, or another `switch(JOB_CODE)`.

## 5. Armor

There are two routes:

### Legacy route

Set `armor.legacyModelIndex` only when the class intentionally reuses one of the existing historical armor rows.

### Data-driven route

For a genuinely new visual class, set `legacyModelIndex = -1` and provide:

- `armor.modelRoot`;
- `normalModels`;
- optional `comicModels`;
- optional `newSetModels`;
- optional `newSet2Models`.

Missing optional variants fall back to the normal model.

`LEGACY_ARMOR_MODEL_ROW_COUNT` is not the class count and should not be increased merely because a new class exists.

## 6. Equipment and generic combat policy

Declare reusable class policy rather than adding runtime class switches:

- `preferredWeaponTypes`;
- `defaultWeaponType`;
- item-specialization affinity behavior;
- optional visual model slots;
- critical-scaling attribute;
- attack anchor and other truly class-level animation compatibility rules;
- item requirement revision percentages, if specialized items use them.

Concrete skill effects remain in the skill subsystem.

## 7. Models and motions

Set a canonical `animation.legacyMotionJobMask` and any historical content aliases used by model `.ini` files.

Remember:

```text
item specialization bit != motion applicability mask
```

The model loader resolves `*APPLICABLE_JOB` through the class registry. A new class should not require editing `BitMaskJobNames[]`.

## 8. Skills

Set:

- `skills.treeKey` — stable route/key for the class skill tree;
- `skills.legacyGroupCode` only if using the existing legacy skill-code layout;
- `usesLegacyTier5Slots` only if the class intentionally uses the existing T51–T54 protocol/handlers.

Do not make a new class inherit T5 behavior merely because its job number happens to be inside a numeric range.

A brand-new skill tree may still require new handlers/content in the skill subsystem. That is an explicit feature integration, not a reason to modify the generic character system.

## 9. Ranks and item parser tokens

Define each rank with:

- rank number;
- specialization bit;
- stable key;
- display key;
- optional historical aliases.

Tokens are normalized by case and by spaces/hyphens/underscores. Conflicting normalized tokens fail registry validation.

The item parser no longer requires a `JobDataBase` row to recognize class/rank identity.

## 10. Generic presentation

Provide only the presentation assets actually supported by the current UI/content:

- kill notification icon;
- party alive/dead icons;
- SOD rank icon;
- ranking icon;
- optional legacy help/master-title keys.

Missing optional legacy presentation must fail safely; it should not cause `jobCode - 1` indexing.

## 11. Optional legacy content integrations

These are **opt-in content contracts**, not requirements for the class to exist:

- low-level present-item row (onboarding adapter);
- old quest reward mappings;
- historical 20/40/60 change-job quest participation;
- onboarding gifts;
- old manufacture/commerce choices;
- Bless Castle target crystals;
- class-specific event kits;
- old localized Castle target strings.

If an old system has no canonical content for the new class, leave that integration unsupported until content is defined. Do not guess.

## 12. Validation checklist

Before setting `enabled=true`:

- registry `Validate()` passes;
- class-domain smoke test contains the new definition or a representative equivalent;
- architecture guard passes;
- no new `JOB_CODE` numeric-range inference was introduced;
- no new `jobCode - 1` indexing was introduced;
- no new `MAX_JOBNUM` consumer was introduced;
- no new parallel class-name/bit table was introduced;
- client/server compile and link in MinGW i686;
- create/select/login/save/relog smoke passes;
- body/head/armor/equipment smoke passes;
- item specialization parser/display passes;
- motion `.ini` applicability passes;
- skill tree and rank progression pass;
- optional events/quests explicitly tested only where supported.

## Reserved Assassin

The current source contains a reserved Assassin (`legacyJobCode = 9`) but it remains `enabled=false`. Enabling it requires canonical job/rank bits, stats, models, armor/equipment contracts, skill tree, localization and persistence/runtime validation. Its reserved entry is intentionally not evidence that those values are known.


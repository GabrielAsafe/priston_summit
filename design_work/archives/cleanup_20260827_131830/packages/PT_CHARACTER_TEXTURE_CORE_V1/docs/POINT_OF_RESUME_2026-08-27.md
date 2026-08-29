# POINT OF RESUME — Summit Tale Blender Tools
## 2026-08-27 — after Character Asset V1 validation

If a new conversation starts, use this file as the primary continuation state.

## Current milestone

```text
CHARACTER C01 GEOMETRY CORE = RUNTIME PROVEN
CHARACTER ASSET HIGH-LEVEL WORKFLOW = PROVEN
```

Runtime-proven geometry includes:

```text
artist free topology
rigid game normalization
single-object generic writer
multi-object atomic writer
HIGH / DEFAULT / LOW
9 C01 LOD blocks in one SMD
UV / material assignment / Physique
independent reimport
INX LOD selection
registry + iterative deploy
```

Important production rule:

```text
--force-test = engineering only
PT_REVIEW_REQUIRED = production policy
```

The previously observed deformation was confirmed to be caused by the
TEST-ONLY forced rigid-skin heuristic; after correcting the rig, the deformation
was gone. This is not a serializer defect.

## Approved C01 all-LODs build

```text
size   = 382625
sha256 = 9b4a992945b7d0ce30aaff63b1141433132ecf57e17a4036177c9d94d8fa5b9c
```

## Character Asset

```text
asset_set_id = CHARSET-PIKEMAN-C01-M5
profile_id   = pikeman_c01_m5
```

Registered components:

```text
model       AST-791F1CAE2A4C  char/tmABCD/MfbC01.smd
skeleton    AST-853CFCE8E046  char/tmABCD/m5.smb
model_info  AST-33CB2B7B5F21  char/tmABCD/M5Bip.inx
texture     AST-B5C43FFBE0EE  char/tmABCD/MfbC01.bmp
variant     AST-439BC35674B2  char/tmABCD/MfbC03.bmp
aux texture AST-F3626D2C5F7D  char/tmABCD/mo-p-ac.BMP
```

High-level Character Asset operations already validated:

```text
inspect
sync-workspace
build
build --publish
deploy-plan
deploy
```

The publish/deploy validation ended idempotently with:

```text
reason=ALREADY_APPLIED
roundtrip_restore_check=PASS
```

## New V1.1

Install:

```bash
bash PT_CHARACTER_ASSET_V1_1/install_character_asset_v1_1.sh .
```

New commands:

```text
publish-build
status
```

`publish-build` publishes an existing PASS manifest without invoking Blender.

## Next feature

Do not continue proving C01 geometry.

Start:

```text
MATERIAL + TEXTURE WORKFLOW
```

Initial target is the registered character texture:

```text
char/tmABCD/MfbC01.bmp
AST-B5C43FFBE0EE
```

Existing historical evidence says protected BMP decode/edit/reprotect has already
worked in runtime, but this must now be integrated into Character Asset as a
production workflow with manifest, backup, validation and roundtrip publication.


---

## Texture Core V1 started

Package:

```text
PT_CHARACTER_TEXTURE_CORE_V1
```

New Character Asset entrypoint:

```text
pt_character_asset_v1_2.py
```

Initial asset:

```text
role=texture
AST-B5C43FFBE0EE
char/tmABCD/MfbC01.bmp
```

Architecture:

```text
registered source BMP = immutable structural baseline
artist edits standard decoded BMP
build copies pixel payload only
all non-pixel bytes remain exact baseline bytes
```

Immediate resume commands:

```bash
python3 pt_character_asset_v1_2.py texture-inspect \
  --workspace ./blender_workspace_pikeman \
  --role texture

python3 pt_character_asset_v1_2.py texture-prepare \
  --workspace ./blender_workspace_pikeman \
  --role texture
```

Required before proceeding:

```text
texture_inspect_check=PASS
texture_prepare_check=PASS
noop_byte_identical=True
```

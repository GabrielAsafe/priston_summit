# V0.5.4 Local Test Results

## 1. Compile

```text
python3 -m compileall pt_asset_tool
PASS
```

## 2. Physical representative semantic parse

All 12 SMDs from `PT_MULTILAYER_TEXLINK_REPS.zip` were parsed with the V0.5.4 semantic parser.

Result:

```text
12 / 12 source_semantics = PASS
12 / 12 hTexture slot mapping errors = 0
```

Observed representative profile distribution by file:

```text
REFLEX_ADD                         7 files
SAME_TEXTURE_DYNAMIC_FORM         5 files
SECOND_UV_LIGHTMAP_MODULATE       2 files
DUPLICATE_TEXTURE_DEFAULT_STAGE   2 files
```

A file can contain more than one profile.

Material-profile occurrences in the 12 representatives:

```text
REFLEX_ADD                        25 materials
SECOND_UV_LIGHTMAP_MODULATE       8 materials
SAME_TEXTURE_DYNAMIC_FORM         6 materials
DUPLICATE_TEXTURE_DEFAULT_STAGE   2 materials
```

## 3. Exact hTexture mapping

Every physical TexLink in the 12 representatives matched:

```text
runtime_material_array_base
+ material_id * 320
+ 8
+ stage_index * 4
```

This agrees with the supplied SourceCode call to `AddTexLink(... &smMaterial[m].smTexture[stage] ...)`.

Negative test: one physical `hTexture` value in a copied chandelier SMD was changed by +12.

Result:

```text
source_semantics = FAIL
htexture_mapping_errors = 1
PASS (expected rejection)
```

## 4. Material-stage examples

### Distinct UV lightmap

`Field/Fall_Game/chandelier-ani01.smd`

```text
758 / 758 faces multilayer
758 / 758 stage-1 UV sets differ from stage 0
base textures + oLightingMap_03.bmp
TextureStageState = [0, 0]
TextureFormState  = [0, 0]
```

### Reflection/add

`Field/Iron/i2-bip04_ani.smd`

```text
stage 0 = object texture
stage 1 = env.bmp
stage 1 TextureStageState = D3DTOP_ADD (7)
stage 1 TextureFormState  = FS_REFLEX (5)
```

### Dynamic form

`char/monster/Morgon/Morgon.smd`

```text
same texture in stage 0 / stage 1
TextureFormState = [FS_SCROLL, FS_SCROLL]
```

`StartImage/Opening/king.smd` additionally demonstrated pairs such as:

```text
FS_SCROLL5 + FS_SCROLL3
FS_SCROLL4 + FS_SCROLL6
```

## 5. Semantic mini-corpus audit

The V0.5.4 corpus auditor was run against the 12 physical representative SMDs using a local resolver.

Result:

```text
status=PASS
candidate_files_expected=12
physical_files_audited=12
htexture_mapping_fail_files=0
mixed_chain_depth_same_material_occurrences=0
two_stage_semantics_complete_candidate=true
```

The generated semantic representative ZIP contained the selected SMDs, manifest, and resolvable stage textures.

## 6. Same-material mixed-depth fail-closed test

A copied chandelier SMD was modified so one face of a two-stage material terminated after stage 0 while the remaining faces kept two stages.

Parser result:

```text
mixed_chain_depth_material_count=1
chain_depth_histogram={'1': 1, '2': 429}
```

The V0.5.4 visual validator is wired to refuse this condition rather than flattening it silently.

## 7. Blender UV mapping mock

The Blender postprocessor's UV assignment was executed with a mocked mesh whose loop order was deliberately different from the SMD corner order.

Result:

```text
PT_TEXLINK_STAGE_0 assigned by vertex index: PASS
PT_TEXLINK_STAGE_1 assigned by vertex index: PASS
V-coordinate conversion: PASS
pt_texlink_depth=2: PASS
```

This proves the postprocessor does not assume Blender polygon corner order equals source SMD corner order.

## 8. Real Blender boundary

This environment does not contain the user's Blender/Flatpak runtime. The Python modules compile and the source/UV logic has been tested locally, but material-node appearance still requires the user's Blender 5.1 visual gate.

V0.5.4 therefore does not claim Multilayer TexLink CLOSED yet.

## 9. Protected BMP overlay regression

Several `REFLEX_ADD` representatives use `env.bmp` in the proprietary `BMP_PRISTON_PREFIX14_DIB40_RGB24` family. Passing those bytes directly to Blender would produce a false material failure.

Using the existing proven `pt_character_texture_v1_4.py` decoder, V0.5.4 now converts only the temporary preview-overlay copy to a standard BMP header while preserving every byte after offset 14.

Real representative:

`char/monster/Bguardian/Bguardian.smd`

Result:

```text
texture overlay ready = true
required textures = 7
required failures = 0
env.bmp source header != BM
overlay env.bmp header = BM
source client bytes unchanged = true
status = BMP_PROTECTED_DECODED
```

The resulting overlay BMP was independently opened as a 128x128 RGB image. The source protected BMP remained byte-untouched.

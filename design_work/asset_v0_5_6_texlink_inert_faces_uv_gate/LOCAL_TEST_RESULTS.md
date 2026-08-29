# V0.5.6 Local Test Results

## Compile

```text
python3 -m compileall pt_asset_tool history_py/pt_smd_blender_import_v3_static_armature.py
PASS
```

## Physical `dun-4-ani-33.smd` anomaly

Using the user-supplied `PT_MULTILAYER_TEXLINK_SEMANTIC_REPS_V055.zip`:

```text
ani-31 face 10 material=0 depth=1
ani-31 face 11 material=0 depth=1
ani-32 face 10 material=0 depth=1
ani-32 face 11 material=0 depth=1

material 0:
TextureCounter=0
AnimTexCounter=0
textures=[]
```

All four serialized links map exactly to material 0 / smTexture[0].

Supplied SourceCode runtime evidence:

```text
smRENDER3D::RenderD3D(int MatNum)
if(smRendMaterial[MatNum].TextureCounter == 0 &&
   smRendMaterial[MatNum].AnimTexCounter == 0)
    return FALSE;
```

V0.5.6 result:

```text
Field/dungeon/dun-4-ani-33.smd
source_semantics=PASS
htexture_mapping_errors=0
stage_index_errors=0
inert_zero_texture_texlink_faces=4
PASS
```

## Representative semantic regression

All 10 SMDs physically present in the V055 semantic bundle were parsed:

```text
10/10 source_semantics=PASS
0 hTexture failures
0 stage-index failures
4 inert zero-texture TexLink faces (all in dun-4-ani-33)
PASS
```

A reconstructed corpus audit over those 10 physical files reports:

```text
status=PASS
htexture_mapping_fail_files=0
stage_index_fail_files=0
source_semantics_fail_files=0
inert_zero_texture_texlink_faces=4
two_stage_semantics_complete_candidate=true
PASS
```

## UV transport verifier unit test

The new source->Blender verification function was loaded with a stub `bpy`
module and exercised on a synthetic triangle:

```text
correct UV0/UV1 + depth -> PASS
stage1 U perturbed by 0.01 -> FAIL (mismatch_count=1)
PASS
```

## Existing V0.5.5 behavior retained

The V0.5.5 case-insensitive INX logical NodeName LOD fix is included unchanged.
No validators were made permissive for real missing/ambiguous LOD names.

## Boundary

No Blender/Flatpak runtime is available in this execution environment. The
actual Blender visual gate and the authoritative 121-file corpus audit must run
on the user's machine. No client file was modified.

# V0.5.5 Local Test Results

## Compile

```text
python3 -m compileall pt_asset_tool history_py/pt_smd_blender_import_v3_static_armature.py
PASS
```

## Bguardian runtime-name LOD regression

Physical inputs from the supplied debug bundle were parsed directly.

Observed:

```text
SMD logical names:
BGuardian_h x3
BGuardian_m x3

INX high:    Bguardian_h
INX default: Bguardian_m
```

With Blender-style duplicate names simulated (`.001`, `.002`):

```text
default visible = BGuardian_m, BGuardian_m.001, BGuardian_m.002
high visible    = BGuardian_h, BGuardian_h.001, BGuardian_h.002
PASS
```

## Morgon regression

```text
INX default = Morgon_m
visible     = Morgon_m
PASS
```

## Fail-closed regression

A synthetic INX group referencing `DefinitelyMissing` still raises:

```text
ValueError: INX default group references missing SMD objects
PASS
```

## Multilayer semantic regression

All 12 SMDs from the supplied `PT_MULTILAYER_TEXLINK_REPS.zip` were reparsed:

```text
12 / 12 source_semantics = PASS
PASS
```

## Semantic failure counter regression

A synthetic exact-hTexture + stage-index-error report produces:

```text
htexture_mapping_fail_files = 0
stage_index_fail_files       = 1
source_semantics_fail_files  = 1
two_stage_semantics_complete_candidate = false
```

and the failing source is retained in representative selection.

```text
PASS
```

## Boundary

No Blender/Flatpak runtime is available in this execution environment. The real Blender visual gate must be run on the user's machine. No client file was modified.

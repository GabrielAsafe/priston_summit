# Pikeman Blender Material Round-Trip V3 — BlendType

## Status entering V3

```text
SelfIllum = runtime-proven
TwoSide   = runtime visual accepted
NO-OP     = byte-identical
```

`Transparency` is deliberately deferred because source proves it is coupled to
render ordering, Z-write, and MeshState generation.

V3 adds one isolated field:

```text
pt_blend_type
```

Source-defined values:

```text
0 SMMAT_BLEND_NONE
1 SMMAT_BLEND_ALPHA
2 SMMAT_BLEND_COLOR
3 SMMAT_BLEND_SHADOW
4 SMMAT_BLEND_LAMP
5 SMMAT_BLEND_ADDCOLOR
6 SMMAT_BLEND_INVSHADOW
```

Runtime gate:

```text
material 0 / TmbC03.bmp:
BlendType 1 -> 4
SelfIllum remains 0
TwoSide remains 0

material 1 remains unchanged
```

For `LAMP`, source sets:

```text
ALPHABLENDENABLE = TRUE
SRCBLEND  = SRCALPHA
DESTBLEND = ONE
```

This should produce a strong additive appearance on the C03 chest.

## Headless workflow — no Blender UI required

### 1. Bind V3 properties

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_bind_v3.py -- \
  TmbC01.smd \
  blender/pikeman_c01_m4/armor_family/C002_material_blendtype_base_v3.blend
```

### 2. Set material 0 BlendType to LAMP automatically

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_material_blendtype_base_v3.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_set_v3.py -- \
  0 pt_blend_type 4 \
  blender/pikeman_c01_m4/armor_family/C002_material_blendtype_lamp_v3.blend
```

### 3. Export

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_material_blendtype_lamp_v3.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_export_v3.py -- \
  logs/characters/pikeman_c01_m4/materials/C002_material_blendtype_lamp_v3_edit_manifest.json
```

Expected:

```text
MATERIAL 0 self_illum=0.0 two_side=0 blend_type=4
MATERIAL 1 self_illum=0.0 two_side=0 blend_type=1
export_check=PASS
```

### 4. Build

```bash
python3 pt_smd_material_build_from_blender_v3.py \
  --edit-manifest logs/characters/pikeman_c01_m4/materials/C002_material_blendtype_lamp_v3_edit_manifest.json
```

Expected:

```text
MATERIAL 0 ... BlendType 1 -> 4
MATERIAL 1 ... BlendType 1 -> 1
outside_allowed_field_changed_byte_count=0
file_size_preserved=True
runtime_installed=False
build_check=PASS
```

Then run the independent material audit and V3 static-armature reimport before
any deploy.

Do not deploy until those gates pass.

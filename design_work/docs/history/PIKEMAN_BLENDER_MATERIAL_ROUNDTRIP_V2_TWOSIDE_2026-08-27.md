# Pikeman Blender Material Round-Trip V2 — TwoSide

V1 is closed:

```text
NO-OP Blender round-trip = BYTE-IDENTICAL PASS
SelfIllum Blender round-trip = BYTE-IDENTICAL to runtime-proven artifact
```

V2 adds exactly one new editable Summit field:

```text
pt_two_side
```

Source-proved runtime meaning:

```text
TwoSide == 0  -> normal culling
TwoSide != 0  -> D3DRS_CULLMODE disabled / both sides rendered
```

Authoring V2 constrains the property to `0` or `1`.

## Create artist blend

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_bind_v2.py -- \
  TmbC01.smd \
  blender/pikeman_c01_m4/armor_family/C002_material_twoside_artist_v2.blend
```

Open the blend and select:

```text
SMD_MAT_00_TmbC03.bmp
```

Change only:

```text
pt_two_side: 0 -> 1
```

Keep:

```text
pt_self_illum = 0.0
```

Save.

## Export

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_material_twoside_artist_v2.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_export_v2.py -- \
  logs/characters/pikeman_c01_m4/materials/C002_material_twoside_artist_v2_edit_manifest.json
```

Required:

```text
MATERIAL 0 self_illum=0.0 two_side=1
MATERIAL 1 self_illum=0.0 two_side=0
export_check=PASS
```

## Build

```bash
python3 pt_smd_material_build_from_blender_v2.py \
  --edit-manifest logs/characters/pikeman_c01_m4/materials/C002_material_twoside_artist_v2_edit_manifest.json
```

Required:

```text
MATERIAL 0 SelfIllum 0.0 -> 0.0 TwoSide 0 -> 1
MATERIAL 1 SelfIllum 0.0 -> 0.0 TwoSide 0 -> 0
outside_allowed_field_changed_byte_count=0
file_size_preserved=True
runtime_installed=False
build_check=PASS
```

Do not deploy yet. Run the independent material audit and V3 SMD reimport first.

Runtime observation note:
TwoSide is a culling property, so unlike SelfIllum it may not change the outside
appearance of a closed surface. The runtime gate should inspect views where the
inside/back faces of the C03 chest become visible. Absence of a dramatic outside
change is not a failure by itself.

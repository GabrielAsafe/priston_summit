# Pikeman Blender Material Round-Trip V1

## Goal

Move the first runtime-proven Summit material field into the artist workflow:

```text
SMD SelfIllum
→ Blender material custom property
→ artist edit
→ edit manifest
→ strict SMD build
```

V1 deliberately exposes only:

```text
pt_self_illum
```

as editable.

Other decoded Summit material properties are imported into Blender as custom
properties but remain read-only by policy in this version.

## 1. Bind Summit material properties into a clean C002 Blender preview

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_bind_v1.py -- \
  TmbC01.smd \
  blender/pikeman_c01_m4/armor_family/C002_material_artist_v1.blend
```

Expected:

```text
materials_bound=2
editable_fields=['pt_self_illum']
material_bind_check=PASS
```

Open the output `.blend`.

In Material Properties > Custom Properties, select:

```text
SMD_MAT_00_TmbC03.bmp
```

and change:

```text
pt_self_illum = 0.0
```

to a controlled value, e.g.:

```text
pt_self_illum = 1.0
```

Do not change the `pt_*` identity/read-only fields.

## 2. Export material edit manifest from the artist blend

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_material_artist_v1.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_export_v1.py -- \
  logs/characters/pikeman_c01_m4/materials/C002_material_artist_v1_edit_manifest.json
```

## 3. Build SMD from the Blender material manifest

```bash
python3 pt_smd_material_build_from_blender_v1.py \
  --edit-manifest logs/characters/pikeman_c01_m4/materials/C002_material_artist_v1_edit_manifest.json
```

Strict guarantees:

```text
registered source SHA required
only SelfIllum fields may change
zero bytes may change outside those 4-byte fields
file size must remain identical
runtime_installed=False
build_check=PASS
```

This V1 is the first true:

```text
Blender Material
→ Summit smMATERIAL
→ SMD
```

authoring round-trip.

Do not deploy the generated SMD until the independent material audit and SMD
reimport pass, using the same gates already proven in the direct SelfIllum test.

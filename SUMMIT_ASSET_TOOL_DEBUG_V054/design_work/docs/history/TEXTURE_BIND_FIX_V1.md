# Texture Bind Fix V1

The real BMP codec gates passed. The failure was Blender preview binding only.

V2 resolution order:

1. existing image basename/path;
2. SMD global material IDs from the Character Asset profile.

For `role=texture`, the profile supplies material slots `[1, 2]`.

The fallback scans actual polygon `material_index` usage and resolves the material
datablock through each mesh's own material slots. It does not trust global
`bpy.data.materials` ordering.

Retry:

```bash
python3 pt_character_asset_v1_3.py texture-bind-blend \
  --role texture \
  --artist blender/MfbC01_artist_free_mesh_v1.blend \
  --out blender/pikeman_c01_texture_edit_v1.blend
```

Expected:

```text
resolution_strategy=SMD_MATERIAL_SLOT
requested_material_slots=[1, 2]
materials_verified=>0
artist_master_overwritten=False
texture_bind_blend_check=PASS
```

If needed, read-only diagnostic:

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/MfbC01_artist_free_mesh_v1.blend \
  --python pt_blender_material_texture_probe_v1.py
```

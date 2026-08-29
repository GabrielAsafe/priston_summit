# PT Texture Runtime Gate V1

Creates a separate standard-BMP test texture with a global red-channel increase.

It never overwrites the editable source BMP. This is useful because the untouched
`MfbC01_editable.bmp` remains an exact restore source.

## Create test texture

```bash
python3 pt_bmp_visible_test_patch_v1.py \
  --in blender/textures/pikeman_c01_m5/texture/MfbC01_editable.bmp \
  --out blender/textures/pikeman_c01_m5/texture/MfbC01_editable_runtime_gate_v1.bmp \
  --amount 72
```

## Bind it into a Blender copy

```bash
python3 pt_character_asset_v1_3.py texture-bind-blend \
  --role texture \
  --artist blender/MfbC01_artist_free_mesh_v1.blend \
  --texture blender/textures/pikeman_c01_m5/texture/MfbC01_editable_runtime_gate_v1.bmp \
  --out blender/pikeman_c01_texture_runtime_gate_v1.blend
```

## Build game texture

```bash
python3 pt_character_asset_v1_3.py texture-build \
  --role texture \
  --edited blender/textures/pikeman_c01_m5/texture/MfbC01_editable_runtime_gate_v1.bmp \
  --name MfbC01_texture_runtime_gate_v1
```

## Restore source after runtime test

Because `MfbC01_editable.bmp` was not modified:

```bash
python3 pt_character_asset_v1_3.py texture-build \
  --role texture \
  --edited blender/textures/pikeman_c01_m5/texture/MfbC01_editable.bmp \
  --name MfbC01_texture_restore_source_v1
```

That build must return the registered source SHA:
`4c97f923db485407b3fe1fcd77e7c36ada36ea2e22255b93937332ada3ad4e18`.

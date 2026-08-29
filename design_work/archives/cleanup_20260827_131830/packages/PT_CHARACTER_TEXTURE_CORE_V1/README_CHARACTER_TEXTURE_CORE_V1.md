# PT Character Texture Core V1

First production-oriented texture stage for the Character Asset workflow.

## Initial target

```text
role=texture
asset_id=AST-B5C43FFBE0EE
target=char/tmABCD/MfbC01.bmp
material_slots=[1,2]
```

The protected BMP family used by this gate is not generalized beyond what was
already proven:

```text
14 proprietary bytes replacing BITMAPFILEHEADER
40-byte BITMAPINFOHEADER
256x256
24 bpp
BI_RGB
196662 bytes for MfbC01.bmp
```

## Core safety invariant

The game build is NOT made by trusting a newly written BMP header.

Instead:

```text
registered source game BMP
        +
pixel payload from artist standard BMP
        ↓
game BMP
```

Every byte outside the pixel payload remains byte-identical to the registered
source. This preserves the proprietary 14-byte prefix and DIB exactly.

## Gate 1 — inspect

```bash
python3 pt_character_asset_v1_2.py texture-inspect \
  --workspace ./blender_workspace_pikeman \
  --role texture
```

Read-only.

## Gate 2 — prepare + no-op roundtrip

```bash
python3 pt_character_asset_v1_2.py texture-prepare \
  --workspace ./blender_workspace_pikeman \
  --role texture
```

Creates an immutable registered-source baseline and a standard editable BMP.

Expected:

```text
noop_byte_identical=True
texture_prepare_check=PASS
```

No workspace/client write occurs.

## Optional — bind editable texture into a copy of the Blender character

```bash
python3 pt_character_asset_v1_2.py texture-bind-blend \
  --role texture \
  --artist blender/MfbC01_artist_free_mesh_v1.blend \
  --out blender/pikeman_c01_texture_edit_v1.blend
```

The master is not overwritten.

## Build

After editing the standard BMP:

```bash
python3 pt_character_asset_v1_2.py texture-build \
  --role texture \
  --name MfbC01_texture_gate_v1
```

The validator requires unchanged dimensions/bpp/compression and preserves every
non-pixel baseline byte exactly.

## Publish / deploy

```bash
python3 pt_character_asset_v1_2.py texture-publish \
  --workspace ./blender_workspace_pikeman \
  --role texture \
  --replace-published

python3 pt_character_asset_v1_2.py texture-deploy-plan \
  --workspace ./blender_workspace_pikeman \
  --role texture

python3 pt_character_asset_v1_2.py texture-deploy \
  --workspace ./blender_workspace_pikeman \
  --role texture
```

Publish modifies only the workspace. Deploy remains protected by Restore V3.

# Pikeman Blender Material Round-Trip V4 — Blink Color

Source-proven composite semantic:

```text
sMATS_SCRIPT_BLINK_COLOR = 0x4000

UseState & 0x4000
→ SetBlinkColor(sColors, Diffuse, WindMeshBottom)
→ SetD3DRendBuff3(...)
→ SetBlinkColorVertex(...)

Diffuse = smFCOLOR { float r,g,b }

BLINK_TIME1..8
→ WindMeshBottom 9..16

no explicit BLINK_TIME
→ default WindMeshBottom = 9
```

Important: `Diffuse` is NOT exposed as a static diffuse/material color. In this
renderer it is the RGB amplitude for BLINK_COLOR.

V4 authoring properties:

```text
pt_blink_color_enabled
pt_blink_r
pt_blink_g
pt_blink_b
pt_blink_time   # 1..8
```

The builder:
- sets/clears only UseState bit 0x4000;
- preserves all other UseState bits;
- maps BLINK_TIME1..8 to WindMeshBottom 9..16 when enabled;
- resets WindMeshBottom to 0 when disabled for this registered Pike baseline;
- patches only approved material fields;
- rejects any changed byte outside allowed ranges;
- preserves file size.

## Gate build

Create V4 blend:

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_bind_v4.py -- \
  TmbC01.smd \
  blender/pikeman_c01_m4/armor_family/C002_material_blink_base_v4.blend
```

Set material 0 to a source-proven red blink, using BLINK_TIME1/default code 9:

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_material_blink_base_v4.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_set_blink_v4.py -- \
  0 1 1.0 0.0 0.0 1 \
  blender/pikeman_c01_m4/armor_family/C002_material_blink_red_v4.blend
```

Export:

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_material_blink_red_v4.blend \
  --python-exit-code 1 \
  --python pt_blender_summit_material_export_v4.py -- \
  logs/characters/pikeman_c01_m4/materials/C002_material_blink_red_v4_edit_manifest.json
```

Build:

```bash
python3 pt_smd_material_build_from_blender_v4.py \
  --edit-manifest logs/characters/pikeman_c01_m4/materials/C002_material_blink_red_v4_edit_manifest.json
```

Required material 0 semantic result:

```text
SelfIllum=0
TwoSide=0
BlendType=1
Diffuse=[1.0,0.0,0.0]
UseState=16384 / 0x4000
WindMeshBottom=9 / BLINK_TIME1
```

Material 1 remains source baseline.

Do NOT deploy before:
1. independent material audit;
2. V3 structural SMD reimport.

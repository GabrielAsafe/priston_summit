# Pikeman SelfIllum Runtime Gate V1

## Source audit result

The current source proves:

```cpp
struct smMATERIAL {
    ...
    DWORD BlendType;
    DWORD Shade;
    DWORD TwoSide;
    DWORD SerialNum;
    smFCOLOR Diffuse;
    float Transparency;
    float SelfIllum;
    ...
};
```

Blend constants:

```text
SMMAT_BLEND_NONE      = 0
SMMAT_BLEND_ALPHA     = 1
SMMAT_BLEND_COLOR     = 2
SMMAT_BLEND_SHADOW    = 3
SMMAT_BLEND_LAMP      = 4
SMMAT_BLEND_ADDCOLOR  = 5
SMMAT_BLEND_INVSHADOW = 6
```

Renderer:

```text
SelfIllum != 0
→ SetD3DRendBuff2(...)
→ vertex RGB forced white
→ alpha preserved
```

`TwoSide != 0` sets `D3DCULL_NONE`; otherwise `D3DCULL_CW`.

`Transparency` is float and values above `0.2f` affect Z-write behavior.

Therefore the first controlled material gate is:

```text
TmbC01.smd
material 0
texture TmbC03.bmp
SelfIllum 0.0f -> 1.0f
```

This is chosen because C002 uses material 0 for the C03 body while its C01 arms
remain on material 1 as an internal visual control.

## Build

```bash
python3 pt_pikeman_smd_selfillum_build_v1.py
```

Required:

```text
source_sha256=9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c
material_id=0
material_texture=char/tmABCD/TmbC03.bmp
old_value=0.0
new_value=1.0
target_field_size=4
outside_target_field_changed_byte_count=0
file_size_preserved=True
material_region_end_check_before=PASS
material_region_end_check_after=PASS
semantic_changes_except_target=0
runtime_installed=False
build_check=PASS
```

Note: writing float 1.0f changes only the non-zero bytes inside the four-byte
field, so `changed_byte_count` may be less than four. The strict invariant is
that no byte outside the four-byte SelfIllum field changes.

## Independent material audit

```bash
python3 pt_pikeman_smd_material_audit_v1.py \
  --smd pt_build/characters/pikeman_c01_m4/materials/TmbC01_material0_selfillum_runtime_gate_v1.smd \
  --report logs/TmbC01_material0_selfillum_runtime_gate_v1_material_audit.json
```

Material 0 must report:

```text
SelfIllum_raw_f32=1.0
```

and the region must close exactly.

## Independent SMD reimport

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background \
  --python-exit-code 1 \
  --python history_py/pt_smd_blender_import_v3_static_armature.py -- \
  --smd pt_build/characters/pikeman_c01_m4/materials/TmbC01_material0_selfillum_runtime_gate_v1.smd \
  --skeleton m4.smb \
  --model-info C002.inx \
  --lod default \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --out blender/pikeman_c01_m4/armor_family/C002_selfillum_runtime_gate_v1_reimport.blend \
  --no-pack
```

Required:

```text
static_armature_import_check=PASS
```

## Publish to registered model slot

Only after build + audit + reimport pass:

```bash
python3 summit_workspace_asset_bridge_v1_2.py \
  publish-file ./blender_workspace_pikeman_real \
  --build-manifest logs/characters/pikeman_c01_m4/materials/TmbC01_material0_selfillum_runtime_gate_v1_build_manifest.json \
  --target char/tmABCD/TmbC01.smd
```

This changes the workspace only.

Then use Restore V4:

```bash
python3 summit_roundtrip_restore_v4.py \
  ./blender_workspace_pikeman_real \
  --target char/tmABCD/TmbC01.smd
```

Dry-run must identify a managed candidate before `--apply`.

Do not restore or republish the source until the runtime visual test has been
performed.

## Runtime visual expectation

Use Pikeman armor visual index 2 / C002.

Expected:

```text
C03 body / chest → self-illuminated / bright-white influenced
C01 arms         → normal control
no crash
normal animation
```

After the runtime gate, rebuild/publish the exact registered source and deploy
it back using Restore V4.

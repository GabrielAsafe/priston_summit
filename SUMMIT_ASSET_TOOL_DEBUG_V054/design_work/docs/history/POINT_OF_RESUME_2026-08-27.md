# POINT OF RESUME — 2026-08-27

## Critical identity correction

```text
Priestess:
MfbC01.smd + m5.smb + M5Bip.inx

Pikeman:
TmbC01.smd + m4.smb + M4Bip.inx
```

Do not mix the two profiles/workspaces.

## Real Pikeman structural state

```text
profile_id   = pikeman_c01_m4
asset_set_id = CHARSET-PIKEMAN-C01-M4

TmbC01.smd SHA =
9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c

m4.smb SHA =
48af614678593eaa0b659c2be514bd60f86b94f72cfb4305ed8352f48f12cc4a

M4Bip.inx SHA =
b8bb4adaaa2175b57fa604ccdacf419ca699d48d47bb04710e6169189de2f192
```

LOD:

```text
HIGH    TbhC01 TahC01
DEFAULT TbmC01 TamC01
LOW     TblC01 TalC01
```

Materials:

```text
material 1 -> char/tmABCD/TmbC01.bmp
material 0 -> char/tmABCD/TmbC03.bmp
```

## Texture status

`TmbC01.bmp`:

```text
inspect                  PASS
protected decode          PASS
no-op byte-identical      PASS
Blender binding           PASS
visible artist edit       PASS
game BMP rebuild          PASS
workspace publish         PASS
deploy dry-run            PASS
verified deploy           PASS
runtime visual            PASS
```

Official:

```text
PIKEMAN TmbC01 TEXTURE WORKFLOW = RUNTIME PROVEN
```

## Immediate next action

Restore the original TmbC01 from the untouched editable baseline:

```bash
python3 pt_character_asset_v1_3.py \
  --profile profiles/pt_character_profile_pikeman_c01_m4_v1.json \
  texture-build \
  --role texture \
  --edited blender/textures/pikeman_c01_m4/texture/TmbC01_editable.bmp \
  --name TmbC01_texture_restore_source_v1
```

Then publish with `--replace-published`, dry-run, and deploy.

After restoration, continue with:

```text
role=texture_variant
target=char/tmABCD/TmbC03.bmp
material_slot=0
```

Do not return to Priestess geometry work unless explicitly requested.

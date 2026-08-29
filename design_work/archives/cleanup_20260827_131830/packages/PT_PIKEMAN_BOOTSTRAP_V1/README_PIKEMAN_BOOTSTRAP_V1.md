# PT Pikeman Bootstrap V1

This package converts the now-proven physical Pikeman identity chain into a
real Character Asset profile.

Known current-build identity:

```text
M4Bip.inx
  szModelFile  = char\tmABCD\TmbC01.ASE
  szMotionFile = char\tmABCD\m4.ase

physical:
  char/tmABCD/TmbC01.smd
  char/tmABCD/m4.smb

textures discovered:
  char/tmABCD/TmbC01.bmp
  char/tmABCD/TmbC03.bmp
```

## Run

```bash
python3 pt_pikeman_bootstrap_v1.py \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
```

The bootstrap:

1. reads High/Default/Low directly from M4Bip.inx;
2. creates verified local baseline copies:
   `TmbC01.smd`, `m4.smb`, `M4Bip.inx`;
3. imports the Pike with the existing V3 importer;
4. inventories every object/count/material;
5. measures polygon use by material ID;
6. maps material IDs to texture paths;
7. generates:

```text
profiles/pt_character_profile_pikeman_c01_m4_v1.json
```

No client file is modified.

## Critical status rule

The generated profile intentionally says:

```text
geometry_status = UNPROVEN for Pikeman
```

The Priestess runtime proofs are format/tool evidence, not Pikeman-specific
runtime proof.

## After PASS

Inspect the generated profile, then register the actual Pike as a new workspace:

```bash
python3 pt_character_asset_v1_3.py \
  --profile profiles/pt_character_profile_pikeman_c01_m4_v1.json \
  inspect \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --workspace ./blender_workspace_pikeman_real
```

A fresh workspace will be prepared/registered next; do not reuse the legacy
misnamed Priestess workspace for the Pike.

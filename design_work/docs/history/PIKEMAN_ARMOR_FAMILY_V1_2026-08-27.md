# Pikeman Armor Family V1

## Evidence state

The real client has now proven that C001..C005 are model-info configurations
for the Pikeman armor family:

```text
C001.inx
  model TmbC01
  HIGH    TbhC01 TahC01
  DEFAULT TbmC01 TamC01
  LOW     TblC01 TalC01

C002.inx
  model TmbC01
  HIGH    TbhC03 TahC01
  DEFAULT TbmC03 TamC01
  LOW     TblC03 TalC01

C003.inx
  model tmbC03
  HIGH    TbhC03 TahC03 TlhC03
  DEFAULT TbmC03 TamC03 TlmC03
  LOW     TblC03 TalC03 TllC03

C004/C005
  model tmbC03
  C03/C06 combinations
```

Source evidence establishes that `SetPlayerArmor()` selects the class row and
visual armor index from `szArmorNumFile`, then loads that logical model-info
configuration. The exact required level corresponding to each visual index
remains UNPROVEN.

## Purpose

Build faithful Blender previews for C001..C005 and measure which textures are
actually active in the DEFAULT LOD of each configuration.

This is the correct way to decide where `TmbC03.bmp` should be runtime-tested.

## Run

```bash
python3 pt_pikeman_armor_family_v1.py \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
```

No client file is modified.

Expected final gate:

```text
config_count=5
armor_family_check=PASS
client_modified=False
```

The output for C002/C003 should show whether `TmbC03.bmp` is used by the
actually selected DEFAULT LOD.

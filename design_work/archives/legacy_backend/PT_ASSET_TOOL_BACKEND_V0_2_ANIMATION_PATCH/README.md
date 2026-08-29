# PT Asset Tool Backend V0.2.0 — Animation gate

Incremental patch over installed backend V0.1.1.

## What it adds

- `pt_asset.py asset motions <asset>` lists active INX motions that resolve to SMB frame containers.
- `pt_asset.py validate animation <asset>` builds Blender Actions from source-equivalent SMB evaluation.
- Supports both known INX layouts through `pt_inx_motion_core_v2.py`: MODERN_172 and LEGACY_120.
- Uses the existing self-contained rigid SMD+SMB+INX resolver.
- Texture overlay from V0.1.1 remains active.
- Geometry is pristine during the animation gate: no geometry witness modification is applied.
- Separate human certifications for rig/deformation, selected animation, texture/material, and animation asset overall.
- Does not modify client files.

## Action naming

`PT_<STATE>_<INX_INDEX>` e.g. `PT_STAND_011`, `PT_ATTACK_012`.

`--load-actions selected` loads only the requested action.

`--load-actions core` (default) loads one representative action for common states such as STAND/WALK/RUN/ATTACK/DAMAGE/DEAD.

`--load-actions all` loads every active motion that has a valid SMB container. This is useful for mobs/NPCs with a small motion set. For player-class INX files with many motions, `core` is intentionally faster.

## Install

```bash
bash install_pt_asset_tool_backend_v0_2_animation_patch.sh \
  "$HOME/Área de Trabalho/summit/design_work"
```

## First real gate

```bash
cd ~/Área\ de\ Trabalho/summit/design_work
python3 pt_asset.py doctor
python3 pt_asset.py asset motions Monbagon-C.smd
python3 pt_asset.py validate animation Monbagon-C.smd \
  --motion ATTACK \
  --load-actions all
```

When Blender opens, press Space to play. Inspect deformation, active animation, textures/materials, then close Blender. The terminal asks three independent y/n/s questions.

Animation frame spacing is currently a preview mapping of one source logical frame to one Blender frame at 30 FPS. Motion transforms are source-evaluated; preview playback speed is not yet certified as runtime-exact.

## Failure logs

For Monbagon:

`pt_build/pt_asset_tool/AST-21DEF886F2A7/visual_animation/`

Important files:

- `01_import.log`
- `motion_bake_payload.json`
- `02_animation_bake.log`
- `animation_bake_report.json`
- `03_blender_gui.log`
- `summary.json`

## Safety

`client_modified=False`. The patch only reads SMD/SMB/INX/textures and writes build `.blend`, logs and certification records under `design_work`.

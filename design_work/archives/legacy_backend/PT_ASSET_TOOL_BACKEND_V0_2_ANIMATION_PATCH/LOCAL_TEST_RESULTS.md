# PT Asset Tool Backend V0.2.0 — local test results

## Status

Local non-Blender logic is PASS. Real Blender 5.1 bake/visual gate is pending on the user's machine because Blender/Flatpak is not installed in the assistant container.

## Tests completed

- Python syntax: PASS.
- Patch installer simulation over exact V0.1.1 files: PASS.
- Base V0.1.1 SHA guard: PASS.
- Installed version check: 0.2.0 PASS.
- Legacy INX LEGACY_120 motion enumeration (Monbagon): PASS.
- Modern INX MODERN_172 motion enumeration (Vampiric): PASS.
- Source-equivalent SMB pose generation on 6 physical representative chains: PASS.
  - Monbagon: selected STAND, active 4, core loaded 4, 25 bones.
  - Monshadow: selected ATTACK, active 2, core loaded 2, 55 bones.
  - Vampiric: selected STAND, active 5, core loaded 5, 32 bones.
  - SN-002: selected STAND, active 2, core loaded 1, 78 bones.
  - MN-005: selected STAND, active 3, core loaded 1, 59 bones.
  - MN-009: selected STAND, active 3, core loaded 2, 62 bones.
- Monbagon `--load-actions all`: WALK/STAND/ATTACK/DAMAGE generated: PASS.
- Bind-to-current transform identity check:
  source row-vector delta `B^-1 * G` versus Blender column-vector delta `G^T * (B^T)^-1` max numerical discrepancy < 6e-14: PASS.
- Parent-first pose assignment uses SMB `TmSort`: PASS by structural invariant from evaluator.
- End-to-end backend orchestration with simulated Blender process: PASS.
  - structural certification
  - texture prepare certification
  - action selection
  - rig/animation/texture prompts
  - certification persistence
  - `ANIM` status column
- Client modification during all local tests: false.

## Blender 5.x API compatibility work

The bake script does not use the removed Blender 5 legacy `Action.fcurves` API. It creates Action slots explicitly and uses `keyframe_insert()` on pose bones, which is compatible with the current slotted/layered Action model.

## Pending real gate

Run on the user's Blender 5.1 Flatpak:

```bash
python3 pt_asset.py validate animation Monbagon-C.smd --motion ATTACK --load-actions all
```

Expected loaded Actions:

- PT_WALK_010
- PT_STAND_011
- PT_ATTACK_012
- PT_DAMAGE_013

The `.blend` must preserve pristine geometry and textures, deform correctly through the Armature, and play the selected Action without exploding/shearing/disconnected skin.

Preview timing currently maps one source logical frame to one Blender frame at 30 FPS. Transform evaluation is source-equivalent; exact runtime playback-speed certification is a separate future gate.

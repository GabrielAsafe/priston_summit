# Summit Tale Asset Tool — V0.2.8 STAND Container Anchor

## Evidence behind this patch

The V0.2.7 segment audit for Monbagon showed:

- STAND: global range 43..93 (engine 6880..14880).
- A very large source-pose discontinuity from logical frame 41 -> 42 (~34.98 max matrix step).
- Immediately after that, frame 42 -> 43 is nearly continuous (~0.01269).
- The selected STAND itself starts at frame 43.
- Mid-segment evaluation did not change when frame_end moved by +/- one logical frame.
- Source rigidity of STAND was in the same small range as WALK/ATTACK, so scale/shear was not the differentiator.

Together this is evidence that logical frame 42 behaves like a motion-container reference/reset frame immediately before the STAND payload, while the backend had been skinning every source pose only against the model-wide bind transform.

## Change

For STAND only, V0.2.8 evaluates the reference pose at:

    anchor_logical = global_start - 1

and rebases each source bone matrix so the container anchor maps back to the model bind matrix.

The source matrices are row-major. The correction is:

    corrected_row = bind_row @ inverse(anchor_row) @ current_row

This is equivalent to using the motion-local delta `current @ inverse(anchor)` in the column-space skinning convention used by the Blender diagnostic.

### Conservative scope

- `STAND`: `CONTAINER_ANCHOR`
- `WALK`: unchanged (`GLOBAL_BIND`)
- `ATTACK`: unchanged (`GLOBAL_BIND`)
- No SMD/SMB/INX/client files are modified.
- Validators are not weakened.
- Existing certifications remain append-only.

This deliberately avoids changing WALK and ATTACK because both already passed human visual validation on V0.2.5.

## Extra visibility

Animation validation and animation diagnostic JSON now include:

- `source_pose_reference`
- `source_pose_anchor_engine`

For Monbagon STAND the expected values are:

    source_pose_reference = CONTAINER_ANCHOR
    source_pose_anchor_engine = 6720

The animation diagnostic prompt is also state-neutral and prints the selected action rather than always saying ATTACK.

## Recommended test

First run:

    python3 pt_asset.py diagnose animation Monbagon-C.smd --motion STAND

If SOURCE_DIRECT and BLENDER_ARMATURE are both visually coherent, run:

    python3 pt_asset.py validate animation Monbagon-C.smd --motion STAND --load-actions selected

If the rebase makes STAND worse, answer `n`; the installer created a timestamped `pt_asset_tool.backup_*` of V0.2.7 for immediate rollback.

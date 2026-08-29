# V0.5.1 Local Test Results

## Compile

`python3 -m compileall pt_asset_tool`: PASS.

## Legacy-affine source regression

The three physical representatives from `PT_OBJECT_LOCAL_RARE_REPS.zip` still
parse through the V0.5.0 raw non-unit quaternion evaluator:

- `dancesword.smd`: PASS, legacy affine objects=1
- `mg3_danceSd40_110_125.smd`: PASS, legacy affine objects=1
- `Mg3_Diastrophism24.smd`: PASS, legacy affine objects=12

No source evaluator/math code was changed by V0.5.1.

## Blender 5.1 API compatibility

The legacy-affine preview no longer assigns `ShapeKey.frame` and no longer uses
absolute `eval_time` authoring. It animates the writable `ShapeKey.value`
property of relative shape keys.

Static source audit confirms no remaining write to `.frame` in the preview
implementation.

## Crossfade invariant

Synthetic non-uniform sample frames `[1.0, 2.5, 5.0, 9.0]` were tested. At
0%, 25%, 50%, 75% and 100% across every adjacent interval:

- all weights remained within `[0,1]`;
- active weights summed to exactly 1 within floating-point tolerance;
- exact sample frames selected exactly one sampled shape at weight 1.

Result: `crossfade_partition_of_unity=PASS`.

## Blender runtime limitation of this environment

This container has no Blender/bpy executable, so the final real Blender 5.1
visual gate must run on the user's installation. The exact V0.5.0 crash path is
removed and the replacement uses the documented writable shape-key value
animation path.

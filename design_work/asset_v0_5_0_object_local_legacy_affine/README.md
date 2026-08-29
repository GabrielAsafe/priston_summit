# Summit Tale Asset Tool V0.5.0 — Legacy Affine Object-Local Preview

Base: V0.4.9.

## Goal

Close the remaining object-local rotation subtype found in old effect SMDs where
serialized quaternion keys are intentionally/non-accidentally non-unit and the
client preserves the resulting affine matrix (scale/shear) instead of normalizing
it to a pure rotation.

## Proven source rule

For the physical representatives:

- `Effect/AssaEffect/Skill3Mesh/dancesword.smd`
- `Effect/AssaEffect/Skill3Mesh/mg3_danceSd40_110_125.smd`
- `Effect/AssaEffect/Skill3Mesh/Mg3_Diastrophism24.smd`

`TmPrevRot` is reproduced by accumulating the raw quaternion-to-matrix conversion
**without quaternion normalization**.  Maximum source error on the three reps is
below `2.1e-7`.

V0.4.9 normalized these matrices for Blender safety and therefore correctly
failed closed. V0.5.0 preserves the affine matrix instead.

## Blender preview representation

Normal object-local animation keeps V0.4.9 behavior:

`DELTA_TRANSFORMS_OVER_STATIC_BIND`

Legacy affine scenes use:

`AFFINE_WORLD_DELTA_ABSOLUTE_SHAPE_KEYS`

The evaluator reproduces the client row-matrix hierarchy, computes a full 4x4
world delta against the first preview frame, transposes it to Blender convention,
and applies it to the imported geometry through absolute shape-key samples.

Why shape keys: Blender Object TRS channels cannot represent arbitrary accumulated
shear. This representation preserves the complete sampled affine result for visual
validation without pretending that shear is a normalized rotation.

This is **preview-only**. It does not enable authoring/export of object-local curves.
The source SMD and client are never modified.

## Interpolation

The source `smOBJ3D::GetRotFrame` calls `D3DMath_QuaternionSlerp` from the zero
quaternion to the next incremental quaternion and multiplies that step by
`TmPrevRot`. V0.5.0 implements this compatibility path and samples every 160 engine
units (one preview frame).

Timing remains:

`PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED`

## Next verification

Run visual gates for the three physical legacy-affine representatives, then rerun
`audit object-local-corpus`. If all 432 physical candidates full-parse, object-local
format support can be closed; otherwise the audit will identify the remaining
subfamily.

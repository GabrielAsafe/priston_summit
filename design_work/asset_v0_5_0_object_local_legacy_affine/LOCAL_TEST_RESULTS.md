# V0.5.0 Local Test Results

## Physical parser corpus used locally

19 physical object-local SMD representatives accumulated across previous debug
bundles were parsed with `strict_v1=True`.

Result: **19/19 PASS**.

This includes single-object, flat multi, parent/static-child, animated child,
deep hierarchy, unresolved runtime helper roots, and the three legacy affine reps.

## Legacy affine source reproduction

| File | Legacy affine objects | max raw accumulation error | max normalized-path error |
|---|---:|---:|---:|
| `dancesword.smd` | 1 | `2.085103e-7` | `0.001535402` |
| `mg3_danceSd40_110_125.smd` | 1 | `9.610536e-8` | `0.052782714` |
| `Mg3_Diastrophism24.smd` | 12 | `1.730282e-7` | `0.197442928` |

Every serialized `TmPrevRot` keyframe is reproduced exactly by the new evaluator
at the stored keyframe (`key_err=0` in the test harness).

Reference-frame normalized world affine deltas are identity within floating-point
error (`<= 1.35e-14` on the three reps; one multiobject scene reaches ~`9.8e-13`).

## Regression

Normalized source samples from V0.4.9 and V0.5.0 are byte-for-byte JSON-equivalent
for:

- `Test.smd`
- `Web.smd`
- `crystal_ani01.smd`
- `RotObj01.smd`
- `chandelier-ani01.smd`

So the normal V0.4.9 object-local path is unchanged.

## Static checks

- `python -m py_compile pt_asset_tool/object_local.py` PASS
- `python -m py_compile pt_asset_tool/blender/bake_object_local.py` PASS
- `python -m py_compile pt_asset_tool/validation.py` PASS

## Environment limitation

This environment does not provide Blender/bpy. The affine shape-key bake therefore
requires the user's real Blender visual gate. No claim of Blender PASS is made here.

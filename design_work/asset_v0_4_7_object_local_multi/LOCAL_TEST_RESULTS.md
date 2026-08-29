# V0.4.7 local test results

## Parser / hierarchy

All three supplied physical multi-object representatives parse with `strict_v1=True` and pass hierarchy validation:

| Asset | Objects | Animated | Parent edges | Animated children |
|---|---:|---:|---:|---:|
| `bong.smd` | 3 | 3 | 0 | 0 |
| `At3_SplitJavelin40.smd` | 2 | 1 | 1 | 0 |
| `3ani-04.smd` | 2 | 2 | 0 | 0 |

`At3_SplitJavelin40.smd` resolves `wt108 -> bip SJ01`. The fixed-point `Tm * parent.TmInvert` bind relation is near identity (`max error ~0.00627`, expected fixed-point inverse rounding).

## Blender orchestration simulation

A fake Blender object graph exercised the real V0.4.7 bake script:

- `bong`: all 3 roots received animation keys;
- `3ani-04`: both roots received animation keys;
- `At3_SplitJavelin40`: `wt108` received keys, `bip SJ01` received no local keys and was parented to `wt108`, proving inheritance orchestration.

## Negative test

A synthetic `bong` copy was modified so animated `Plane03` had `NodeParent=Plane01`. V0.4.7 detected `animated_child_count=1` and refused the bake with the expected fail-closed error requesting a physical animated-child representative.

## Regression

`python -m compileall pt_asset_tool` PASS.

No original SMD, client file, or certification history was modified by these tests.

## Test.smd texture regression

The physical `Test1.tga` is a standard RGBA 64×8 image. The material path is serialized with repeated backslashes. Before normalization this became a double-slash logical path and could fail Linux resolver lookup. V0.4.7 collapses repeated separators; with the real `ClientResolver`, the face-referenced material now resolves exactly one required texture with `required_failures=0` and `ready=true`.

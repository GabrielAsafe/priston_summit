# Summit Tale Asset Tool V0.5.1 — Blender 5.1 Legacy Affine Shape-Key Compatibility

V0.5.1 is a focused compatibility fix over V0.5.0.

## Bug fixed

Blender 5.1 exposes `ShapeKey.frame` as read-only. V0.5.0 used the old
absolute-shape-key authoring path and therefore failed before opening the
legacy-affine preview:

```
AttributeError: bpy_struct: attribute "frame" from "ShapeKey" is read-only
```

The source SMD parser/evaluator was already correct; the failure was only in
Blender preview authoring.

## New preview representation

The legacy-affine matrices are unchanged. Each sampled full 4x4 world-affine
delta is still applied to the source vertices. The Blender representation is
now:

```
relative sampled shape keys
+ animated ShapeKey.value
+ linear crossfade between adjacent samples
```

At an exact source sample, the corresponding shape key has weight 1. Between
samples i and i+1, shape i fades 1->0 while shape i+1 fades 0->1. Therefore the
weights form a partition of unity and linearly interpolate the same sampled
vertex positions without using `ShapeKey.frame` or `Key.eval_time`.

## Scope

This changes only legacy-affine Blender preview generation. Normal object-local
hierarchy, parser semantics, runtime matrix reconstruction, static roundtrip,
textures, player animation and all prior gates are unchanged.

Legacy-affine authoring/export of the animation curves themselves remains
unsupported; this is a visual/semantic preview path only.

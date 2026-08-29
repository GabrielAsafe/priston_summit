# PT Asset Tool 0.2.6 — Source Motion Audit

Incremental diagnostic patch over 0.2.5.

Adds:

```bash
python3 pt_asset.py diagnose motion-source Monbagon-C.smd --motion STAND
```

This is a source-only diagnostic. It does not open Blender and does not modify client files.
It evaluates the selected motion plus WALK/ATTACK baselines and measures whether
`current * inverse(bind)` remains a rigid transform (unit scale, orthogonal basis,
|determinant| ~= 1). This distinguishes source interpolation producing scale/shear
from a motion-segment/keyframe semantic issue with otherwise-rigid matrices.

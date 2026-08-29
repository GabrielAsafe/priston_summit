# PT Asset Tool Backend V0.2.1 Animation Fix

Fixes the first real Monbagon animation gate failure.

Changes:
- Correct backend interpretation of the legacy null quaternion operand as identity for the interpolation path.
- Keep historical evaluator file untouched; correction is scoped to backend loading.
- Enforce quaternion sign continuity before keying Blender pose channels.
- Force generated Action interpolation to LINEAR, including Blender 5 slot/channelbag actions.
- If rig/deformation is FAIL, animation visual result is automatically SKIP/blocked; texture can still be inspected independently.
- Client files are never modified by this gate.

Recommended retest:

```bash
python3 pt_asset.py validate animation Monbagon-C.smd --motion ATTACK --load-actions all
```

The animation gate uses pristine geometry. It does not apply the geometry witness used by `validate visual`.

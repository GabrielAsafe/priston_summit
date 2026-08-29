# Local test results — V0.2.1

- Python compile: PASS.
- Motion payload generation after corrected quaternion semantics:
  - Monbagon: 4 Actions PASS
  - Monshadow: 2 Actions PASS
  - Vampiric: 5 Actions PASS
  - SN-002: 2 Actions PASS
  - MN-005: 3 Actions PASS
  - MN-009: 3 Actions PASS
- Monbagon ATTACK has no active scale track. Relative bind→pose transforms after the correction:
  - determinant range: 0.999458247 .. 1.009551846
  - max singular-value deviation from rigid transform: 0.006710723
  - PASS for numerical rigidity sanity check.
- Blender GUI/runtime visual gate: PENDING on user's Blender 5.1 environment.
- Client modification: False.

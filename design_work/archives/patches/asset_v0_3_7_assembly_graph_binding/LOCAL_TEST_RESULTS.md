# V0.3.7 local test results

- `python3 -m compileall pt_asset_tool`: PASS.
- Assembly helper, unique explicit component: PASS.
  - target INX -> exact shared SMD -> second INX -> root SMB.
  - unrelated SMB with a valid different skeleton signature was not included.
  - result: `ASSEMBLY_COMPONENT_SKELETON_UNIQUE`.
- Assembly helper, two skeleton signatures in the same explicit component: PASS.
  - result: `AMBIGUOUS_ASSEMBLY_COMPONENT_SKELETON`.
- LOD slot overlap diagnostic: PASS.
  - unrelated INX sharing slot `Body` appeared only under
    `DIAGNOSTIC_ONLY_NOT_BINDING` and did not affect the decision.
- V6 -> V7 resumable migration: PASS.
  - prior tested SMB rows reused without invoking the structural gate again.
- Full CLI discovery + registry reload: PASS.
  - cache status `ASSEMBLY_COMPONENT_SKELETON_UNIQUE` reloaded as
    `ROOT_SKELETON_BOUND` / `ASSEMBLY_GRAPH_UNIQUE`.
  - `rigid_binding_ready=true`.
  - `motions=[]`.
  - `animation_binding_ready=false`.
- Partial inherited-motion regression: PASS.
  - a prior per-row `INX_SHARED_ROOT` candidate is cleared when a skeleton-only
    discovery cache wins; no accidental animation promotion remains.

# V0.3.9 local test results

## Compile

- `python3 -m compileall -q pt_asset_tool`: PASS

## Real client subset supplied in PT_ASSET_DEBUG_V037

Target body:

- `char/tmABCD/CmfbB02.smd`
- root: `M2Bip.inx / m2.smb`
- LOD used by body: `high`

Head resolver:

- 132 `.inf`-backed head/face assets resolve to the same verified M2 root.
- deterministic auto representative after H/M/L prioritization:
  `char/tmABCD/MfhB01.smd`
- descriptor: `char/tmABCD/Mfh-B01.inf`
- LODs: `high, default, low`
- SMD object_count: 3
- binding: `ROOT_SKELETON_BOUND / INX_LINK_FILE`
- animation_binding_ready: true

Structural gate, real binary files:

`MfhB01.smd + m2.smb`:

- PASS
- object_count = 3
- bind_vertices = 335
- bind_bones_used = 4
- skeleton_bones = 67
- missing_bind_bones = []
- client_modified = false

Additional head variant `Mfh-B01d.smd + m2.smb`:

- PASS
- object_count = 3
- bind_vertices = 1609
- bind_bones_used = 4
- used bones include `Bip01 Head` and `Bip-hair01/02/03`
- missing_bind_bones = []

## Root-family discovery sanity

Verified `.inf`-backed head candidate populations were found for M1–M7.
No filename prefix is used by the resolver; the deciding evidence is `.inf`
physical presence plus equality of verified root motion INX/SMB.

## Assembly orchestration test

Because Blender/Flatpak is unavailable in this environment, a fake Blender
runner was used only to test command orchestration and output handling.

With body `CmfbB02.smd`, auto head `MfhB01.smd`, fake Blender and a synthetic
bake payload:

- body import invoked separately: PASS
- head import invoked separately: PASS
- body bake invoked separately: PASS
- head bake invoked separately: PASS
- assembly stage invoked: PASS
- output summary:
  - `assembly_mode = BODY_PLUS_INDEPENDENT_HEAD`
  - `head_lod = high`
  - `assembly_visual = PASS` (non-interactive synthetic result)
  - `client_modified = false`

This does **not** claim a real Blender visual PASS. Alignment/deformation of the
combined real blend remains a human gate on the user's Blender installation.

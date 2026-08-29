# V0.6.2 local test results

## Physical missing-reference bundle

`PT_STAGE072_MISSING_TEXTURE_REPS_V061.zip` contains the 16 Stage0.72 files with
literal missing references. Re-running the V0.6.2 classifier over those physical
SMDs with the V0.6.1 exact-missing set reproduced:

- exact missing unique: 45
- runtime trailing-space aliases: 8
- serialized invalid unique: 5
  - directory-only: 2
  - MSVC debug-fill `0xCC`: 3
- client-file-missing unique: 32
- unclassified: 0
- classification complete candidate: true

The eight trailing-space names are `field/Sod/f-flame_0.tga ` through
`field/Sod/f-flame_7.tga `. The same physical corpus contains face-used
no-trailing-space references for those animation frames, so V0.6.2 may test the
narrow alias physically without basename guessing.

## Overlay alias test

Synthetic exact serialized name:

`field/Sod/f-flame_0.bmp `

Physical alias:

`field/Sod/f-flame_0.bmp`

Result:

- `LINK_READY_RUNTIME_ALIAS`
- exact serialized overlay destination exists including trailing space
- `hard_preparation_failures=[]`
- `material_preview_complete=true`

## Compile

`python3 -m compileall` / `py_compile` passes for the modified modules.

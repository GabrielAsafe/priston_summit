# V0.5.3 Local Test Results

## 1. Object-local final manifest

The uploaded V0.5.2 remaining-representatives manifest reports:

- `candidate_files_expected = 432`
- `physical_files_audited = 432`
- `full_parser_pass_files = 432`
- `FULL_PARSE_FAILURE = 0`
- `segmented_tmframe_files = 0`
- `object_local_complete_candidate = true`

Conclusion: object-local animation is CLOSED for the canonical corpus.

## 2. Canonical inventory Multilayer TexLink discovery

The canonical Model0.62 partition contains 3,148 SMD files. Using the inventory invariant that every face has at least one TexLink, files with `total_texlinks > total_faces` were counted as physical multilayer candidates.

Result:

- candidates: 121
- STATIC: 80
- RIGID: 41
- MAP_STAGE: 87
- MOB_MONSTER: 30
- CHARACTER: 2
- UNKNOWN: 2
- exactly 2.0 TexLinks/face: 101 files
- maximum aggregate ratio: 2.0

No aggregate evidence of a 3+ stage chain was found.

## 3. Real physical pointer-chain parse

Representative:

`Field/Fall_Game/chandelier-ani01.smd`

Result:

```text
object_count=5
material_count=6
total_faces=758
total_texlinks=1516
multilayer_faces=758
chain_depth_histogram={2: 758}
max_layers=2
uv_different_multilayer_faces=758
invalid_chain_count=0
cycle_count=0
htexture_stride_histogram={4: 758}
```

All 758 faces have a valid two-link chain. The second UV stage is genuinely different on every face. The serialized texture-handle fields are adjacent (+4), consistent with consecutive texture slots in the material's texture array.

## 4. Bundle test

A synthetic one-record client fixture containing the real chandelier SMD and two fake referenced texture files was audited.

Result:

```text
status=PASS
candidate_files_expected=1
physical_files_audited=1
valid_chain_files=1
max_layer_depth_counts={2: 1}
full_face_multilayer_files=1
partial_face_multilayer_files=0
distinct_uv_stage_files=1
multilayer_two_stage_complete_candidate=true
```

The generated ZIP contained:

- the SMD
- both referenced texture files
- `PT_MULTILAYER_TEXLINK_REPS/manifest.json`

## 5. Negative checks wired into the scanner

The parser rejects / records:

- misaligned face TexLink pointer
- face TexLink outside the object's TexLink block
- misaligned `NextTex`
- `NextTex` outside the block
- cycles
- chains longer than the defensive hard cap
- physical face/TexLink counts that disagree with inventory

## 6. Compile

`python3 -m compileall pt_asset_tool` => PASS.

## 7. Source-code stage semantics

The supplied client SourceCode confirms that the object builder iterates `smMatPattern[mNum].TextureCounter` and calls `obj->AddTexLink()` once per texture stage. If `bLightMap` is enabled, the normal texture-stage loop excludes the last slot and then appends the lightmap as a separate final TexLink using `tLightMapPoint/tLightMapFace`.

This independently proves that chained TexLinks represent ordered material texture stages with potentially different UV sets.

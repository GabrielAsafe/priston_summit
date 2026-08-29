# Summit Tale Asset Tool V0.5.3 — Multilayer TexLink Physical Corpus Audit

V0.5.3 is based on V0.5.2. It does not change the now-closed object-local animation semantics. Its purpose is to start the next 1.0 roadmap block: chained / multilayer `smTEXLINK` support.

## Object-local closure carried forward

The V0.5.2 physical audit completed with:

- 432 / 432 object-local candidates physically audited
- 432 / 432 full parser PASS
- zero `FULL_PARSE_FAILURE`
- zero segmented `TmFrameCnt`
- `object_local_complete_candidate=true`

Object-local animation can therefore be treated as CLOSED for the canonical client corpus.

## Why Multilayer TexLink needs a physical audit

A face stores a pointer to a first `smTEXLINK`. Each link stores one UV triplet and an optional `NextTex` pointer. The client code appends additional links by following `NextTex`, so the format is a true per-face texture-stage chain rather than duplicated bookkeeping.

The canonical inventory already proves 121 Model 0.62 SMD files have more TexLinks than faces:

- 80 STATIC
- 41 RIGID
- 87 MAP_STAGE
- 30 MOB_MONSTER
- 2 CHARACTER
- 2 UNKNOWN

101 of those files have exactly `total_texlinks == 2 * total_faces`. The aggregate inventory has no file with a ratio above 2.0, but only a physical pointer-chain audit can prove whether every chain is actually two stages and structurally valid.

## New commands

### Full physical corpus audit

```bash
python3 pt_asset.py audit texlink-corpus \
  --output pt_build/multilayer_texlink_corpus_audit.json \
  --bundle PT_MULTILAYER_TEXLINK_REPS.zip
```

The scanner only opens canonical Model0.62 candidates where `total_texlinks > total_faces` and checks:

- `Face.lpTexLink` pointer validity
- every `NextTex` pointer
- chain depth per face
- cycles / dangling or misaligned pointers
- full-face vs partial-face multilayer coverage
- whether later stages use distinct UV coordinates
- serialized `hTexture` pointer stride patterns
- physical counts vs inventory counts
- STATIC / RIGID and category distribution

The generated ZIP includes representative SMDs and all physically resolvable texture references already known by the canonical inventory.

### Inspect one SMD

```bash
python3 pt_asset.py asset texlinks Field/Fall_Game/chandelier-ani01.smd
```

## Current physical representative

`Field/Fall_Game/chandelier-ani01.smd` was already available from the object-local investigation and was inspected with the V0.5.3 parser:

- objects: 5
- materials: 6
- faces: 758
- TexLinks: 1516
- multilayer faces: 758 / 758
- chain depth histogram: `{2: 758}`
- max layers: 2
- later-stage UV differs from stage 0 on 758 / 758 faces
- invalid chains: 0
- cycles: 0
- serialized `hTexture` stride stage0→stage1: +4 bytes on 758 / 758 faces

The source code's `smOBJ3D::AddTexLink()` links stages via `NextTex`; therefore this is real two-stage per-face data, not a duplicate face UV array.

The ASE/SMD build path also confirms stage semantics: it loops over `smMaterial.TextureCounter` and calls `AddTexLink()` once per texture stage. When a lightmap is present, the lightmap is appended as the final TexLink using the separate lightmap UV set. This makes a two-link face directly meaningful as two material texture stages, often including a base/lightmap pair.

## What V0.5.3 intentionally does NOT do yet

V0.5.3 is discovery/audit only. It does not yet remove the existing `MULTILAYER_TEXLINK` authoring blocker and does not claim Blender material parity.

Before changing the importer/writer, the full physical audit should establish whether the 121 canonical assets are all two-stage chains and provide representative STATIC, RIGID, full-coverage, and partial-coverage assets with their textures.

## Safety

- source SMD files are read-only
- client files are never modified
- generated representative ZIP is a copy only
- no historical certification rows are removed
- unsupported chains remain fail-closed

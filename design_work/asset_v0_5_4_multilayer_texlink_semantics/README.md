# Summit Tale Asset Tool V0.5.4 — Multilayer TexLink Semantics + Blender Preview V1

V0.5.4 is based on V0.5.3. Object-local animation remains CLOSED. This release advances the Multilayer TexLink block from physical topology discovery to exact stage/material semantics and a two-stage Blender inspection preview.

## Corpus facts carried from V0.5.3

The user's physical audit proved:

- 121 / 121 canonical multilayer Model0.62 SMDs physically audited
- 121 valid pointer-chain files
- every file has maximum chain depth exactly 2
- 101 full-face multilayer files
- 20 partial-face multilayer files
- 71 files with a distinct second UV stage
- 0 three-plus-layer files
- 0 invalid-chain files
- 0 cycles
- 0 inventory/physical count mismatches
- stage0→stage1 serialized `hTexture` stride is +4 on 75,321 occurrences

Therefore the canonical corpus is a two-stage TexLink corpus.

## Exact hTexture semantics

The supplied client SourceCode calls `AddTexLink()` with the address of a material texture slot:

```cpp
obj->AddTexLink(face, (DWORD*)&smMaterial[material].smTexture[stage], ...);
```

The physical SMDs preserve those historical pointer values. Across all 12 V0.5.3 representatives, every TexLink satisfies exactly:

```text
hTexture = runtime_material_array_base
         + material_id * sizeof(smMATERIAL)
         + offsetof(smTexture)
         + stage_index * 4

sizeof(smMATERIAL) = 320
offsetof(smTexture) = 8
```

No filename heuristic is used. V0.5.4 verifies this mapping for every multilayer face and fails closed on a mismatch.

## Material stage/form semantics

The SourceCode maps explicit material script tokens to `TextureStageState` Direct3D operation codes and stores `TextureFormState` as the index of the `FS_*` form token.

Relevant forms observed in the supplied representatives include:

- `FS_NONE`
- `FS_SCROLL`, `FS_SCROLL3`, `FS_SCROLL4`, `FS_SCROLL5`, `FS_SCROLL6`
- `FS_REFLEX`

Relevant stage operations include:

- default/no explicit `BS_*` token
- `D3DTOP_ADD`

The ASE loader also appends lightmap UVs as a separate TexLink stage, independently proving that stage 1 can be a true second UV set/lightmap.

## New semantic profiles

The local representative set produced these profiles:

- `REFLEX_ADD`: base texture + `env.bmp`, stage 1 `D3DTOP_ADD`, `FS_REFLEX`
- `SECOND_UV_LIGHTMAP_MODULATE`: separate stage-1 UVs with a lighting-map texture
- `SAME_TEXTURE_DYNAMIC_FORM`: same texture in two stages with one or more scrolling form states
- `DUPLICATE_TEXTURE_DEFAULT_STAGE`: same image twice, no explicit stage/form state
- conservative fallback profiles remain available for corpus cases not represented locally

## New commands

### Audit semantic profiles across all 121 physical candidates

```bash
python3 pt_asset.py audit texlink-semantics \
  --output pt_build/multilayer_texlink_semantics_audit.json \
  --bundle PT_MULTILAYER_TEXLINK_SEMANTIC_REPS.zip
```

The audit verifies:

- exact `hTexture -> material.smTexture[stage]` mapping
- material stage/form state usage
- semantic profile distribution
- whether one material mixes one-stage and two-stage faces
- representative selection per semantic profile

### Inspect one SMD semantically

```bash
python3 pt_asset.py asset texlink-semantics \
  Field/Fall_Game/chandelier-ani01.smd
```

### Blender visual preview

```bash
python3 pt_asset.py validate texlinks \
  Field/Fall_Game/chandelier-ani01.smd
```

The Blender postprocessor creates and preserves two independent UV layers:

```text
PT_TEXLINK_STAGE_0
PT_TEXLINK_STAGE_1
```

It also adds a per-face integer attribute:

```text
pt_texlink_depth
```

and reconstructs two-stage preview materials from the exact material/stage mapping.

## Blender Preview V1 boundary

This release distinguishes source semantics from preview fidelity.

### Source semantics certified by the automatic gate

- two-stage chain ordering
- stage index
- material id
- exact `hTexture` slot relation
- UV0 and UV1 data
- texture names
- serialized `TextureStageState`
- serialized `TextureFormState`

### Preview behavior

- `FS_REFLEX`: represented with reflection-vector coordinates and the stage-1 environment image
- `D3DTOP_ADD`: additive combination
- explicit `D3DTOP_MODULATE`: multiply combination
- default/no explicit BS operation: conservative multiply preview, not runtime-certified
- `FS_SCROLL*`: frame-0/source-UV preview only; scrolling speed/timing is not certified here

The result JSON records exact vs approximate preview material counts. Approximate preview does not weaken the source semantic gate.

## Fail-closed rules

V0.5.4 refuses:

- chain depth other than the canonical two-stage maximum
- invalid exact hTexture/material/stage mapping
- a material that mixes one-stage and two-stage depth across its faces (until a per-face material strategy is explicitly implemented)
- unsupported Model0.62 family paths in the visual gate

## Safety

- source SMDs are read-only
- client textures are read through the generated overlay
- the client is never modified
- historical certification rows remain append-only
- dynamic UV timing and runtime render parity are not claimed
- Multilayer TexLink authoring/write-back is not enabled yet

## Protected BMP preview overlay

Some `REFLEX_ADD` materials reference proprietary Prefix14 BMP files such as monster `env.bmp`. V0.5.4 detects a non-`BM` BMP and uses the already-proven `pt_character_texture_v1_4.py` decoder to create a standard-BMP equivalent **only inside the temporary Blender texture overlay**. Bytes after the 14-byte proprietary file header are preserved exactly; the client texture is never rewritten.

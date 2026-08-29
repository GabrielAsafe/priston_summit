# Summit Tale Asset Tool V0.5.7 — Strict Multilayer TexLink Roundtrip

V0.5.7 continues directly from the V0.5.6 semantic/UV-transport closure. It
adds the first strict write-back gate for two-stage Model0.62 TexLinks.

This gate is **automatic**. The user is not asked to judge a UV map or decide
whether a technical texture-stage edit is correct.

## Source/runtime boundary used by the writer

The supplied Summit Tale SourceCode defines `smTEXLINK` as:

```cpp
struct smTEXLINK
{
    float u[3], v[3];
    DWORD *hTexture;
    smTEXLINK *NextTex;
};
```

The Model0.62 loader/writer serializes the raw `smTEXLINK` array and relocates
`NextTex` after loading. V0.5.7 therefore does **not** rebuild TexLink arrays.
It copies the original SMD and patches only the authorized serialized UV float
field in place.

This means the gate can prove byte-for-byte preservation of:

- face -> first TexLink pointers;
- TexLink chain topology;
- `NextTex`;
- `hTexture`;
- material region and texture-stage metadata;
- geometry;
- object-local animation payloads;
- Physique payloads;
- every non-witness byte in the SMD.

## Automatic witness

`validate texlink-roundtrip` performs the following pipeline:

1. parse and require the V0.5.6 two-stage source semantics to PASS;
2. import the source through the normal STATIC or RIGID Blender path;
3. materialize `PT_TEXLINK_STAGE_0` and `PT_TEXLINK_STAGE_1`;
4. automatically choose one real two-stage face;
5. apply one intentional stage-1 U-coordinate witness edit in Blender;
6. scan the edited Blend and fail closed if any unexpected UV coordinate was
   edited;
7. copy the source SMD and patch only that authorized `float` field;
8. reparse source and output and compare every face/chain structurally;
9. require `hTexture` and `NextTex` to remain byte-identical;
10. require all bytes outside the witness float to remain byte-identical;
11. reimport the rebuilt SMD and rerun the V0.5.6 automatic per-face/per-corner
    UV assignment gate.

No client file is modified. The rebuilt SMD remains under `pt_build` as a test
artifact.

## Install

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_5_7_texlink_roundtrip_strict.zip
./asset_v0_5_7_texlink_roundtrip_strict/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py doctor
```

Expected version:

```text
0.5.7
```

## Representative closure gates

Run these four automatic gates in this order:

```bash
# Full-face STATIC, distinct UV stage / lightmap
python3 pt_asset.py validate texlink-roundtrip \
  Field/Fall_Game/chandelier-ani01.smd

# Partial-face STATIC
python3 pt_asset.py validate texlink-roundtrip \
  Field/dungeon/Dun-6a-ani-015.smd

# Partial-face RIGID mob, reflection/dynamic forms
python3 pt_asset.py validate texlink-roundtrip \
  char/monster/Bguardian/Bguardian.smd

# Full-face RIGID character
python3 pt_asset.py validate texlink-roundtrip \
  char/tmABCD/CtmbC008.smd
```

A PASS prints three automatic checkpoints:

```text
ROUNDTRIP=PASS
structure_exact=True
hTexture_NextTex_byte_identical=True
non_witness_bytes_byte_identical=True
REIMPORT_UV=PASS
```

The command writes an append-only automatic certification named
`MULTILAYER_TEXLINK_ROUNDTRIP`.

## Fail-closed behavior

The writer refuses to continue if any of these occurs:

- source SHA changed after witness creation;
- source semantics are not PASS;
- source max depth is not exactly 2;
- the edited Blend contains an unexpected UV edit;
- witness object/face/stage/corner no longer maps to the source;
- the witness edit quantizes away;
- any non-witness output byte changes;
- `hTexture`/`NextTex`/chain structure changes;
- rebuilt semantics fail;
- rebuilt source -> Blender UV transport has any mismatch.

V0.5.7 still does not certify runtime FS_SCROLL speed/timing. That remains in
the later runtime-timing block.

# V0.5.7 Local Test Results

## Compile / CLI

```text
python3 -m compileall pt_asset_tool
PASS

python3 pt_asset.py --version
0.5.7

python3 pt_asset.py validate texlink-roundtrip --help
PASS
```

## Physical SourceCode proof

The user-supplied SourceCode contains:

```cpp
struct smTEXLINK
{
    float u[3], v[3];
    DWORD *hTexture;
    smTEXLINK *NextTex;
};
```

`smOBJ3D::AddTexLink` assigns the three U/V pairs, then `hTexture`, then
`NextTex`. The Model0.62 file path reads/writes the raw `smTEXLINK` array and
relocates `NextTex` after load. The serialized V0.62 corpus uses a 32-byte
TexLink stride, matching 24 UV bytes + 4-byte `hTexture` + 4-byte `NextTex`.

The strict writer therefore patches only an authorized 4-byte UV float and
never reserializes either pointer field.

## Strict writer regression on physical representatives

Using all 10 physical SMDs from the user-supplied
`PT_MULTILAYER_TEXLINK_SEMANTIC_REPS_V055.zip`, a stage-1 U witness was applied
to one real two-stage face in each file and then reparsed/compared:

```text
10/10 strict UV witness write PASS
10/10 rebuilt source_semantics PASS
10/10 TexLink structure exact
10/10 hTexture + NextTex preserved
10/10 non-witness bytes byte-identical
```

Files exercised:

```text
Field/Fall_Game/chandelier-ani01.smd
Field/Iron/i2-bip04_ani.smd
Field/Iron/i2-bip05_ani.smd
Field/dungeon/Dun-6a-ani-015.smd
Field/dungeon/dun-4-ani-33.smd
char/monster/Bguardian/Bguardian.smd
char/monster/Morgon/Morgon.smd
char/monster/ironfist/ironfist.smd
char/tmABCD/CtmbC008.smd
char/tmABCD/CtmbC010.smd
```

This covers all five semantic profiles present in the V0.5.6 corpus evidence:

```text
DUPLICATE_TEXTURE_DEFAULT_STAGE
GENERIC_TWO_STAGE
REFLEX_ADD
SAME_TEXTURE_DYNAMIC_FORM
SECOND_UV_LIGHTMAP_MODULATE
```

and includes full-face/partial-face plus STATIC/RIGID physical SMDs.

## Negative gates

A valid witness roundtrip was deliberately corrupted by changing a `NextTex`
pointer byte outside the witness field. The comparator refused it with:

```text
OUTPUT_SEMANTICS
TEXLINK_STRUCTURE
NON_WITNESS_BYTES_CHANGED
PASS (failure correctly detected)
```

A source SHA mismatch was also injected. The strict writer refused to write:

```text
source sha256 drift
PASS (failure correctly detected)
```

## Blender boundary

Blender/Flatpak is not available in this execution environment. Therefore the
new end-to-end `validate texlink-roundtrip` pipeline — automatic Blender witness,
strict edited-Blend scan, rebuilt import and source->Blender UV gate — must run
on the user's machine. The pure binary writer/comparator was tested against the
physical SMD representatives above.

No client file was modified.

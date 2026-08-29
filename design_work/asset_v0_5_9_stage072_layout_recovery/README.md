# Summit Tale Asset Tool V0.5.9 — Stage0.72 Physical Layout Recovery

Base: V0.5.8.

V0.5.8 correctly found all 63 physical `SMD Stage data Ver 0.72` files, but its parser rejected all 63 before geometry because two serialization invariants were wrong in the audit implementation. V0.5.9 fixes those parser assumptions using both the shipped corpus and the supplied SourceCode.

## Fix 1 — exact serialized `smSTAGE3D` size

The 32-bit `smSTAGE3D` serialized by `WriteFile(hFile, &Head, sizeof(smSTAGE3D), ...)` is **262260 bytes**, not 262272.

The size follows directly from the 32-bit field layout:

- `Head`: 4 bytes
- `StageArea[256][256]`: 65536 x 4-byte pointers
- remaining pointers / counters / lighting fields
- `POINT3D VectLight`: 12 bytes
- `RECT StageMapRect`: 16 bytes

The wrong +12 offset in V0.5.8 made the material parser start inside the `smMATERIAL_GROUP`, producing the apparent `material count mismatch` on every file.

## Fix 2 — `wAreaSize` is a capacity, not an exact serialized slot count

The loader allocates `DWORD[wAreaSize]`, advances `wbCnt` by each cell's `slen`, and only requires the consumed lists to fit the capacity. The physical corpus proves that equality is not an invariant.

V0.5.9 therefore requires:

```text
sum(slen) <= wAreaSize
```

rather than `sum(slen) == wAreaSize`.

## Physical StageArea families proved by the 63 uploaded files

Local audit of the exact `PT_STAGE072_REPS_V058.zip` supplied from the client:

```text
63/63 full parse PASS

Area encoding:
  DWORD32                  59 files
  WORD16                    1 file
  DWORD32_TRAILING_OPAQUE   3 files
```

The single physical WORD16 file is:

```text
Field/Lost/lost.smd
```

The three DWORD32 files with a runtime-ignored trailing suffix are:

```text
cSelect/select.smd                         124344 trailing bytes
game/maps/chrselect/scene.smd             441728 trailing bytes
game/maps/login/login.smd                   10996 trailing bytes
```

The Stage loader consumes the referenced area cells and closes the file without an EOF check. V0.5.9 classifies these suffixes explicitly as opaque and requires future writers to preserve them byte-identically rather than interpret or discard them.

## Stage0.72 TexLink topology discovered

The physical Stage0.72 corpus is not limited to two TexLink stages:

```text
max depth 1: 16 files
max depth 2: 44 files
max depth 3:  3 files
```

Depth-3 representatives:

```text
Field/Mine/mine-1.smd
Field/Slab/Slab.smd
Field/endless/dun-7.smd
```

All three have valid `hTexture -> smMaterial[].smTexture[stage]` mapping through stage 2. Their depth-3 faces use stage-2 UVs distinct from stages 0/1, so Stage0.72 import/edit support must preserve **up to three UV stages**. Do not reuse the Model0.62 two-stage ceiling here.

## Inert zero-texture materials

The renderer returns before drawing a material when both:

```text
TextureCounter == 0
AnimTexCounter == 0
```

The physical corpus contains 5957 depth-1 TexLinks on such materials across 11 files. Two exact handle forms occur:

```text
hTexture == NULL                         4544 faces
hTexture == &smMaterial[mid].smTexture[0] 1413 faces
```

V0.5.9 accepts only these two forms for a depth-1 zero-texture material. Any other handle or deeper chain remains FAIL-closed.

## Command

```bash
python3 pt_asset.py audit stage072-corpus \
  --output pt_build/stage072_corpus_audit_v059.json \
  --bundle PT_STAGE072_REPS_V059.zip
```

Expected result on the current canonical client:

```text
stage072_corpus_audit=PASS audited=63/63 full_parse=63 \
  depths={'1': 16, '2': 44, '3': 3} \
  area={'DWORD32': 59, 'DWORD32_TRAILING_OPAQUE': 3, 'WORD16': 1} \
  depth3=3 opaque=3 complete_candidate=true
```

This patch remains an **audit/parser patch**. It does not yet write Stage0.72 files and does not modify the client.

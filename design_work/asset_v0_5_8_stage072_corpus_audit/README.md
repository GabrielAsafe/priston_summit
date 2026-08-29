# Summit Tale Asset Tool V0.5.8 — Stage0.72 Physical Corpus Audit

Base: V0.5.7. This patch starts the next roadmap block after Multilayer TexLink roundtrip.

## What is closed before this patch

V0.5.7 representative strict roundtrip gate passed for:
- STATIC full-face distinct UV/lightmap (`chandelier-ani01.smd`)
- STATIC partial-face (`Dun-6a-ani-015.smd`)
- RIGID partial-face/dynamic form (`Bguardian.smd`)
- RIGID character (`CtmbC008.smd`)

Each gate proves an intentional stage-1 UV edit, strict non-witness byte identity, `hTexture`/`NextTex` identity and reimport UV equality.

## New command

```bash
python3 pt_asset.py audit stage072-corpus \
  --output pt_build/stage072_corpus_audit_v058.json \
  --bundle PT_STAGE072_REPS_V058.zip
```

## Source-backed Stage0.72 parser

The parser follows the 32-bit runtime serialization in `smStage3d.cpp`, `smStage3d.h`, `smType.h`, `smTexture.cpp`:

- exact header: `SMD Stage data Ver 0.72`
- serialized raw `smSTAGE3D` header size: 262272 bytes
- material group and `smMATERIAL` payload
- `smSTAGE_VERTEX` (28 bytes)
- `smSTAGE_FACE` (28 bytes; `Vertex[3]` is material id)
- `smTEXLINK` (32 bytes)
- `smLIGHT3D` (28 bytes)
- face `lpTexLink` / `NextTex` pointer rebasing from the serialized TexLink base
- exact `hTexture -> smMaterial[material].smTexture[stage]` mapping
- zero-texture placeholder TexLinks created by the no-TVertex SourceCode path
- StageArea occupancy and serialized area payload

## Important SourceCode discrepancy being audited physically

`smSTAGE3D::SaveFile()` writes each StageArea cell as:

- `slen` as 32-bit int
- `slen` entries as 16-bit `WORD`

The legacy client `LoadFile()` source reads the entries as 32-bit `DWORD`.

V0.5.8 does not guess which behavior the shipped corpus actually needs. It tries both encodings against every physical Stage0.72 file and reports which representation closes the file exactly and preserves valid face indices / `wAreaSize`.

This is intentionally an audit patch only. No Stage0.72 writer, Blender importer or client modification is introduced yet.

## Expected current corpus

The canonical inventory previously identified 63 Stage0.72 files. A strong result is:

```text
stage072_corpus_audit=PASS audited=63/63 full_parse=63 ... complete_candidate=true
```

The report and representative bundle then become the proof base for the Stage0.72 Blender/import/edit/build implementation.

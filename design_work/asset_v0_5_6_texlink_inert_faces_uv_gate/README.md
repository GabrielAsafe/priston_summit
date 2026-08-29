# Summit Tale Asset Tool V0.5.6 — Inert TexLink Faces + Automatic UV Transport Gate

V0.5.6 is a narrow continuation of V0.5.5. It closes the only remaining
`stage_index` semantic anomaly observed in the 121-file Multilayer TexLink
corpus and makes the Blender UV transport check automatic.

It does **not** implement TexLink roundtrip/write-back yet and does not modify
client SMD/INX/SMB files.

## 1. `dun-4-ani-33.smd`: source-proven inert TexLink faces

V0.5.5 isolated four `stage_index_errors`:

- `ani-31` faces 10/11
- `ani-32` faces 10/11
- material 0
- `TextureCounter=0`
- chain depth 1

Physical inspection proves the single TexLink on each face points to
`smMaterial[0].smTexture[0]`. The material has:

- `TextureCounter=0`
- `AnimTexCounter=0`
- no texture names

The supplied SourceCode supplies the runtime boundary: `smRENDER3D::RenderD3D`
returns immediately when both counters are zero. Therefore this single TexLink
is not a renderable material stage; it is inert runtime data.

V0.5.6 accepts **only** the narrow shape:

```text
TextureCounter == 0
AnimTexCounter == 0
TexLink chain depth == 1
```

as `inert_zero_texture_texlink_faces`. Any other TexLink depth beyond the
material texture count still fails closed.

The four physical faces are retained in reports; they are not erased.

## 2. Automatic source -> Blender UV transport verification

The previous human prompt asked the user to decide whether UVs looked
"embaralhados". That is a poor evidence surface:

- REFLEX_ADD uses a Reflection coordinate for stage 1 in the preview;
- lightmaps intentionally use a distinct second UV set;
- multiple objects/UV islands can look visually dense or overlapping in the UV
  Editor even when every source corner is mapped correctly.

V0.5.6 now reads both Blender UV layers back after assignment and compares every
face/corner against the serialized SMD TexLink values (including Blender's V
flip). It also checks the per-face TexLink depth attribute.

The Blender postprocess fails automatically unless:

```text
uv_assignment_check = PASS
uv_assignment_mismatch_count = 0
uv_assignment_depth_mismatch_count = 0
```

The human gate now asks only about the visible effect on the rendered model:

```text
No MODELO renderizado, o efeito da segunda camada
(reflexo/lightmap/overlay) aparece nas superfícies corretas,
sem artefatos visuais evidentes?
```

FS_SCROLL* remains frame-0 only; runtime texture-motion speed/behavior is not
certified by this gate.

## Install

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_5_6_texlink_inert_faces_uv_gate.zip
./asset_v0_5_6_texlink_inert_faces_uv_gate/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py doctor
```

The installer backs up the current backend and the historical V3 Blender
importer, preserving the V0.5.5 LOD/runtime-name fix.

## Corpus gate

```bash
python3 pt_asset.py audit texlink-semantics \
  --output pt_build/multilayer_texlink_semantics_audit_v056.json \
  --bundle PT_MULTILAYER_TEXLINK_SEMANTIC_REPS_V056.zip
```

Expected from the supplied V0.5.5 full-corpus result plus the physical anomaly
proof:

```text
htexture_mapping_fail_files = 0
stage_index_fail_files = 0
source_semantics_fail_files = 0
inert_zero_texture_texlink_faces = 4
two_stage_semantics_complete_candidate = true
```

The real 121-file audit on the user's client is authoritative.

## Visual gates to rerun

```bash
python3 pt_asset.py validate texlinks \
  char/monster/Bguardian/Bguardian.smd

python3 pt_asset.py validate texlinks \
  char/monster/Morgon/Morgon.smd

python3 pt_asset.py validate texlinks \
  Field/Fall_Game/chandelier-ani01.smd
```

Judge the rendered model/effect, not whether the UV Editor looks crowded.
Previous FAIL/SKIP certifications remain append-only and are not deleted.

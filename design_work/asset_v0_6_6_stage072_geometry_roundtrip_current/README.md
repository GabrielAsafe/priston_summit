# Summit Tale Asset Tool V0.6.6 — Stage0.72 current-family geometry roundtrip

V0.6.4 physically proved `StageArea`, `wAreaSize`, pointer layout and `StageMapRect` reconstruction on all 63 Stage0.72 maps. V0.6.5 classified `smSTAGE_FACE::VectNormal` generation: 46 maps are byte-exact with the literal current `SetNormal` evaluation, while 17 maps carry legacy floating/provenance variants.

The full SourceCode search then located the exact implementation in both `SrcGame/src/smLib3d/smMap3d.cpp` and `SrcServer/src/smLib3d/smMap3d.cpp`. They are identical:

```cpp
float i, j, k, d;
// integer cross -> float i/j/k
d = (float)sqrt((double)((i * i) + (j * j) + (k * k)));
i = (float)(i / d * 32767);
j = (float)(j / d * 32767);
k = (float)(k / d * 32767);
normal->x = (int)i;
normal->y = (int)j;
normal->z = (int)k;
```

`FLOATNS` is 8, and `SetVertexShade()` feeds `Vertex.x/y/z >> 8` into this function.

## Safety boundary

V0.6.6 enables the **first real Stage0.72 vertex writer**, but only for the 46-map current exact normal family.

It remains fail-closed for:

- the 7 reciprocal legacy-normal maps;
- the 10 one-LSB unresolved legacy/provenance maps;
- the one WORD16 StageArea map when it is also legacy-normal;
- Stage0.72 files with an opaque trailing suffix when their normal family is legacy.

The writer does not alter the original client file. It creates a rebuilt diagnostic artifact only.

## What the strict geometry witness changes

The automatic witness moves exactly one Blender vertex in X by a default of 64 world units (an exact fixed-point delta of 16384). The planner selects a low-degree vertex for which the move:

1. changes the runtime StageArea membership,
2. does not create a degenerate incident face,
3. stays under the original `AreaList[4096]` face-cell limit.

The writer then:

- writes exactly that one fixed-point vertex coordinate;
- recomputes `VectNormal[0..2]` only for incident faces using the physically-proven current `SetNormal` model;
- preserves every `VectNormal[3]` byte exactly;
- rebuilds all StageArea cells, pointer flags/addresses and `wAreaSize`;
- rebuilds `StageMapRect` from the edited vertex bounds;
- preserves materials, TexLinks/UVs, `hTexture`, `NextTex`, lights, colors, topology and all unrelated header bytes.

The output is reparsed, the full StageArea byte-exact reconstruction gate is rerun against the edited geometry, the normal-family audit must remain current-exact, and the rebuilt SMD is reimported into a fresh Blender process for persisted transport verification.

## Install

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_6_6_stage072_geometry_roundtrip_current.zip
./asset_v0_6_6_stage072_geometry_roundtrip_current/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py --version
python3 pt_asset.py doctor
```

Expected version: `0.6.6`.

## Automatic geometry gates

Recommended current-family representatives:

```bash
# DWORD32 + 97 lights
python3 pt_asset.py validate stage072-geometry-roundtrip \
  Field/dungeon/dun-1.smd

# depth-3 TexLink map; geometry writer must preserve all 3 UV stages
python3 pt_asset.py validate stage072-geometry-roundtrip \
  Field/endless/dun-7.smd

# current-normal family with a physically missing texture reference;
# structural geometry transport must remain independent from material completeness
python3 pt_asset.py validate stage072-geometry-roundtrip \
  Field/Quest/quest_IV.smd
```

The command should report:

```text
STAGE072_GEOMETRY_ROUNDTRIP ...
WITNESS vertex=... delta_world=64.0 incident_faces=...
ROUNDTRIP=PASS normal_family=CURRENT_EXACT stagearea_changed=True vectnormal3_exact=True texlinks_exact=True lights_exact=True
STAGEAREA_REBUILD=PASS ...
REIMPORT_TRANSPORT=PASS ...
LEGACY_GEOMETRY_WRITE=DEFERRED maps=17 ...
```

Trying the command on a legacy-normal Stage0.72 file is expected to fail closed before Blender writing.

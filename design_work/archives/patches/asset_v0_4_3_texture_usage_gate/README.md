# Summit Tale Asset Tool V0.4.3 — Face-Used Texture Readiness

## Why this patch exists

The three V0.4.2 static weapon/item roundtrip representatives passed structural and visual validation. `itWH118.smd`, however, reported the contradictory combination:

- `texture_material_visual = PASS`
- `static_roundtrip = PASS`
- `texture_prepare = FAIL`

The Model0.62 inventory records every serialized material texture reference. Real files may retain historical material slots that no face references. Treating every serialized texture as visually required makes `texture_prepare` stricter than the rendered geometry and conflicts with the already-established Auto Profile rule that distinguishes face-used materials from historical serialized slots.

## V0.4.3 change

`prepare_texture_overlay()` now derives the material IDs actually referenced by Model0.62 faces and decodes the serialized material table with the existing `pt_pikeman_smd_material_audit_v1.py` parser.

Texture readiness semantics are now:

- texture from a face-referenced material: REQUIRED;
- texture only from an unreferenced/historical material slot: HISTORICAL_UNUSED;
- required texture missing/decode failure: `texture_prepare = FAIL`;
- historical-unused texture missing/decode failure: warning only, does not fail readiness;
- if material/face-use classification cannot be proven: fail closed to the previous `ALL_SERIALIZED_FALLBACK` behavior.

The overlay still attempts to make all serialized textures available. This patch only corrects which failures are allowed to influence the readiness gate.

## New report fields

Visual and static-roundtrip summaries now expose:

- `texture_usage_semantics`
- `texture_required_count`
- `texture_historical_unused_count`
- `texture_prepare_warning_count`

The detailed `texture_overlay_report.json` also records `used_material_ids`, required vs historical status per entry, and separate required/historical failure counts.

## Safety

No source SMD, texture, INX, SMB or client file is modified. If face-use classification fails, V0.4.3 remains stricter rather than guessing.

## Install

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_4_3_texture_usage_gate.zip
./asset_v0_4_3_texture_usage_gate/install.sh \
"/home/gabriel/Área de Trabalho/summit/design_work"
```

Expected version: `0.4.3`.

## Focused verification

Only `itWH118` needs to be repeated to verify whether its previous preparation failure was historical/unreferenced:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate roundtrip \
  image/Sinimage/Items/DropItem/itWH118.smd
```

If the previous failure was from an unused historical slot, the new result will show `texture_prepare = PASS` plus a non-zero warning count. If a face-used texture actually fails preparation, it remains `FAIL`.

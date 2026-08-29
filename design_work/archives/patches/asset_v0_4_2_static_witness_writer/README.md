# Summit Tale Asset Tool V0.4.2 — Strict Static Witness Writer

## Why this patch exists

V0.4.1 reached the real Blender writer step for `itPR101.smd` and failed in `03_static_writer.log`. The CLI only surfaced the log path, so the exact Blender exception was not visible in the pasted terminal output.

The V0.4.1 certification path routed a position-only witness through the broader static/free-mesh writer, which unnecessarily regenerated normals/triangulation/UV-facing records. For a roundtrip certification whose intentional edit is only vertex XYZ, that is a wider mutation surface than necessary.

## V0.4.2 change

`validate roundtrip` now uses `pt_asset_tool/blender/write_static_witness.py`.

The strict writer:

1. loads the edited Blender mesh;
2. requires the same vertex count as the source object;
3. reads the witness manifest;
4. proves the Blender changed-vertex set is exactly the witness set;
5. copies the source SMD byte-for-byte;
6. replaces only the 12-byte XYZ fields of the witnessed vertices;
7. proves reverting those XYZ ranges recovers the source SMD exactly.

Therefore normals, face records, UV/TexLink records, materials, object headers and all non-target data remain byte-identical in this certification path.

The broader `pt_blender_to_smd_static_v1.py` / free-mesh writer is kept in the project. V0.4.2 does not claim that broader topology authoring is certified by this witness gate.

## Independent comparator hardening

`pt_asset_tool/static_roundtrip.py` now independently checks the final file at byte level. Any byte difference outside the allowed witness XYZ ranges raises `NON_XYZ_BYTES_CHANGED`.

This means the binary preservation invariant is checked independently of the writer report.

## Better failures

`run_logged()` now includes the last lines of failed subprocess output in the raised error, in addition to the full log path. A future Blender-only failure should therefore be actionable directly from terminal output.

## Install

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_4_2_static_witness_writer.zip
./asset_v0_4_2_static_witness_writer/install.sh \
"/home/gabriel/Área de Trabalho/summit/design_work"
```

Expected version: `0.4.2`.

## Retest

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate roundtrip \
  image/Sinimage/Items/DropItem/itPR101.smd
```

If `itPR101` passes, continue with `itWA104` and `itWH118` one at a time.

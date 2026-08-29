# V0.4.2 Local Test Results

## Compile

- `python3 -m compileall pt_asset_tool`: PASS

## Strict witness writer fixture

Using the valid V0.4.1 static Model0.62 fixture:

- source object: 3 vertices / 1 face / 1 TexLink;
- witness changes vertex 0 only;
- output is byte-different from source;
- writer reports `non_xyz_bytes_byte_identical=true`;
- writer reports `normals_preserved_byte_identical=true`;
- writer reports faces/UV/material/TexLink preserved byte-identical;
- `PT_STATIC_WITNESS_WRITER_V1`: PASS.

## Independent static comparator

The rebuilt fixture passes `compare_static_roundtrip()` with:

- expected changed vertices `[0]`;
- actual changed vertices `[0]`;
- `non_xyz_bytes_byte_identical=true`;
- `byte_identical=false`.

## Adversarial checks

1. Blender mesh changes a second unregistered vertex:
   - strict writer rejects with changed-set mismatch: PASS.
2. One byte in a vertex normal is modified after the valid rebuild:
   - independent comparator rejects with `NON_XYZ_BYTES_CHANGED`: PASS.

## Diagnostic behavior

`run_logged()` now includes a bounded tail of subprocess output in failures while retaining the full log path.

## Environment limitation

The assistant runtime has no Blender/Flatpak and the supplied V0.3.7 diagnostic bundle contains `char/tmABCD`, not the physical DropItem weapon SMDs. The real Blender execution for `itPR101`, `itWA104`, and `itWH118` therefore remains a client-side integration gate.

# V0.4.1 Local Test Results

## Compile

- `python3 -m compileall pt_asset_tool`: PASS

## Registry coverage using user's V0.3.7 diagnostic inventory

- `itPR101.smd`: `WEAPON_ITEM / SMD_MODEL_062_STATIC`, 1 object, 1 material, 5 vertices, 4 faces.
- `itWA104.smd`: `WEAPON_ITEM / SMD_MODEL_062_STATIC`, 1 object, 1 material, 86 vertices, 146 faces.
- `itWH118.smd`: `WEAPON_ITEM / SMD_MODEL_062_STATIC`, 1 object, 13 materials, 94 vertices, 150 faces.

## Static roundtrip comparator fixture

Synthetic valid Model0.62 static fixture:

- intentional witness changed vertex 0 only;
- source/output byte-identical: false;
- expected changed vertices: `[0]`;
- actual changed vertices: `[0]`;
- face/UV/material semantics exact;
- comparator: PASS.

Adversarial fixture with a second unregistered vertex change:

- rejected with `CHANGED_VERTEX_SET` + `UNTOUCHED_VERTEX_CHANGED`: PASS.

## Full backend orchestration fixture

Using a fake Blender executable to exercise the backend sequence:

1. static capability probe;
2. pristine import;
3. witness edit;
4. writer invocation;
5. strict roundtrip comparator;
6. rebuilt static capability probe;
7. rebuilt Blender reimport;
8. certifications and summary.

Result:

- `static_roundtrip_structural=PASS`
- `static_roundtrip=PASS`
- `changed_vertices=[0]`
- `byte_identical=false`
- `client_modified=false`

## Environment limitation

The assistant container has no Blender/Flatpak runtime and the diagnostic ZIP supplied by the user contains `char/tmABCD`, not the physical weapon SMDs. Therefore the real Blender execution for the three weapon representatives must run on the user's existing client. The V0.4.1 gate performs all automatic checks before asking for human visual approval.

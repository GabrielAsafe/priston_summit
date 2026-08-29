# G5 Manual Artist Gate V1.1

Fixes the V1 baseline loader bug that could raise:

`KeyError: 'TbmC03'`

Cause:
the canonical requested-name list was passed directly to
`bpy.data.libraries.load()` and then reused to construct the baseline mapping.

V1.1:
- keeps canonical mesh names in an immutable tuple;
- passes a fresh list to `data_to.objects`;
- snapshots the loaded object list only after the library-load context exits;
- requires exactly 9 loaded baseline objects;
- rejects any null/missing baseline object explicitly;
- constructs the mapping from the preserved requested string names.

No validator, skin, material, topology, C002, client, or non-target isolation
gate is weakened.

The gate still:
- requires pristine client;
- requires TbmC03 to differ from baseline;
- requires all 8 non-target objects unchanged;
- accepts arbitrary artist topology on TbmC03;
- stops on unresolved skin ambiguity;
- rebuilds vertices/faces/TexLinks/Physique;
- performs authoritative C002 reimport;
- audits baseline materials;
- never deploys automatically.

# G6 Manual All-LOD Gate V1.1

V1.1 fixes artist-path handling.

The V1 wrapper hardcoded:
`blender/pikeman_c01_m4/artist/C002_TbmC03_artist_master_g6_all_lods_v1.blend`

V1.1 adds `--artist`, allowing a preserved `.blend`, `.blend1`, or recovery copy
to be used directly without renaming or overwriting it.

Examples:

    python3 pt_pikeman_manual_all_lod_gate_g6_v1_1.py inspect \
      --artist blender/pikeman_c01_m4/artist/_recovery_g6/C002_TbmC03_artist_master_g6_all_lods_v1.blend1

    python3 pt_pikeman_manual_all_lod_gate_g6_v1_1.py build \
      --artist blender/pikeman_c01_m4/artist/_recovery_g6/C002_TbmC03_artist_master_g6_all_lods_v1.blend1

All strict G6 gates remain unchanged:
- pristine client;
- TbhC03, TbmC03, TblC03 all manually changed;
- material 0 only;
- SMD_UV only;
- 11 vertex groups per target;
- six C01 non-target meshes unchanged;
- no force-test;
- one multi-object direct SMD;
- six non-target SMD blocks byte-identical;
- independent C002 high/default/low reimports;
- material audit;
- no automatic deploy.

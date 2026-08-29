# G5 — Manual Artist Workflow

This gate does not create or reshape geometry.

Artist source:
`blender/pikeman_c01_m4/artist/C002_TbmC03_artist_master_g5_v1.blend`

Target:
`TbmC03` from real Pikeman `TmbC01.smd`, C002 DefaultModel.

Strict pre-export inspection:
- client must be pristine;
- target must actually differ from the baseline C002 preview;
- target must retain exactly one active `SMD_UV`;
- target must retain the 11 vertex-group schema;
- target may use only source material 0;
- all eight non-target mesh objects must match the baseline file exactly
  (mesh coordinates/topology, UVs, materials, vertex groups/weights, transform).

Build:
- runs production artist pipeline V2.1 without force-test;
- safe rigid normalization is allowed;
- unresolved ambiguous skin remains a hard failure;
- arbitrary target topology/count/order is allowed;
- all target vertices/faces/TexLinks/Physique are rebuilt;
- eight non-target SMD object blocks must remain byte-identical;
- performs authoritative independent reimport with C002.inx;
- preserves baseline SMD materials;
- creates a managed build manifest;
- never deploys to client automatically.

Runtime install remains an explicit separate step.

Implementation note: baseline comparison ignores temporary appended datablock names, so Blender `.001` renaming cannot create a false drift.

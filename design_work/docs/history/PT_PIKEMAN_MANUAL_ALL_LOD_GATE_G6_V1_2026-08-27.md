# G6 — Manual All-LOD Authoring

Targets in one artist master:
- TbhC03 = C002 High body
- TbmC03 = C002 Default body
- TblC03 = C002 Low body

The G6 artist master starts as an exact copy of the successful G5 artist master,
so the manual Default remodel is preserved. The artist must then manually edit
High and Low (and may further refine Default).

Strict artist inspection:
- all three C03 targets must differ from pristine C002 baseline;
- each target must keep one active SMD_UV;
- each target must keep 11 vertex groups;
- each target may use only material 0;
- the six C01 objects must remain exactly unchanged.

Build:
- uses production pt_character_multi_object_lod_pipeline_v2.py;
- explicitly targets only TbhC03, TbmC03, TblC03;
- no --force-test;
- one SMD is produced;
- writer must preserve 6 non-target SMD object blocks byte-identically;
- outer gate independently reimports with C002.inx at High, Default and Low;
- expected C002 visible pairs:
  high    TbhC03 + TahC01
  default TbmC03 + TamC01
  low     TblC03 + TalC01
- material region is audited;
- no automatic client deployment.

Runtime proof must confirm the three manually authored LOD bodies appear at
their corresponding distance bands without crashes or geometry corruption.

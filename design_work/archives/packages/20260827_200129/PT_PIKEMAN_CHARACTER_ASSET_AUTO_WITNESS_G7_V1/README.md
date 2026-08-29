# G7 Auto Witness

Purpose: complete the full-character-asset structural/runtime gate without
requiring additional manual Blender work.

This does NOT replace the already-proven manual-authoring milestones G5/G6.
It only provides deterministic changed geometry for the six C01 meshes needed
to exercise C001 together with the already-edited C03 meshes used by C002.

Changes:
- TblC01/TbmC01/TbhC01: XY scale 1.20 around local centroid
- TahC01/TamC01/TalC01: XY scale 1.12 around local centroid

Preserved exactly by the edit:
- vertex count/order
- face topology
- UV data/schema
- material assignment
- vertex-group schema
- vertex weights
- armature modifier

The witness writes a NEW .blend and never deploys to the client.

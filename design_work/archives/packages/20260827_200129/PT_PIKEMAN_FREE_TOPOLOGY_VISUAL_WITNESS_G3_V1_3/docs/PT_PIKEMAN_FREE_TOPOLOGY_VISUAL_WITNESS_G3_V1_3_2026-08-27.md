# G3 Visual Witness V1.3

Fixes V1.2 regression:

V1.2 replaced the mesh datablock and attempted to restore deform weights by
old vertex-group indices without first recreating the object's vertex-group
schema. Blender reported `vertex_groups size 0`.

V1.3:
- captures all 11 group names/order before mesh replacement;
- recreates the group schema after replacement;
- restores all old memberships using original indices;
- compares old memberships/weights by group name;
- requires zero meaningful weight drift;
- assigns the new witness vertex rigidly;
- keeps the +4.0 Y visual witness.

No client files are modified by this helper.

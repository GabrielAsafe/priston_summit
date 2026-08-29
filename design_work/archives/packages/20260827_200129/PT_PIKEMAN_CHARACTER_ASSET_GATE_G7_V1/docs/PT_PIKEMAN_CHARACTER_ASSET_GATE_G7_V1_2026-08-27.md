# G7 — Pikeman Character Asset Authoring

## Goal

Prove the complete `TmbC01.smd` as one editable character asset rather than a
single armor/LOD witness.

The same SMD contains nine meshes. C001 and C002 select different combinations:

| LOD | C001 | C002 |
|---|---|---|
| High | TbhC01 + TahC01 | TbhC03 + TahC01 |
| Default | TbmC01 + TamC01 | TbmC03 + TamC01 |
| Low | TblC01 + TalC01 | TblC03 + TalC01 |

The union is all nine SMD meshes.

## Artist master

The prepare script copies the successful G6 artist master, preserving the
already-proven manual C03 High/Default/Low edits. The artist then manually edits
the six C01 meshes:

- TbhC01, TbmC01, TblC01
- TahC01, TamC01, TalC01

The inspector requires all nine meshes to differ from the pristine baseline.

## Strict semantics

- object names preserved;
- exactly `SMD_UV`;
- vertex-group schema names/order preserved per object;
- ARMATURE modifier preserved;
- C03 body meshes use material 0 only;
- C01 body/arm meshes use material 1 only;
- material slot count preserved;
- arbitrary topology/position/UV/weight changes allowed;
- no force-test;
- unresolved rigid-skin ambiguity remains a hard stop.

## Build

All nine objects are targeted in one run of
`pt_character_multi_object_lod_pipeline_v2.py`.

Because every SMD mesh is rebuilt:
`non_target_object_blocks_byte_identical=0`.

The outer gate independently reimports the single generated SMD six times:

- C001 high/default/low
- C002 high/default/low

It requires 9 objects, 2 materials, 58 skeleton objects, 32 bind bones used,
dynamic target counts in every reimport, correct selected visible pairs, and
baseline material audit.

No client deployment is automatic.

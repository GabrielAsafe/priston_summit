# PT Asset Tool 0.2.5 — explicit pose-basis bridge

Focused animation bridge correction after the live diagnostic proved:

- SOURCE_DIRECT: visually correct ATTACK
- BLENDER_ARMATURE: explodes
- motion selection/evaluator/source rigid skinning are therefore not changed

The baker now converts the desired absolute source deformation to Blender
`PoseBone.matrix_basis` explicitly, parent-first. On Blender versions that expose
`Bone.convert_local_to_pose`, that API is used so parent/rest/inherit-scale rules
are respected. A compatibility fallback is retained for older versions.

No SMB/INX/client data is modified and validators are not weakened.

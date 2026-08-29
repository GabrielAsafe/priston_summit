# G4 Visual Gate V1.1

G4 V1 structurally passed but its visual effect was too subtle.

V1.1 preserves the same topology target:
- TbmC03: 74/122/122 -> 82/138/138
- +8 vertices
- +16 faces
- same material semantics
- same rigid-skin constraints

Visual witness change:
- choose an exposed connected 8-face safe patch (prefer Spine1)
- move every original source vertex participating in that patch radially
  outward by 2.50 Blender units
- add the 8 center vertices required by the topology refinement
- place each new center another 1.50 units outward beyond its moved corners

Unlike G4 V1, this changes existing boundary vertices, so neighboring faces and
the local silhouette deform as a continuous region. It should be visually
obvious when the camera is rotated around the C002 Default/medium character.

Do not infer anatomical front/back from an axis sign.
Runtime proof remains a separate explicit deploy/test step.

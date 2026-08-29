# G3 — Pikeman Free Topology Gate V1

Target:
- profile `pikeman_c01_m4`
- source `TmbC01.smd`
- skeleton `m4.smb`
- runtime selection `C002.inx`, DefaultModel
- object `TbmC03`

Controlled topology edit:
- source 74 vertices / 122 triangles
- choose one exposed triangle whose 3 vertices belong rigidly to the same bone
- add one center vertex
- displace it 0.80 Blender units outward
- replace 1 triangle with 3 triangles
- interpolate UV center
- assign new vertex rigidly to the same bone

Expected artist result:
- 75 vertices
- 124 faces

The gate then delegates serialization to the existing V2.1 artist pipeline /
free-mesh writer already used for the prior free-topology work. It refuses to
run if V2.1 does not expose an explicit profile selector.

Strict structural requirements:
- `output_counts=75/124/124`
- all vertices/faces/TexLinks/Physique rebuilt by existing writer
- 8 unrelated TmbC01 object blocks byte-identical
- independent V3 reimport
- 9 objects, 2 materials, 58 bones
- 956 bind vertices, 32 used bones
- TbmC03 = 75V/124F/124UV
- C002 default = TbmC03 + TamC01
- no client modification

Only after `g3_structural_gate_check=PASS` should the generated build manifest
be published/deployed with the existing workspace bridge + Restore V4.

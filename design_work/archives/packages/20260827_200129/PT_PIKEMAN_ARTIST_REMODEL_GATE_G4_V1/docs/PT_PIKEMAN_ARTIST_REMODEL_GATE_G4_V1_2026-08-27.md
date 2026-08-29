# G4 — Pikeman Artist Remodel Gate V1

Purpose: move beyond the one-vertex G3 witness into a connected, multi-face
topology edit that resembles normal remodeling.

Baseline:
- real Pikeman `TmbC01.smd`
- `m4.smb`
- `C002.inx`
- DefaultModel `TbmC03`
- 74 vertices / 122 triangles / 122 TexLinks

Controlled remodel:
1. Find connected source triangles whose three vertices are already rigid to
   the same bone.
2. Prefer a `Bip01 Spine1` component when it has at least 8 faces; otherwise
   use the largest safe same-bone component.
3. Select 8 connected faces.
4. Replace each selected triangle with three triangles around a new center
   vertex.
5. Interpolate one UV center per source triangle.
6. Place the 8 new center vertices on a smooth outward dome with max 0.90-unit
   offset.
7. Recreate the exact 11-group schema and preserve every old weight exactly.
8. Bind all new vertices rigidly to the safe patch bone.

Expected:
- TbmC03 = 82V / 138F / 138 TexLinks
- delta = +8V / +16F
- total model bind vertices = 963
- total model faces = 1666
- SMD growth = 1536 bytes
- SMD size = 189040
- inherited non-manifold edge count remains exactly 1
- 8 unrelated object blocks remain byte-identical
- independent reimport uses C002.inx, not M4Bip.inx
- client is never modified by build

Runtime proof is a separate explicit deploy/test step.

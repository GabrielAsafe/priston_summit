# Pikeman Real Geometry — Vertex Gate V1.1

V1 failed before building because its operation validator re-derived the
original Blender bind positions through a second manual matrix implementation.
The difference reached ~0.005994 Blender units.

V1.1 does NOT loosen that validator.

Instead:

1. the helper embeds the exact original Blender coordinates before editing;
2. the builder proves the edit exactly against those embedded coordinates;
3. the builder still converts the edited world position to SMD bone-local XYZ;
4. it still forward-checks the quantized local XYZ with max fit <= 0.01;
5. only the target vertex XYZ byte ranges may change;
6. an additional post-reimport comparator checks the generated SMD against the
   artist Blender geometry vertex-by-vertex.

Controlled operation:

```text
TbmC03
74 vertices / 122 faces
X,Y *= 1.20 around Blender baseline centroid
Z unchanged
```

No client modification occurs until explicit managed deploy.

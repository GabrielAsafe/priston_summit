# G3 V1.2 — Visual witness

G3 V1.1 is structurally successful but its 0.8-unit face-normal poke was not
visually distinguishable in runtime.

V1.2 changes only the witness design:
- source remains pristine TmbC01.smd
- target remains TbmC03 / C002 DefaultModel
- topology remains +1 vertex, +2 faces
- UV center is interpolated
- new vertex remains exactly one rigid bone
- choose the eligible same-bone triangle with maximum centroid Y
- place the new vertex 4.0 Blender units farther in +Y

This intentionally creates an exaggerated runtime witness. It is not an
artist-quality edit.

Expected counts:
75V / 124F / 124 TexLinks.

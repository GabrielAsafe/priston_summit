# G3 Pikeman Free Topology Gate V1.1

V1 failed in the Blender edit helper before SMD serialization.

Cause:
- V1 stored deform weights by vertex-group numeric index.
- after assigning a newly created Mesh datablock to `TbmC03`, Blender 5.1
  presented an empty `obj.vertex_groups` collection in this workflow.
- V1 then attempted `obj.vertex_groups[0]`, causing IndexError.

V1.1 does not relax validation.

Fix:
1. snapshot vertex-group schema by NAME;
2. snapshot every old vertex's weights by GROUP NAME;
3. rebuild the mesh topology;
4. explicitly recreate the complete group-name schema;
5. restore every old weight by name;
6. verify group membership and weight equality (`max_error <= 1e-7`);
7. assign the new vertex to exactly one known rigid bone with weight 1.0.

Controlled topology remains:
74V / 122F -> 75V / 124F
one triangle -> three triangles
+1 new rigid vertex
UV center interpolated
0.80 unit outward spike

No client write occurs during the structural gate.

#!/usr/bin/env python3
import bpy

print("PT_BLENDER_MATERIAL_TEXTURE_PROBE_V1")
print(f"objects={len(bpy.data.objects)} materials={len(bpy.data.materials)} images={len(bpy.data.images)}")

for i, mat in enumerate(bpy.data.materials):
    nodes = []
    if mat.node_tree:
        for n in mat.node_tree.nodes:
            if n.bl_idname == "ShaderNodeTexImage":
                nodes.append(
                    (
                        n.name,
                        n.image.name if n.image else None,
                        n.image.filepath if n.image else None,
                    )
                )
    print(f"MATERIAL_DATA {i}: name={mat.name!r} image_nodes={nodes}")

for obj in bpy.data.objects:
    if obj.type != "MESH":
        continue
    counts = {}
    for p in obj.data.polygons:
        counts[int(p.material_index)] = counts.get(int(p.material_index), 0) + 1
    slots = [m.name if m else None for m in obj.data.materials]
    print(f"MESH {obj.name}: polygon_material_counts={counts} material_slots={slots}")

print("material_texture_probe_check=PASS")

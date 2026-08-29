#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
Create the controlled V5c.2 topology test in Blender (helper v2).

Operation on one triangle:
    (A,B,C)
      ->
    D = centroid(A,B,C)

    (A,B,D)
    (B,C,D)
    (C,A,D)

Safety:
- target object defaults to MahC20;
- selects only a face whose three vertices are rigidly assigned
  to exactly the same Bip* bone with weight 1.0;
- preserves all old vertex coordinates;
- preserves all unaffected face order;
- appends exactly one vertex;
- replaces one face by three consecutive faces;
- preserves old UVs and creates one center UV;
- preserves materials;
- rebuilds old vertex-group weights exactly and assigns only the
  new vertex to the common bone with weight 1.0;
- stores metadata needed by the controlled V5c.2 exporter.
"""

import argparse
import sys
from pathlib import Path

import bpy


EPS = 1e-6


def parse_args():
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    p = argparse.ArgumentParser()
    p.add_argument("--object", default="MahC20")
    p.add_argument("--out-blend", required=True)
    return p.parse_args(argv)


def rigid_bone(obj, vertex_index):
    v = obj.data.vertices[vertex_index]
    active = []

    for membership in v.groups:
        group = obj.vertex_groups[membership.group]
        if not group.name.lower().startswith("bip"):
            continue
        if membership.weight <= EPS:
            continue
        active.append((group.name, float(membership.weight)))

    if len(active) != 1:
        return None

    name, weight = active[0]
    if abs(weight - 1.0) > 1e-5:
        return None

    return name


def capture_vertex_weights(obj):
    captured = {}

    for v in obj.data.vertices:
        memberships = []

        for membership in v.groups:
            group = obj.vertex_groups[membership.group]
            memberships.append(
                (group.name, float(membership.weight))
            )

        captured[int(v.index)] = memberships

    return captured


def choose_face(obj):
    for poly in obj.data.polygons:
        if len(poly.vertices) != 3:
            continue

        bones = [
            rigid_bone(obj, int(vid))
            for vid in poly.vertices
        ]

        if any(b is None for b in bones):
            continue

        if bones[0] == bones[1] == bones[2]:
            return poly, bones[0]

    raise RuntimeError(
        f"{obj.name}: no triangle whose three vertices share "
        "one rigid Bip* bone was found"
    )


def main():
    args = parse_args()

    obj = bpy.data.objects.get(args.object)
    if obj is None or obj.type != "MESH":
        raise RuntimeError(
            f"Mesh object {args.object!r} not found"
        )

    old_mesh = obj.data

    if old_mesh.shape_keys is not None:
        raise RuntimeError(
            f"{obj.name}: shape keys present; controlled helper refuses"
        )

    uv_layer = (
        old_mesh.uv_layers.get("ASE_UV")
        or old_mesh.uv_layers.active
    )
    if uv_layer is None:
        raise RuntimeError(
            f"{obj.name}: no UV layer"
        )

    if any(len(p.vertices) != 3 for p in old_mesh.polygons):
        raise RuntimeError(
            f"{obj.name}: non-triangle polygon present"
        )

    target_poly, common_bone = choose_face(obj)
    target = int(target_poly.index)
    a, b, c = [int(x) for x in target_poly.vertices]

    old_vertex_count = len(old_mesh.vertices)
    old_face_count = len(old_mesh.polygons)
    new_vertex_index = old_vertex_count

    old_coords = [
        tuple(float(x) for x in v.co)
        for v in old_mesh.vertices
    ]

    old_faces = [
        tuple(int(x) for x in p.vertices)
        for p in old_mesh.polygons
    ]

    old_material_indices = [
        int(p.material_index)
        for p in old_mesh.polygons
    ]

    old_face_uvs = []

    for p in old_mesh.polygons:
        uvs = []
        for li in p.loop_indices:
            uv = uv_layer.data[li].uv
            uvs.append((float(uv.x), float(uv.y)))
        old_face_uvs.append(tuple(uvs))

    # Vertex-group names live at object level, but assigning a freshly
    # created Mesh datablock can leave the group collection unavailable or
    # reset depending on Blender version/build. Preserve the complete group
    # table explicitly so we can recreate it deterministically after swap.
    old_group_names = [
        group.name
        for group in obj.vertex_groups
    ]

    old_weights = capture_vertex_weights(obj)

    va = old_coords[a]
    vb = old_coords[b]
    vc = old_coords[c]

    center = (
        (va[0] + vb[0] + vc[0]) / 3.0,
        (va[1] + vb[1] + vc[1]) / 3.0,
        (va[2] + vb[2] + vc[2]) / 3.0,
    )

    uva, uvb, uvc = old_face_uvs[target]

    uv_center = (
        (uva[0] + uvb[0] + uvc[0]) / 3.0,
        (uva[1] + uvb[1] + uvc[1]) / 3.0,
    )

    new_coords = old_coords + [center]

    new_faces = []
    new_face_uvs = []
    new_material_indices = []

    for i, face in enumerate(old_faces):
        if i != target:
            new_faces.append(face)
            new_face_uvs.append(old_face_uvs[i])
            new_material_indices.append(
                old_material_indices[i]
            )
            continue

        mat = old_material_indices[i]

        new_faces.extend([
            (a, b, new_vertex_index),
            (b, c, new_vertex_index),
            (c, a, new_vertex_index),
        ])

        new_face_uvs.extend([
            (uva, uvb, uv_center),
            (uvb, uvc, uv_center),
            (uvc, uva, uv_center),
        ])

        new_material_indices.extend([
            mat,
            mat,
            mat,
        ])

    if len(new_coords) != old_vertex_count + 1:
        raise RuntimeError("Internal vertex-count error")

    if len(new_faces) != old_face_count + 2:
        raise RuntimeError("Internal face-count error")

    # Build a fresh mesh so ordering is deterministic.
    new_mesh = bpy.data.meshes.new(
        old_mesh.name + "_V5C2"
    )
    new_mesh.from_pydata(
        new_coords,
        [],
        new_faces,
    )
    new_mesh.update()

    # Copy material slots in the exact same order.
    for mat in old_mesh.materials:
        new_mesh.materials.append(mat)

    for p, mat_idx in zip(
        new_mesh.polygons,
        new_material_indices,
    ):
        p.material_index = mat_idx

    new_uv = new_mesh.uv_layers.new(
        name=uv_layer.name
    )

    for p, uv_triplet in zip(
        new_mesh.polygons,
        new_face_uvs,
    ):
        if len(p.loop_indices) != 3:
            raise RuntimeError(
                "Fresh split mesh generated non-triangle polygon"
            )

        for li, uv_value in zip(
            p.loop_indices,
            uv_triplet,
        ):
            new_uv.data[li].uv = uv_value

    # Preserve old object custom properties and modifiers by swapping only mesh.
    obj.data = new_mesh

    # Blender 5.x Flatpak builds may clear/reset the deform-group table when a
    # fresh Mesh datablock is assigned. Recreate the COMPLETE original group
    # table explicitly, in original order, before restoring any weights.
    for group in list(obj.vertex_groups):
        obj.vertex_groups.remove(group)

    for group_name in old_group_names:
        obj.vertex_groups.new(name=group_name)

    recreated_names = [
        group.name
        for group in obj.vertex_groups
    ]

    if recreated_names != old_group_names:
        raise RuntimeError(
            "Vertex-group table recreation mismatch:\n"
            f"before={old_group_names}\n"
            f"after ={recreated_names}"
        )

    # Rebuild every old membership by group name.
    for vertex_index, memberships in old_weights.items():
        for group_name, weight in memberships:
            group = obj.vertex_groups.get(group_name)
            if group is None:
                raise RuntimeError(
                    f"Vertex group was not recreated: {group_name!r}"
                )

            group.add(
                [vertex_index],
                weight,
                "REPLACE",
            )

    group = obj.vertex_groups.get(common_bone)
    if group is None:
        raise RuntimeError(
            f"Common bone group {common_bone!r} disappeared"
        )

    group.add(
        [new_vertex_index],
        1.0,
        "REPLACE",
    )

    final_bone = rigid_bone(
        obj,
        new_vertex_index,
    )

    if final_bone != common_bone:
        raise RuntimeError(
            f"New vertex rigid assignment failed: "
            f"expected {common_bone!r}, got {final_bone!r}"
        )

    # Controlled exporter metadata.
    obj["pt_v5c2_operation"] = "SPLIT_ONE_FACE_1_TO_3"
    obj["pt_v5c2_original_face"] = target
    obj["pt_v5c2_new_vertex"] = new_vertex_index
    obj["pt_v5c2_common_bone"] = common_bone
    obj["pt_v5c2_old_vertex_count"] = old_vertex_count
    obj["pt_v5c2_old_face_count"] = old_face_count

    out_blend = Path(
        args.out_blend
    ).expanduser().resolve()

    out_blend.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    bpy.ops.wm.save_as_mainfile(
        filepath=str(out_blend)
    )

    print("=" * 80)
    print("PT V5c.2 CONTROLLED TOPOLOGY TEST")
    print("=" * 80)
    print("Object             :", obj.name)
    print("Operation          : SPLIT_ONE_FACE_1_TO_3")
    print("Original face      :", target)
    print("Original triangle  :", (a, b, c))
    print("Common rigid bone  :", common_bone)
    print()
    print("Old vertices       :", old_vertex_count)
    print("New vertices       :", len(obj.data.vertices))
    print("New vertex index   :", new_vertex_index)
    print("New vertex coord   :", center)
    print()
    print("Old faces          :", old_face_count)
    print("New faces          :", len(obj.data.polygons))
    print("Replacement faces  :")
    print(" ", (a, b, new_vertex_index))
    print(" ", (b, c, new_vertex_index))
    print(" ", (c, a, new_vertex_index))
    print()
    print("Center UV          :", uv_center)
    print("Vertex groups      :", len(old_group_names), "restored")
    print("New vertex bone    :", final_bone)
    print("Saved              :", out_blend)
    print("=" * 80)


if __name__ == "__main__":
    main()

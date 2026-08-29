#!/usr/bin/env python3
"""
Summit Tale / Priestess C01 - controlled topology helper V1

Creates exactly one controlled topology change on MsmC01:
    one triangle -> three triangles
    +1 vertex
    +2 faces

The new vertex:
    - is appended as the last vertex
    - inherits one rigid Physique bone shared by the source face
    - gets UV at the average of the source face corners
    - is optionally displaced along the source face normal for a visible runtime test

This helper modifies only the Blender copy and saves to a NEW .blend.
It does NOT touch SMD/client files.

Recommended input:
  blender/MfbC01_direct_smd_v3_static_armature_clean.blend
"""

import argparse
import hashlib
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default="MsmC01")
    p.add_argument("--face", type=int,
                   help="Exact polygon index. Default: auto-select largest suitable face.")
    p.add_argument("--bone", default="Bip01 Spine1",
                   help="Preferred rigid bone for auto face selection; use '' for any bone.")
    p.add_argument("--offset", type=float, default=0.75,
                   help="Displace new center vertex along source face normal (Blender units).")
    p.add_argument("--source-smd", default="MfbC01.smd",
                   help="Optional source SMD used only to record SHA256 metadata.")
    p.add_argument("--out",
                   default="blender/MfbC01_topology_split_v1.blend")
    return p.parse_args(argv)


def identity_error(m):
    e = 0.0
    for r in range(4):
        for c in range(4):
            want = 1.0 if r == c else 0.0
            e = max(e, abs(float(m[r][c]) - want))
    return e


def rigid_membership(obj, vertex_index):
    v = obj.data.vertices[vertex_index]
    memberships = []
    for elem in v.groups:
        if elem.weight <= 1e-8:
            continue
        if elem.group < 0 or elem.group >= len(obj.vertex_groups):
            raise RuntimeError(f"vertex {vertex_index}: invalid group index {elem.group}")
        memberships.append((obj.vertex_groups[elem.group].name, float(elem.weight)))

    if len(memberships) != 1:
        return None

    name, weight = memberships[0]
    if abs(weight - 1.0) > 1e-5:
        return None
    return name


def capture_vertex_groups(obj):
    group_names = [g.name for g in obj.vertex_groups]
    memberships = []
    for v in obj.data.vertices:
        row = []
        for elem in v.groups:
            if elem.weight > 1e-8:
                row.append((int(elem.group), float(elem.weight)))
        memberships.append(row)
    return group_names, memberships


def restore_vertex_groups(obj, group_names, memberships, new_vertex_group_index):
    # Deterministic recreation, avoiding stale deform tables after mesh swap.
    for vg in list(obj.vertex_groups):
        obj.vertex_groups.remove(vg)

    groups = [obj.vertex_groups.new(name=name) for name in group_names]
    if [g.name for g in groups] != group_names:
        raise RuntimeError("vertex-group name/order recreation mismatch")

    for vidx, rows in enumerate(memberships):
        for gidx, weight in rows:
            groups[gidx].add([vidx], weight, "REPLACE")

    groups[new_vertex_group_index].add([len(memberships)], 1.0, "REPLACE")


def polygon_uvs(mesh, poly, uv_layer):
    return [tuple(float(x) for x in uv_layer.data[li].uv) for li in poly.loop_indices]


def choose_face(obj, preferred_bone, exact_face):
    mesh = obj.data

    if exact_face is not None:
        if exact_face < 0 or exact_face >= len(mesh.polygons):
            raise RuntimeError(f"--face {exact_face} out of range 0..{len(mesh.polygons)-1}")
        candidates = [mesh.polygons[exact_face]]
    else:
        candidates = list(mesh.polygons)

    suitable = []
    for poly in candidates:
        if len(poly.vertices) != 3:
            continue
        bones = [rigid_membership(obj, int(v)) for v in poly.vertices]
        if any(b is None for b in bones):
            continue
        if not (bones[0] == bones[1] == bones[2]):
            continue
        bone = bones[0]
        if preferred_bone and bone != preferred_bone:
            continue
        if float(poly.area) <= 1e-10:
            continue
        suitable.append((float(poly.area), int(poly.index), bone))

    if not suitable and exact_face is None and preferred_bone:
        # Fallback to any common rigid bone, still deterministic.
        return choose_face(obj, "", None)

    if not suitable:
        detail = f"face={exact_face}" if exact_face is not None else f"bone={preferred_bone!r}"
        raise RuntimeError(f"no suitable triangular rigid-Physique face found ({detail})")

    suitable.sort(key=lambda x: (-x[0], x[1]))
    _, index, bone = suitable[0]
    return index, bone


def main(argv):
    args = parse_args(argv)

    obj = bpy.data.objects.get(args.object)
    if obj is None:
        raise RuntimeError(f"object {args.object!r} not found")
    if obj.type != "MESH":
        raise RuntimeError(f"{args.object}: expected MESH, got {obj.type}")

    if identity_error(obj.matrix_world) > 1e-7:
        raise RuntimeError("object matrix must remain identity; use Edit Mode geometry")

    mesh = obj.data
    mesh.update()

    old_v = len(mesh.vertices)
    old_f = len(mesh.polygons)

    saved_v = obj.get("pt_smd_vertex_count")
    saved_f = obj.get("pt_smd_face_count")
    if saved_v is not None and int(saved_v) != old_v:
        raise RuntimeError(
            f"input blend is not pristine for {args.object}: metadata V={saved_v}, actual V={old_v}"
        )
    if saved_f is not None and int(saved_f) != old_f:
        raise RuntimeError(
            f"input blend is not pristine for {args.object}: metadata F={saved_f}, actual F={old_f}"
        )

    if old_v != 125 or old_f != 238:
        raise RuntimeError(
            f"V1 is intentionally restricted to pristine MsmC01 125/238; got {old_v}/{old_f}"
        )

    if len(mesh.uv_layers) != 1:
        raise RuntimeError(f"expected exactly one UV layer, found {len(mesh.uv_layers)}")
    uv_layer = mesh.uv_layers.active
    if uv_layer is None:
        raise RuntimeError("active UV layer missing")

    target_face, common_bone = choose_face(obj, args.bone, args.face)
    poly = mesh.polygons[target_face]
    a, b, c = [int(x) for x in poly.vertices]

    bones = [rigid_membership(obj, x) for x in (a, b, c)]
    if bones != [common_bone, common_bone, common_bone]:
        raise RuntimeError("source-face rigid bone changed during selection")

    group_names, old_memberships = capture_vertex_groups(obj)
    try:
        new_group_index = group_names.index(common_bone)
    except ValueError:
        raise RuntimeError(f"common bone group {common_bone!r} missing")

    old_vertices = [tuple(float(x) for x in v.co) for v in mesh.vertices]
    old_faces = [tuple(int(x) for x in p.vertices) for p in mesh.polygons]
    old_materials = [int(p.material_index) for p in mesh.polygons]
    old_smooth = [bool(p.use_smooth) for p in mesh.polygons]
    old_uvs = [polygon_uvs(mesh, p, uv_layer) for p in mesh.polygons]

    source_coords = [Vector(old_vertices[x]) for x in (a, b, c)]
    center = (source_coords[0] + source_coords[1] + source_coords[2]) / 3.0
    normal = Vector(poly.normal)
    if normal.length <= 1e-12:
        raise RuntimeError("source face has zero normal")
    normal.normalize()
    new_pos = center + normal * float(args.offset)

    uv_a, uv_b, uv_c = [Vector(x) for x in old_uvs[target_face]]
    uv_center = (uv_a + uv_b + uv_c) / 3.0

    new_idx = old_v
    target_material = old_materials[target_face]
    target_smooth = old_smooth[target_face]

    # Preserve every old face index except the target itself.
    new_faces = list(old_faces)
    new_faces[target_face] = (a, b, new_idx)
    new_faces.append((b, c, new_idx))
    new_faces.append((c, a, new_idx))

    new_materials = list(old_materials)
    new_materials[target_face] = target_material
    new_materials.extend([target_material, target_material])

    new_smooth = list(old_smooth)
    new_smooth[target_face] = target_smooth
    new_smooth.extend([target_smooth, target_smooth])

    new_uvs = list(old_uvs)
    new_uvs[target_face] = (tuple(uv_a), tuple(uv_b), tuple(uv_center))
    new_uvs.append((tuple(uv_b), tuple(uv_c), tuple(uv_center)))
    new_uvs.append((tuple(uv_c), tuple(uv_a), tuple(uv_center)))

    new_vertices = list(old_vertices)
    new_vertices.append(tuple(float(x) for x in new_pos))

    old_mesh = mesh
    new_mesh = bpy.data.meshes.new(old_mesh.name + "_TopologySplitV1")
    for mat in old_mesh.materials:
        new_mesh.materials.append(mat)

    new_mesh.from_pydata(new_vertices, [], new_faces)
    new_mesh.update(calc_edges=True)

    uv_new = new_mesh.uv_layers.new(name=uv_layer.name)
    for pidx, p in enumerate(new_mesh.polygons):
        p.material_index = new_materials[pidx]
        p.use_smooth = new_smooth[pidx]
        coords = new_uvs[pidx]
        if len(coords) != len(p.loop_indices):
            raise RuntimeError(f"UV corner count mismatch at polygon {pidx}")
        for li, uv in zip(p.loop_indices, coords):
            uv_new.data[li].uv = uv

    obj.data = new_mesh

    restore_vertex_groups(
        obj,
        group_names,
        old_memberships,
        new_group_index,
    )

    # Validate group state after mesh swap.
    for vidx in range(old_v):
        before = old_memberships[vidx]
        after = [(int(x.group), float(x.weight)) for x in obj.data.vertices[vidx].groups
                 if x.weight > 1e-8]
        if len(before) != len(after):
            raise RuntimeError(f"old vertex {vidx}: group membership count changed")

    if rigid_membership(obj, new_idx) != common_bone:
        raise RuntimeError("new vertex does not have expected rigid bone")

    if len(obj.data.vertices) != old_v + 1:
        raise RuntimeError("post-split vertex count mismatch")
    if len(obj.data.polygons) != old_f + 2:
        raise RuntimeError("post-split face count mismatch")

    expected = [
        (a, b, new_idx),
        (b, c, new_idx),
        (c, a, new_idx),
    ]
    actual = [
        tuple(int(x) for x in obj.data.polygons[target_face].vertices),
        tuple(int(x) for x in obj.data.polygons[-2].vertices),
        tuple(int(x) for x in obj.data.polygons[-1].vertices),
    ]
    if actual != expected:
        raise RuntimeError(f"split face topology mismatch: {actual!r} != {expected!r}")

    source_smd = Path(args.source_smd).expanduser()
    source_sha = ""
    if source_smd.is_file():
        source_sha = hashlib.sha256(source_smd.read_bytes()).hexdigest()

    obj["pt_topology_operation"] = "SPLIT_ONE_FACE_1_TO_3"
    obj["pt_topology_version"] = 1
    obj["pt_topology_source_vertex_count"] = old_v
    obj["pt_topology_source_face_count"] = old_f
    obj["pt_topology_source_face_index"] = target_face
    obj["pt_topology_source_face_a"] = a
    obj["pt_topology_source_face_b"] = b
    obj["pt_topology_source_face_c"] = c
    obj["pt_topology_new_vertex_index"] = new_idx
    obj["pt_topology_common_bone"] = common_bone
    obj["pt_topology_offset"] = float(args.offset)
    obj["pt_topology_source_smd_sha256"] = source_sha

    out = Path(args.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    print("PT_C01_TOPOLOGY_MAKE_SPLIT_V1")
    print("object=", args.object)
    print("operation=SPLIT_ONE_FACE_1_TO_3")
    print("source_vertices=", old_v)
    print("new_vertices=", len(obj.data.vertices))
    print("source_faces=", old_f)
    print("new_faces=", len(obj.data.polygons))
    print("source_face_index=", target_face)
    print("source_face=", (a, b, c))
    print("source_face_area=", f"{float(poly.area):.9g}")
    print("common_bone=", repr(common_bone))
    print("new_vertex_index=", new_idx)
    print("offset=", f"{float(args.offset):.9g}")
    print("new_vertex_world=", tuple(round(float(x), 9) for x in new_pos))
    print("uv_center=", tuple(round(float(x), 9) for x in uv_center))
    print("source_smd_sha256=", source_sha or "NOT_RECORDED")
    print("blend=", out)
    print("topology_helper_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"topology_helper_check=FAIL: {type(exc).__name__}: {exc}")
        raise

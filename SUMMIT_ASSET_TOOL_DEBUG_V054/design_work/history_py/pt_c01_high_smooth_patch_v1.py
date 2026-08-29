#!/usr/bin/env python3
"""
Priestess C01 HIGH smooth patch helper V1.

Goal:
  add useful topology to MshC01 without artificial constant "bump" offsets.

Method:
  - choose a connected patch of pristine MshC01 faces rigidly bound to one bone
  - split every selected triangle into 4 using shared edge midpoints
  - internal patch edges use a PN-style cubic edge midpoint derived from
    equal-weight geometric vertex normals
  - patch-boundary edges remain linear midpoints to avoid cracks/T-junction
    displacement against untouched neighboring faces
  - old 173 vertices remain exactly unchanged
  - all new vertices inherit the common rigid bone
  - UV midpoint is interpolated per face corner, so UV seams remain valid

This only saves a NEW .blend. It never touches SMD/client files.
"""

import argparse
import hashlib
import math
import sys
from collections import defaultdict, deque
from pathlib import Path

import bpy
from mathutils import Vector


SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
TARGET = "MshC01"
SOURCE_V = 173
SOURCE_F = 331


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default=TARGET)
    p.add_argument("--faces", type=int, default=12)
    p.add_argument("--bone", default="Bip01 Spine1")
    p.add_argument("--strength", type=float, default=1.0,
                   help="0=linear subdivision, 1=PN edge midpoint.")
    p.add_argument("--source-smd", default="MfbC01.smd")
    p.add_argument("--out", default="blender/MfbC01_high_smooth_patch_v1.blend")
    return p.parse_args(argv)


def rigid_bone(obj, vi):
    rows = []
    for e in obj.data.vertices[vi].groups:
        if e.weight <= 1e-8:
            continue
        if not (0 <= e.group < len(obj.vertex_groups)):
            raise RuntimeError(f"vertex {vi}: invalid group index {e.group}")
        rows.append((obj.vertex_groups[e.group].name, float(e.weight)))
    if len(rows) != 1 or abs(rows[0][1] - 1.0) > 1e-5:
        return None
    return rows[0][0]


def common_face_bone(obj, poly):
    if len(poly.vertices) != 3:
        return None
    bones = [rigid_bone(obj, int(v)) for v in poly.vertices]
    if any(b is None for b in bones):
        return None
    return bones[0] if bones[0] == bones[1] == bones[2] else None


def all_edge_faces(mesh):
    out = defaultdict(list)
    for p in mesh.polygons:
        ids = [int(v) for v in p.vertices]
        if len(ids) != 3:
            continue
        for k in range(3):
            a, b = ids[k], ids[(k + 1) % 3]
            if a > b:
                a, b = b, a
            out[(a, b)].append(int(p.index))
    return out


def eligible_components(obj, bone):
    mesh = obj.data
    eligible = {
        int(p.index)
        for p in mesh.polygons
        if common_face_bone(obj, p) == bone and float(p.area) > 1e-10
    }
    if not eligible:
        raise RuntimeError(f"no suitable {bone!r} faces in {TARGET}")

    ef = all_edge_faces(mesh)
    adj = {fi: set() for fi in eligible}
    for faces in ef.values():
        if len(faces) != 2:
            continue
        a, b = faces
        if a in eligible and b in eligible:
            adj[a].add(b)
            adj[b].add(a)

    unseen = set(eligible)
    comps = []
    while unseen:
        seed = min(unseen)
        unseen.remove(seed)
        q = deque([seed])
        comp = []
        while q:
            x = q.popleft()
            comp.append(x)
            for n in sorted(adj[x]):
                if n in unseen:
                    unseen.remove(n)
                    q.append(n)
        comps.append(comp)

    comps.sort(key=lambda c: (-len(c), min(c)))
    return comps[0], adj, ef


def choose_connected_patch(obj, bone, requested):
    if requested < 1:
        raise RuntimeError("--faces must be >= 1")

    mesh = obj.data
    comp, adj, ef = eligible_components(obj, bone)
    count = min(requested, len(comp))
    comp_set = set(comp)

    seed = sorted(comp, key=lambda fi: (-float(mesh.polygons[fi].area), fi))[0]
    seen = {seed}
    q = deque([seed])
    chosen = []

    while q and len(chosen) < count:
        fi = q.popleft()
        chosen.append(fi)
        ns = [n for n in adj[fi] if n in comp_set and n not in seen]
        ns.sort(key=lambda n: (-float(mesh.polygons[n].area), n))
        for n in ns:
            seen.add(n)
            q.append(n)

    if len(chosen) != count:
        raise RuntimeError("connected patch selection failed")
    return chosen, len(comp), seed, ef


def equal_geometric_normals(mesh):
    acc = [Vector((0.0, 0.0, 0.0)) for _ in mesh.vertices]
    count = [0] * len(mesh.vertices)

    for p in mesh.polygons:
        if len(p.vertices) != 3:
            raise RuntimeError("non-triangle source polygon")
        ids = [int(v) for v in p.vertices]
        a, b, c = [Vector(mesh.vertices[v].co) for v in ids]
        n = (b - a).cross(c - a)
        if n.length <= 1e-12:
            raise RuntimeError(f"face {p.index}: degenerate")
        n.normalize()
        for v in ids:
            acc[v] += n
            count[v] += 1

    result = []
    for vi, (n, cnt) in enumerate(zip(acc, count)):
        if cnt == 0 or n.length <= 1e-12:
            raise RuntimeError(f"vertex {vi}: no geometric normal")
        n.normalize()
        result.append(n.copy())
    return result


def pn_edge_midpoint(p0, p1, n0, n1, strength):
    """
    Cubic PN-style edge curve evaluated at t=0.5.

    B0 = P0
    B1 = (2P0 + P1 - dot(P1-P0,N0)N0)/3
    B2 = (2P1 + P0 - dot(P0-P1,N1)N1)/3
    B3 = P1

    midpoint = (B0 + 3B1 + 3B2 + B3)/8
    """
    linear = (p0 + p1) * 0.5

    w01 = (p1 - p0).dot(n0)
    w10 = (p0 - p1).dot(n1)
    b1 = (2.0 * p0 + p1 - w01 * n0) / 3.0
    b2 = (2.0 * p1 + p0 - w10 * n1) / 3.0
    curved = (p0 + 3.0 * b1 + 3.0 * b2 + p1) / 8.0

    return linear + (curved - linear) * float(strength)


def capture_groups(obj):
    names = [g.name for g in obj.vertex_groups]
    rows = []
    for v in obj.data.vertices:
        rows.append([
            (int(e.group), float(e.weight))
            for e in v.groups if e.weight > 1e-8
        ])
    return names, rows


def restore_groups(obj, names, old_rows, new_bones):
    for g in list(obj.vertex_groups):
        obj.vertex_groups.remove(g)

    groups = [obj.vertex_groups.new(name=n) for n in names]
    index = {n: i for i, n in enumerate(names)}

    for vi, memberships in enumerate(old_rows):
        for gi, w in memberships:
            groups[gi].add([vi], w, "REPLACE")

    base = len(old_rows)
    for j, bone in enumerate(new_bones):
        if bone not in index:
            raise RuntimeError(f"missing group for new bone {bone!r}")
        groups[index[bone]].add([base + j], 1.0, "REPLACE")


def poly_uvs(mesh, poly, uv_layer):
    return [
        tuple(float(x) for x in uv_layer.data[li].uv)
        for li in poly.loop_indices
    ]


def main(argv):
    args = parse_args(argv)

    if args.object != TARGET:
        raise RuntimeError("smooth patch V1 supports only MshC01")
    if not (0.0 <= args.strength <= 2.0):
        raise RuntimeError("--strength must be between 0 and 2")

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")
    mesh = obj.data
    mesh.update()

    if len(mesh.vertices) != SOURCE_V or len(mesh.polygons) != SOURCE_F:
        raise RuntimeError(
            f"requires pristine MshC01 {SOURCE_V}/{SOURCE_F}; "
            f"got {len(mesh.vertices)}/{len(mesh.polygons)}"
        )
    if len(mesh.uv_layers) != 1 or mesh.uv_layers.active is None:
        raise RuntimeError("requires exactly one active UV layer")
    if any(len(p.vertices) != 3 for p in mesh.polygons):
        raise RuntimeError("source contains non-triangle polygon")

    src = Path(args.source_smd).expanduser().resolve()
    if not src.is_file():
        raise RuntimeError(f"source SMD missing: {src}")
    source_sha = hashlib.sha256(src.read_bytes()).hexdigest()
    if source_sha != SOURCE_SHA:
        raise RuntimeError(f"unexpected source SMD SHA256: {source_sha}")

    selected, component_size, seed, edge_faces = choose_connected_patch(
        obj, args.bone, args.faces
    )
    selected_set = set(selected)

    normals = equal_geometric_normals(mesh)
    uv_layer = mesh.uv_layers.active
    group_names, old_groups = capture_groups(obj)

    old_vertices = [tuple(float(x) for x in v.co) for v in mesh.vertices]
    old_faces = [tuple(int(x) for x in p.vertices) for p in mesh.polygons]
    old_mats = [int(p.material_index) for p in mesh.polygons]
    old_smooth = [bool(p.use_smooth) for p in mesh.polygons]
    old_uvs = [poly_uvs(mesh, p, uv_layer) for p in mesh.polygons]

    # Unique edges used by selected faces.
    patch_edges = set()
    for fi in selected:
        ids = old_faces[fi]
        for k in range(3):
            a, b = ids[k], ids[(k + 1) % 3]
            if a > b:
                a, b = b, a
            patch_edges.add((a, b))

    # Internal edge iff every incident mesh face is in the selected patch
    # and exactly two faces use the edge.
    internal_edges = set()
    boundary_edges = set()
    for edge in patch_edges:
        incident = edge_faces[edge]
        if len(incident) == 2 and all(fi in selected_set for fi in incident):
            internal_edges.add(edge)
        else:
            boundary_edges.add(edge)

    vertices = list(old_vertices)
    new_bones = []
    edge_vertex = {}
    displacement_rows = []

    for edge in sorted(patch_edges):
        a, b = edge
        pa = Vector(old_vertices[a])
        pb = Vector(old_vertices[b])
        linear = (pa + pb) * 0.5

        if edge in internal_edges:
            pos = pn_edge_midpoint(pa, pb, normals[a], normals[b], args.strength)
            mode = "PN_INTERNAL"
        else:
            pos = linear
            mode = "LINEAR_BOUNDARY"

        new_idx = len(vertices)
        vertices.append(tuple(float(x) for x in pos))
        edge_vertex[edge] = new_idx
        new_bones.append(args.bone)

        disp = (pos - linear).length
        displacement_rows.append((edge, new_idx, mode, float(disp), tuple(float(x) for x in pos)))

    # Keep unselected faces at original indices. Replace each selected face
    # with its first child and append the remaining 3 children.
    faces = list(old_faces)
    mats = list(old_mats)
    smooth = list(old_smooth)
    uvs = list(old_uvs)

    appended_faces = []
    appended_mats = []
    appended_smooth = []
    appended_uvs = []

    def eid(x, y):
        return (x, y) if x < y else (y, x)

    for fi in selected:
        a, b, c = old_faces[fi]
        mab = edge_vertex[eid(a, b)]
        mbc = edge_vertex[eid(b, c)]
        mca = edge_vertex[eid(c, a)]

        ua, ub, uc = [Vector(x) for x in old_uvs[fi]]
        uab = (ua + ub) * 0.5
        ubc = (ub + uc) * 0.5
        uca = (uc + ua) * 0.5

        children = [
            ((a, mab, mca), (tuple(ua), tuple(uab), tuple(uca))),
            ((mab, b, mbc), (tuple(uab), tuple(ub), tuple(ubc))),
            ((mca, mbc, c), (tuple(uca), tuple(ubc), tuple(uc))),
            ((mab, mbc, mca), (tuple(uab), tuple(ubc), tuple(uca))),
        ]

        faces[fi] = children[0][0]
        mats[fi] = old_mats[fi]
        smooth[fi] = old_smooth[fi]
        uvs[fi] = children[0][1]

        for child_face, child_uv in children[1:]:
            appended_faces.append(child_face)
            appended_mats.append(old_mats[fi])
            appended_smooth.append(old_smooth[fi])
            appended_uvs.append(child_uv)

    faces.extend(appended_faces)
    mats.extend(appended_mats)
    smooth.extend(appended_smooth)
    uvs.extend(appended_uvs)

    new_mesh = bpy.data.meshes.new(mesh.name + "_SmoothPatchV1")
    for mat in mesh.materials:
        new_mesh.materials.append(mat)
    new_mesh.from_pydata(vertices, [], faces)
    new_mesh.update(calc_edges=True)

    new_uv = new_mesh.uv_layers.new(name=uv_layer.name)
    for pi, poly in enumerate(new_mesh.polygons):
        poly.material_index = mats[pi]
        poly.use_smooth = smooth[pi]
        coords = uvs[pi]
        if len(coords) != 3:
            raise RuntimeError(f"polygon {pi}: UV count mismatch")
        for li, uv in zip(poly.loop_indices, coords):
            new_uv.data[li].uv = uv

    obj.data = new_mesh
    restore_groups(obj, group_names, old_groups, new_bones)

    # Hard gates.
    for vi in range(SOURCE_V):
        after = tuple(float(x) for x in obj.data.vertices[vi].co)
        if after != old_vertices[vi]:
            raise RuntimeError(f"original vertex {vi} moved")

    for j in range(len(new_bones)):
        vi = SOURCE_V + j
        if rigid_bone(obj, vi) != args.bone:
            raise RuntimeError(f"new vertex {vi}: rigid bone verification failed")

    expected_v = SOURCE_V + len(patch_edges)
    expected_f = SOURCE_F + 3 * len(selected)
    if len(obj.data.vertices) != expected_v:
        raise RuntimeError(
            f"vertex count mismatch {len(obj.data.vertices)} != {expected_v}"
        )
    if len(obj.data.polygons) != expected_f:
        raise RuntimeError(
            f"face count mismatch {len(obj.data.polygons)} != {expected_f}"
        )

    disps = [row[3] for row in displacement_rows if row[2] == "PN_INTERNAL"]

    obj["pt_topology_operation"] = "HIGH_PN_EDGE_SUBDIVIDE_1_TO_4"
    obj["pt_topology_helper_version"] = 1
    obj["pt_topology_source_smd_sha256"] = source_sha
    obj["pt_topology_source_vertex_count"] = SOURCE_V
    obj["pt_topology_source_face_count"] = SOURCE_F
    obj["pt_topology_component_size"] = component_size
    obj["pt_topology_seed_face"] = seed
    obj["pt_topology_patch_faces"] = ",".join(map(str, selected))
    obj["pt_topology_bone"] = args.bone
    obj["pt_topology_strength"] = float(args.strength)

    out = Path(args.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    print("PT_C01_HIGH_SMOOTH_PATCH_V1")
    print("object=MshC01")
    print("operation=HIGH_PN_EDGE_SUBDIVIDE_1_TO_4")
    print(f"source_vertices={SOURCE_V}")
    print(f"source_faces={SOURCE_F}")
    print(f"preferred_bone={args.bone!r}")
    print(f"largest_component_faces={component_size}")
    print(f"seed_face={seed}")
    print(f"selected_faces={selected}")
    print(f"selected_face_count={len(selected)}")
    print(f"strength={float(args.strength):.9g}")
    print(f"unique_patch_edges={len(patch_edges)}")
    print(f"internal_curved_edges={len(internal_edges)}")
    print(f"boundary_linear_edges={len(boundary_edges)}")
    print(f"new_vertices={len(obj.data.vertices)}")
    print(f"new_faces={len(obj.data.polygons)}")
    print(f"vertex_delta={len(obj.data.vertices)-SOURCE_V}")
    print(f"face_delta={len(obj.data.polygons)-SOURCE_F:+d}")
    print(f"internal_displacement_min={(min(disps) if disps else 0.0):.9g}")
    print(f"internal_displacement_mean={(sum(disps)/len(disps) if disps else 0.0):.9g}")
    print(f"internal_displacement_max={(max(disps) if disps else 0.0):.9g}")
    for edge, vi, mode, disp, pos in displacement_rows:
        print(
            f"EDGE {edge} -> v={vi} mode={mode} "
            f"disp={disp:.6g} pos={tuple(round(x,6) for x in pos)}"
        )
    print(f"source_smd_sha256={source_sha}")
    print(f"blend={out}")
    print("high_smooth_patch_helper_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"high_smooth_patch_helper_check=FAIL: {type(exc).__name__}: {exc}")
        raise

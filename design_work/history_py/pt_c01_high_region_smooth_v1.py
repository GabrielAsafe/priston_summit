#!/usr/bin/env python3
"""
Priestess C01 HIGH anatomical region smoother V1.

Region:
  upper_chest_v1

Derived from PT_C01_HIGH_ANATOMY_PROBE_V1_1:
  cumulative rings 0..2 = 34 faces
  25 faces are fully rigid Bip01 Spine1
  9 mixed Spine1/Clavicle transition faces are deliberately excluded

This helper subdivides the 25 safe upper-chest faces:
  1 triangle -> 4 triangles using shared edge vertices

Internal region edges:
  PN-style curved midpoint

Region boundary edges:
  linear midpoint, preventing a displacement discontinuity against untouched
  mixed-bone / neighboring faces.

Original 173 MshC01 vertices remain exactly unchanged.
All appended vertices are rigid Bip01 Spine1.

This saves a NEW .blend only. It never modifies SMD/client files.
"""

import argparse
import hashlib
import sys
from collections import defaultdict
from pathlib import Path

import bpy
from mathutils import Vector


SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
TARGET = "MshC01"
SOURCE_V = 173
SOURCE_F = 331
REGION_NAME = "upper_chest_v1"
REGION_BONE = "Bip01 Spine1"

# Proven by anatomy probe:
# Ring 0 common Spine1: 12 faces
# Ring 1 common Spine1:  7 faces
# Ring 2 common Spine1:  6 faces
REGION_FACES = [
    12, 13,
    152, 153, 154,
    160, 161, 162, 163, 164, 165, 166, 167,
    183, 184, 188,
    322, 323, 324, 325, 326, 327, 328, 329, 330,
]

# Explicitly excluded mixed-bone faces from rings 1..2.
MIXED_TRANSITION_FACES = [3, 4, 57, 59, 174, 175, 190, 228, 230]


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default=TARGET)
    p.add_argument("--region", default=REGION_NAME)
    p.add_argument("--strength", type=float, default=1.0)
    p.add_argument("--source-smd", default="MfbC01.smd")
    p.add_argument("--out", default="blender/MfbC01_high_upper_chest_v1.blend")
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
        raise RuntimeError(
            f"vertex {vi}: expected one rigid vertex group weight 1.0; got {rows}"
        )
    return rows[0][0]


def edge(a, b):
    return (a, b) if a < b else (b, a)


def all_edge_faces(mesh):
    result = defaultdict(list)
    for p in mesh.polygons:
        if len(p.vertices) != 3:
            raise RuntimeError(f"polygon {p.index}: source is not triangular")
        a, b, c = [int(v) for v in p.vertices]
        for u, v in ((a, b), (b, c), (c, a)):
            result[edge(u, v)].append(int(p.index))
    return result


def equal_geometric_normals(mesh):
    acc = [Vector((0.0, 0.0, 0.0)) for _ in mesh.vertices]
    counts = [0] * len(mesh.vertices)

    for p in mesh.polygons:
        ids = [int(v) for v in p.vertices]
        a, b, c = [Vector(mesh.vertices[v].co) for v in ids]
        n = (b - a).cross(c - a)
        if n.length <= 1e-12:
            raise RuntimeError(f"polygon {p.index}: degenerate triangle")
        n.normalize()
        for vi in ids:
            acc[vi] += n
            counts[vi] += 1

    out = []
    for vi, n in enumerate(acc):
        if counts[vi] == 0 or n.length <= 1e-12:
            raise RuntimeError(f"vertex {vi}: cannot derive geometric normal")
        n.normalize()
        out.append(n.copy())
    return out


def pn_edge_midpoint(p0, p1, n0, n1, strength):
    linear = (p0 + p1) * 0.5

    w01 = (p1 - p0).dot(n0)
    w10 = (p0 - p1).dot(n1)

    b1 = (2.0 * p0 + p1 - w01 * n0) / 3.0
    b2 = (2.0 * p1 + p0 - w10 * n1) / 3.0

    curved = (p0 + 3.0 * b1 + 3.0 * b2 + p1) / 8.0
    return linear + (curved - linear) * float(strength)


def capture_groups(obj):
    names = [g.name for g in obj.vertex_groups]
    memberships = []
    for v in obj.data.vertices:
        memberships.append([
            (int(e.group), float(e.weight))
            for e in v.groups if e.weight > 1e-8
        ])
    return names, memberships


def restore_groups(obj, names, old_memberships, new_bone_count):
    for g in list(obj.vertex_groups):
        obj.vertex_groups.remove(g)

    groups = [obj.vertex_groups.new(name=name) for name in names]
    name_to_index = {name: idx for idx, name in enumerate(names)}

    for vi, rows in enumerate(old_memberships):
        for gi, weight in rows:
            groups[gi].add([vi], weight, "REPLACE")

    if REGION_BONE not in name_to_index:
        raise RuntimeError(f"required group missing: {REGION_BONE!r}")

    gi = name_to_index[REGION_BONE]
    for vi in range(SOURCE_V, SOURCE_V + new_bone_count):
        groups[gi].add([vi], 1.0, "REPLACE")


def poly_uvs(mesh, poly, uv_layer):
    return [
        tuple(float(x) for x in uv_layer.data[li].uv)
        for li in poly.loop_indices
    ]


def bbox(mesh, vertices):
    pts = [Vector(mesh.vertices[v].co) for v in vertices]
    return {
        "x": (min(p.x for p in pts), max(p.x for p in pts)),
        "y": (min(p.y for p in pts), max(p.y for p in pts)),
        "z": (min(p.z for p in pts), max(p.z for p in pts)),
    }


def main(argv):
    args = parse_args(argv)

    if args.object != TARGET:
        raise RuntimeError("region smoother V1 supports only MshC01")
    if args.region != REGION_NAME:
        raise RuntimeError(
            f"V1 supports only --region {REGION_NAME!r}; got {args.region!r}"
        )
    if not (0.0 <= args.strength <= 2.0):
        raise RuntimeError("--strength must be between 0 and 2")

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    mesh = obj.data
    mesh.update()

    if (len(mesh.vertices), len(mesh.polygons)) != (SOURCE_V, SOURCE_F):
        raise RuntimeError(
            f"requires pristine MshC01 {SOURCE_V}/{SOURCE_F}; "
            f"got {len(mesh.vertices)}/{len(mesh.polygons)}"
        )
    if len(mesh.uv_layers) != 1 or mesh.uv_layers.active is None:
        raise RuntimeError("requires exactly one active UV layer")

    source = Path(args.source_smd).expanduser().resolve()
    if not source.is_file():
        raise RuntimeError(f"source SMD missing: {source}")
    source_sha = hashlib.sha256(source.read_bytes()).hexdigest()
    if source_sha != SOURCE_SHA:
        raise RuntimeError(f"unexpected source SMD SHA256: {source_sha}")

    # Region definition gate.
    if len(REGION_FACES) != 25 or len(set(REGION_FACES)) != 25:
        raise RuntimeError("upper_chest_v1 region definition corrupt")
    if set(REGION_FACES) & set(MIXED_TRANSITION_FACES):
        raise RuntimeError("safe region unexpectedly overlaps mixed-bone faces")

    region_set = set(REGION_FACES)
    region_vertices = set()

    for fi in REGION_FACES:
        if not (0 <= fi < SOURCE_F):
            raise RuntimeError(f"region face {fi}: out of range")
        poly = mesh.polygons[fi]
        ids = [int(v) for v in poly.vertices]
        if len(ids) != 3:
            raise RuntimeError(f"region face {fi}: not triangular")
        region_vertices.update(ids)

        bones = [rigid_bone(obj, vi) for vi in ids]
        if bones != [REGION_BONE, REGION_BONE, REGION_BONE]:
            raise RuntimeError(
                f"region face {fi}: expected all {REGION_BONE!r}; got {bones}"
            )

    edge_faces = all_edge_faces(mesh)
    region_edges = set()
    for fi in REGION_FACES:
        a, b, c = [int(v) for v in mesh.polygons[fi].vertices]
        region_edges.update((edge(a, b), edge(b, c), edge(c, a)))

    # The anatomy-probe-defined region has deterministic topology.
    if len(region_edges) != 43:
        raise RuntimeError(
            f"upper_chest_v1 expected 43 unique edges; got {len(region_edges)}"
        )

    internal_edges = set()
    boundary_edges = set()
    nonmanifold_region_edges = []

    for e in region_edges:
        incident = edge_faces[e]
        if len(incident) > 2:
            nonmanifold_region_edges.append((e, list(incident)))

        if len(incident) >= 2 and all(fi in region_set for fi in incident):
            internal_edges.add(e)
        else:
            boundary_edges.add(e)

    if nonmanifold_region_edges:
        raise RuntimeError(
            f"upper_chest_v1 unexpectedly touches non-manifold edges: "
            f"{nonmanifold_region_edges}"
        )
    if len(internal_edges) != 32 or len(boundary_edges) != 11:
        raise RuntimeError(
            f"upper_chest_v1 edge classification changed: "
            f"internal={len(internal_edges)} boundary={len(boundary_edges)}"
        )

    normals = equal_geometric_normals(mesh)
    uv_layer = mesh.uv_layers.active
    group_names, old_memberships = capture_groups(obj)

    old_vertices = [tuple(float(x) for x in v.co) for v in mesh.vertices]
    old_faces = [tuple(int(x) for x in p.vertices) for p in mesh.polygons]
    old_materials = [int(p.material_index) for p in mesh.polygons]
    old_smooth = [bool(p.use_smooth) for p in mesh.polygons]
    old_uvs = [poly_uvs(mesh, p, uv_layer) for p in mesh.polygons]

    vertices = list(old_vertices)
    edge_vertex = {}
    displacement_rows = []

    for e in sorted(region_edges):
        a, b = e
        pa = Vector(old_vertices[a])
        pb = Vector(old_vertices[b])
        linear = (pa + pb) * 0.5

        if e in internal_edges:
            pos = pn_edge_midpoint(
                pa, pb, normals[a], normals[b], args.strength
            )
            mode = "PN_INTERNAL"
        else:
            pos = linear
            mode = "LINEAR_BOUNDARY"

        new_vi = len(vertices)
        vertices.append(tuple(float(x) for x in pos))
        edge_vertex[e] = new_vi

        displacement_rows.append({
            "edge": e,
            "vertex": new_vi,
            "mode": mode,
            "displacement": float((pos - linear).length),
            "position": tuple(float(x) for x in pos),
        })

    # Preserve unselected source face indices; selected source face index becomes
    # child 0 and remaining 3 children are appended.
    faces = list(old_faces)
    materials = list(old_materials)
    smooth = list(old_smooth)
    uvs = list(old_uvs)

    appended_faces = []
    appended_materials = []
    appended_smooth = []
    appended_uvs = []

    for fi in REGION_FACES:
        a, b, c = old_faces[fi]

        mab = edge_vertex[edge(a, b)]
        mbc = edge_vertex[edge(b, c)]
        mca = edge_vertex[edge(c, a)]

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
        materials[fi] = old_materials[fi]
        smooth[fi] = old_smooth[fi]
        uvs[fi] = children[0][1]

        for child_face, child_uv in children[1:]:
            appended_faces.append(child_face)
            appended_materials.append(old_materials[fi])
            appended_smooth.append(old_smooth[fi])
            appended_uvs.append(child_uv)

    faces.extend(appended_faces)
    materials.extend(appended_materials)
    smooth.extend(appended_smooth)
    uvs.extend(appended_uvs)

    expected_v = SOURCE_V + 43
    expected_f = SOURCE_F + 3 * len(REGION_FACES)

    if len(vertices) != expected_v:
        raise RuntimeError(f"prebuild vertex count {len(vertices)} != {expected_v}")
    if len(faces) != expected_f:
        raise RuntimeError(f"prebuild face count {len(faces)} != {expected_f}")

    new_mesh = bpy.data.meshes.new(mesh.name + "_UpperChestV1")
    for mat in mesh.materials:
        new_mesh.materials.append(mat)

    new_mesh.from_pydata(vertices, [], faces)
    new_mesh.update(calc_edges=True)

    new_uv = new_mesh.uv_layers.new(name=uv_layer.name)
    for pi, poly in enumerate(new_mesh.polygons):
        poly.material_index = materials[pi]
        poly.use_smooth = smooth[pi]

        coords = uvs[pi]
        if len(coords) != 3:
            raise RuntimeError(f"polygon {pi}: UV count mismatch")
        for li, uv in zip(poly.loop_indices, coords):
            new_uv.data[li].uv = uv

    obj.data = new_mesh
    restore_groups(obj, group_names, old_memberships, 43)

    # Original vertices hard gate.
    for vi in range(SOURCE_V):
        after = tuple(float(x) for x in obj.data.vertices[vi].co)
        if after != old_vertices[vi]:
            raise RuntimeError(f"original vertex {vi} moved")

    # Every appended vertex must be rigid Spine1.
    for vi in range(SOURCE_V, expected_v):
        if rigid_bone(obj, vi) != REGION_BONE:
            raise RuntimeError(f"new vertex {vi}: rigid bone verification failed")

    if (len(obj.data.vertices), len(obj.data.polygons)) != (expected_v, expected_f):
        raise RuntimeError("postbuild topology count mismatch")

    internal_disps = [
        row["displacement"] for row in displacement_rows
        if row["mode"] == "PN_INTERNAL"
    ]

    obj["pt_region_name"] = REGION_NAME
    obj["pt_region_version"] = 1
    obj["pt_region_operation"] = "HIGH_ANATOMICAL_PN_SUBDIVIDE_1_TO_4"
    obj["pt_region_source_smd_sha256"] = source_sha
    obj["pt_region_faces"] = ",".join(map(str, REGION_FACES))
    obj["pt_region_mixed_transition_faces_excluded"] = ",".join(
        map(str, MIXED_TRANSITION_FACES)
    )
    obj["pt_region_bone"] = REGION_BONE
    obj["pt_region_strength"] = float(args.strength)

    out = Path(args.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    box = bbox(obj.data, sorted(region_vertices))

    print("PT_C01_HIGH_REGION_SMOOTH_V1")
    print("object=MshC01")
    print(f"region={REGION_NAME}")
    print("operation=HIGH_ANATOMICAL_PN_SUBDIVIDE_1_TO_4")
    print(f"region_bone={REGION_BONE!r}")
    print(f"region_faces={REGION_FACES}")
    print(f"region_face_count={len(REGION_FACES)}")
    print(f"mixed_transition_faces_excluded={MIXED_TRANSITION_FACES}")
    print(f"mixed_transition_face_count={len(MIXED_TRANSITION_FACES)}")
    print(f"region_original_vertex_count={len(region_vertices)}")
    print(
        "region_bbox="
        f"x=[{box['x'][0]:.6f},{box['x'][1]:.6f}] "
        f"y=[{box['y'][0]:.6f},{box['y'][1]:.6f}] "
        f"z=[{box['z'][0]:.6f},{box['z'][1]:.6f}]"
    )
    print(f"strength={float(args.strength):.9g}")
    print(f"unique_region_edges={len(region_edges)}")
    print(f"internal_curved_edges={len(internal_edges)}")
    print(f"boundary_linear_edges={len(boundary_edges)}")
    print(f"source_vertices={SOURCE_V}")
    print(f"source_faces={SOURCE_F}")
    print(f"new_vertices={len(obj.data.vertices)}")
    print(f"new_faces={len(obj.data.polygons)}")
    print(f"vertex_delta={len(obj.data.vertices) - SOURCE_V:+d}")
    print(f"face_delta={len(obj.data.polygons) - SOURCE_F:+d}")
    print(
        f"internal_displacement_min="
        f"{(min(internal_disps) if internal_disps else 0.0):.9g}"
    )
    print(
        f"internal_displacement_mean="
        f"{(sum(internal_disps)/len(internal_disps) if internal_disps else 0.0):.9g}"
    )
    print(
        f"internal_displacement_max="
        f"{(max(internal_disps) if internal_disps else 0.0):.9g}"
    )
    print(f"source_smd_sha256={source_sha}")
    print(f"blend={out}")
    print("high_region_smooth_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"high_region_smooth_check=FAIL: {type(exc).__name__}: {exc}")
        raise

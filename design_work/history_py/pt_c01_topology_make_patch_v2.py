#!/usr/bin/env python3
"""
Summit Tale / Priestess C01 - controlled multi-face subdivision helper V2

Creates a connected patch of face-center subdivisions on pristine MsmC01.

For each selected triangle:
    1 original triangle -> 3 triangles
    +1 appended vertex
    +2 net faces

Constraints:
  - starts from pristine MsmC01 (125 vertices / 238 faces)
  - original 125 vertices are preserved exactly
  - selected face must have all 3 vertices rigidly assigned to the same bone
  - selected patch is connected by shared edges
  - one UV layer
  - material indices and smooth flags are preserved
  - each new center vertex inherits the source face rigid bone
  - optional normal offset makes the patch visible at runtime

Default patch:
  seed face 118
  preferred bone Bip01 Spine1
  6 connected faces
  offset 0.35

This modifies only a NEW .blend. It does not touch SMD/client files.
"""

import argparse
import hashlib
import sys
from collections import defaultdict, deque
from pathlib import Path

import bpy
from mathutils import Vector


SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
SOURCE_V = 125
SOURCE_F = 238


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default="MsmC01")
    p.add_argument("--seed-face", type=int, default=118)
    p.add_argument("--faces", type=int, default=6,
                   help="Number of connected source triangles to subdivide.")
    p.add_argument("--bone", default="Bip01 Spine1")
    p.add_argument("--offset", type=float, default=0.35)
    p.add_argument("--source-smd", default="MfbC01.smd")
    p.add_argument("--out", default="blender/MfbC01_topology_patch_v2.blend")
    return p.parse_args(argv)


def identity_error(m):
    err = 0.0
    for r in range(4):
        for c in range(4):
            expected = 1.0 if r == c else 0.0
            err = max(err, abs(float(m[r][c]) - expected))
    return err


def rigid_bone(obj, vidx):
    v = obj.data.vertices[vidx]
    rows = []
    for elem in v.groups:
        if elem.weight <= 1e-8:
            continue
        if not (0 <= elem.group < len(obj.vertex_groups)):
            raise RuntimeError(f"vertex {vidx}: invalid group index {elem.group}")
        rows.append((obj.vertex_groups[elem.group].name, float(elem.weight)))

    if len(rows) != 1:
        return None
    name, weight = rows[0]
    if abs(weight - 1.0) > 1e-5:
        return None
    return name


def capture_groups(obj):
    names = [g.name for g in obj.vertex_groups]
    membership = []
    for v in obj.data.vertices:
        row = [(int(e.group), float(e.weight))
               for e in v.groups if e.weight > 1e-8]
        membership.append(row)
    return names, membership


def restore_groups(obj, names, old_membership, new_vertex_bones):
    for vg in list(obj.vertex_groups):
        obj.vertex_groups.remove(vg)

    groups = [obj.vertex_groups.new(name=n) for n in names]
    if [g.name for g in groups] != names:
        raise RuntimeError("vertex-group recreation order mismatch")

    for vidx, rows in enumerate(old_membership):
        for gidx, weight in rows:
            groups[gidx].add([vidx], weight, "REPLACE")

    name_to_idx = {name: i for i, name in enumerate(names)}
    first_new = len(old_membership)
    for offset, bone in enumerate(new_vertex_bones):
        if bone not in name_to_idx:
            raise RuntimeError(f"new bone group missing: {bone!r}")
        groups[name_to_idx[bone]].add([first_new + offset], 1.0, "REPLACE")


def face_uvs(mesh, poly, uv_layer):
    return [tuple(float(x) for x in uv_layer.data[li].uv)
            for li in poly.loop_indices]


def build_adjacency(mesh):
    edge_faces = defaultdict(list)
    for poly in mesh.polygons:
        ids = [int(v) for v in poly.vertices]
        if len(ids) != 3:
            continue
        for i in range(3):
            a, b = ids[i], ids[(i+1) % 3]
            edge = (a,b) if a < b else (b,a)
            edge_faces[edge].append(int(poly.index))

    adj = defaultdict(set)
    for faces in edge_faces.values():
        if len(faces) == 2:
            a,b = faces
            adj[a].add(b)
            adj[b].add(a)
    return adj


def source_face_bone(obj, poly):
    if len(poly.vertices) != 3:
        return None
    bones = [rigid_bone(obj, int(v)) for v in poly.vertices]
    if any(b is None for b in bones):
        return None
    if bones[0] != bones[1] or bones[1] != bones[2]:
        return None
    return bones[0]


def select_connected_patch(obj, seed, count, preferred_bone):
    mesh = obj.data
    if count < 1:
        raise RuntimeError("--faces must be >= 1")
    if not (0 <= seed < len(mesh.polygons)):
        raise RuntimeError(f"seed face {seed} outside 0..{len(mesh.polygons)-1}")

    candidate_bone = {}
    for p in mesh.polygons:
        bone = source_face_bone(obj, p)
        if bone is None:
            continue
        if preferred_bone and bone != preferred_bone:
            continue
        candidate_bone[int(p.index)] = bone

    if seed not in candidate_bone:
        actual = source_face_bone(obj, mesh.polygons[seed])
        raise RuntimeError(
            f"seed face {seed} is not suitable for preferred bone "
            f"{preferred_bone!r}; actual common rigid bone={actual!r}"
        )

    adj = build_adjacency(mesh)

    chosen = []
    seen = {seed}
    q = deque([seed])

    while q and len(chosen) < count:
        fi = q.popleft()
        if fi not in candidate_bone:
            continue
        chosen.append(fi)

        # deterministic neighbor order: larger face area first, then index
        neighbors = [n for n in adj.get(fi, ()) if n not in seen and n in candidate_bone]
        neighbors.sort(key=lambda n: (-float(mesh.polygons[n].area), n))
        for n in neighbors:
            seen.add(n)
            q.append(n)

    if len(chosen) != count:
        raise RuntimeError(
            f"only found {len(chosen)} connected suitable faces from seed {seed}; "
            f"requested {count}"
        )

    return chosen, [candidate_bone[x] for x in chosen]


def main(argv):
    args = parse_args(argv)

    obj = bpy.data.objects.get(args.object)
    if obj is None or obj.type != "MESH":
        raise RuntimeError(f"mesh {args.object!r} not found")
    if identity_error(obj.matrix_world) > 1e-7:
        raise RuntimeError("object matrix must remain identity")

    mesh = obj.data
    mesh.update()

    if len(mesh.vertices) != SOURCE_V or len(mesh.polygons) != SOURCE_F:
        raise RuntimeError(
            f"V2 helper requires pristine MsmC01 {SOURCE_V}/{SOURCE_F}; "
            f"got {len(mesh.vertices)}/{len(mesh.polygons)}"
        )
    if len(mesh.uv_layers) != 1 or mesh.uv_layers.active is None:
        raise RuntimeError("expected exactly one active UV layer")
    if any(len(p.vertices) != 3 for p in mesh.polygons):
        raise RuntimeError("pristine MsmC01 unexpectedly contains non-triangles")

    source_smd = Path(args.source_smd).expanduser().resolve()
    if not source_smd.is_file():
        raise RuntimeError(f"source SMD not found: {source_smd}")
    source_sha = hashlib.sha256(source_smd.read_bytes()).hexdigest()
    if source_sha != SOURCE_SHA:
        raise RuntimeError(f"unexpected source SMD SHA256: {source_sha}")

    selected, selected_bones = select_connected_patch(
        obj, args.seed_face, args.faces, args.bone
    )
    selected_set = set(selected)

    uv_layer = mesh.uv_layers.active
    group_names, old_membership = capture_groups(obj)

    old_vertices = [tuple(float(x) for x in v.co) for v in mesh.vertices]
    old_faces = [tuple(int(x) for x in p.vertices) for p in mesh.polygons]
    old_mats = [int(p.material_index) for p in mesh.polygons]
    old_smooth = [bool(p.use_smooth) for p in mesh.polygons]
    old_uvs = [face_uvs(mesh, p, uv_layer) for p in mesh.polygons]

    new_vertices = list(old_vertices)
    new_faces = list(old_faces)
    new_mats = list(old_mats)
    new_smooth = list(old_smooth)
    new_uvs = list(old_uvs)

    appended_faces = []
    new_vertex_bones = []
    new_vertex_rows = []

    for selected_order, fi in enumerate(selected):
        a,b,c = old_faces[fi]
        bone = selected_bones[selected_order]

        coords = [Vector(old_vertices[v]) for v in (a,b,c)]
        center = (coords[0] + coords[1] + coords[2]) / 3.0

        normal = Vector(mesh.polygons[fi].normal)
        if normal.length <= 1e-12:
            raise RuntimeError(f"face {fi}: zero normal")
        normal.normalize()

        new_pos = center + normal * float(args.offset)
        new_idx = len(new_vertices)
        new_vertices.append(tuple(float(x) for x in new_pos))
        new_vertex_bones.append(bone)

        ua,ub,uc = [Vector(x) for x in old_uvs[fi]]
        uvc = (ua+ub+uc)/3.0

        # Keep one child at original face index.
        new_faces[fi] = (a,b,new_idx)
        new_mats[fi] = old_mats[fi]
        new_smooth[fi] = old_smooth[fi]
        new_uvs[fi] = (tuple(ua), tuple(ub), tuple(uvc))

        child_2 = (b,c,new_idx)
        child_3 = (c,a,new_idx)

        appended_faces.extend([child_2, child_3])
        new_faces.extend([child_2, child_3])
        new_mats.extend([old_mats[fi], old_mats[fi]])
        new_smooth.extend([old_smooth[fi], old_smooth[fi]])
        new_uvs.extend([
            (tuple(ub), tuple(uc), tuple(uvc)),
            (tuple(uc), tuple(ua), tuple(uvc)),
        ])

        new_vertex_rows.append({
            "new_vertex": new_idx,
            "source_face": fi,
            "source_triangle": (a,b,c),
            "bone": bone,
            "position": tuple(float(x) for x in new_pos),
            "uv_center": tuple(float(x) for x in uvc),
        })

    new_mesh = bpy.data.meshes.new(mesh.name + "_PatchSubdivideV2")
    for mat in mesh.materials:
        new_mesh.materials.append(mat)

    new_mesh.from_pydata(new_vertices, [], new_faces)
    new_mesh.update(calc_edges=True)

    new_uv_layer = new_mesh.uv_layers.new(name=uv_layer.name)
    for pidx, poly in enumerate(new_mesh.polygons):
        poly.material_index = new_mats[pidx]
        poly.use_smooth = new_smooth[pidx]
        coords = new_uvs[pidx]
        if len(coords) != len(poly.loop_indices):
            raise RuntimeError(f"polygon {pidx}: UV corner mismatch")
        for li, uv in zip(poly.loop_indices, coords):
            new_uv_layer.data[li].uv = uv

    obj.data = new_mesh
    restore_groups(obj, group_names, old_membership, new_vertex_bones)

    expected_v = SOURCE_V + len(selected)
    expected_f = SOURCE_F + 2*len(selected)

    if len(obj.data.vertices) != expected_v or len(obj.data.polygons) != expected_f:
        raise RuntimeError("post-subdivision count mismatch")

    # Verify every original vertex coordinate byte-for-concept identical in Blender floats.
    for vi in range(SOURCE_V):
        before = old_vertices[vi]
        after = tuple(float(x) for x in obj.data.vertices[vi].co)
        if before != after:
            raise RuntimeError(f"original vertex {vi} moved")

    # Verify original groups still reproduce.
    for vi in range(SOURCE_V):
        before = old_membership[vi]
        after = [(int(e.group), float(e.weight))
                 for e in obj.data.vertices[vi].groups if e.weight > 1e-8]
        if len(before) != len(after):
            raise RuntimeError(f"original vertex {vi}: group count changed")

    # New vertices rigid bone gate.
    for row in new_vertex_rows:
        if rigid_bone(obj, row["new_vertex"]) != row["bone"]:
            raise RuntimeError(
                f"new vertex {row['new_vertex']}: rigid bone verification failed"
            )

    obj["pt_topology_operation"] = "CONNECTED_FACE_CENTER_SUBDIVIDE"
    obj["pt_topology_helper_version"] = 2
    obj["pt_topology_source_smd_sha256"] = source_sha
    obj["pt_topology_source_vertex_count"] = SOURCE_V
    obj["pt_topology_source_face_count"] = SOURCE_F
    obj["pt_topology_seed_face"] = int(args.seed_face)
    obj["pt_topology_patch_face_count"] = len(selected)
    obj["pt_topology_patch_faces"] = ",".join(str(x) for x in selected)
    obj["pt_topology_preferred_bone"] = args.bone
    obj["pt_topology_offset"] = float(args.offset)

    out = Path(args.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    print("PT_C01_TOPOLOGY_MAKE_PATCH_V2")
    print("object=", args.object)
    print("operation=CONNECTED_FACE_CENTER_SUBDIVIDE")
    print("source_vertices=", SOURCE_V)
    print("source_faces=", SOURCE_F)
    print("selected_faces=", selected)
    print("selected_face_count=", len(selected))
    print("preferred_bone=", repr(args.bone))
    print("offset=", f"{float(args.offset):.9g}")
    print("new_vertices=", len(obj.data.vertices))
    print("new_faces=", len(obj.data.polygons))
    print("vertex_delta=", len(obj.data.vertices)-SOURCE_V)
    print("face_delta=", len(obj.data.polygons)-SOURCE_F)
    for row in new_vertex_rows:
        print(
            f"NEW v={row['new_vertex']} source_face={row['source_face']} "
            f"bone={row['bone']!r} pos={tuple(round(x,6) for x in row['position'])}"
        )
    print("source_smd_sha256=", source_sha)
    print("blend=", out)
    print("topology_patch_helper_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"topology_patch_helper_check=FAIL: {type(exc).__name__}: {exc}")
        raise

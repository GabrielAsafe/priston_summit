#!/usr/bin/env python3
"""
PT C01 HIGH FREE TOPOLOGY REINDEX HELPER V1

Purpose
-------
Create a visually identical MshC01 whose vertex indices and face ordering are
completely different from the pristine asset.

This is the first FREE TOPOLOGY gate:
  - vertex count stays 173
  - face count stays 331
  - geometry stays visually identical
  - UV/material/rig stay semantically identical
  - every vertex is reindexed
  - every face is moved to a different position in the face array

The helper writes only a new .blend. It never modifies SMD/client files.
"""

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path

import bpy

SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
TARGET = "MshC01"
SOURCE_V = 173
SOURCE_F = 331
VERTEX_SHIFT_DEFAULT = 37
FACE_SHIFT_DEFAULT = 73


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default=TARGET)
    p.add_argument("--source-smd", default="MfbC01.smd")
    p.add_argument("--vertex-shift", type=int, default=VERTEX_SHIFT_DEFAULT)
    p.add_argument("--face-shift", type=int, default=FACE_SHIFT_DEFAULT)
    p.add_argument("--out", default="blender/MfbC01_high_free_topology_reindex_v1.blend")
    return p.parse_args(argv)


def rigid_bone(obj, vi):
    rows = []
    for e in obj.data.vertices[vi].groups:
        if e.weight <= 1e-8:
            continue
        if not (0 <= e.group < len(obj.vertex_groups)):
            raise RuntimeError(f"vertex {vi}: invalid vertex-group index {e.group}")
        rows.append((obj.vertex_groups[e.group].name, float(e.weight)))

    if len(rows) != 1 or abs(rows[0][1] - 1.0) > 1e-5:
        raise RuntimeError(
            f"vertex {vi}: expected exactly one rigid group weight 1.0; got {rows}"
        )
    return rows[0][0]


def capture_groups(obj):
    names = [g.name for g in obj.vertex_groups]
    memberships = []
    for v in obj.data.vertices:
        memberships.append([
            (int(e.group), float(e.weight))
            for e in v.groups if e.weight > 1e-8
        ])
    return names, memberships


def restore_groups_reindexed(obj, names, old_memberships, new_to_old):
    for g in list(obj.vertex_groups):
        obj.vertex_groups.remove(g)

    groups = [obj.vertex_groups.new(name=name) for name in names]

    for new_vi, old_vi in enumerate(new_to_old):
        for gi, weight in old_memberships[old_vi]:
            groups[gi].add([new_vi], weight, "REPLACE")


def polygon_uvs(mesh, poly, uv_layer):
    return [
        (float(uv_layer.data[li].uv.x), float(uv_layer.data[li].uv.y))
        for li in poly.loop_indices
    ]


def canonical_mesh_digest(obj):
    mesh = obj.data
    if mesh.uv_layers.active is None:
        raise RuntimeError("active UV layer missing")
    uv = mesh.uv_layers.active

    h = hashlib.sha256()
    h.update(b"PT_FREE_TOPOLOGY_REINDEX_STATE_V1\0")
    h.update(struct.pack("<II", len(mesh.vertices), len(mesh.polygons)))

    for v in mesh.vertices:
        h.update(struct.pack("<3f", float(v.co.x), float(v.co.y), float(v.co.z)))

        rows = []
        for e in v.groups:
            if e.weight <= 1e-8:
                continue
            name = obj.vertex_groups[e.group].name
            rows.append((name, float(e.weight)))
        rows.sort(key=lambda x: x[0])

        h.update(struct.pack("<I", len(rows)))
        for name, weight in rows:
            raw = name.encode("utf-8")
            h.update(struct.pack("<I", len(raw)))
            h.update(raw)
            h.update(struct.pack("<f", weight))

    for p in mesh.polygons:
        ids = tuple(int(x) for x in p.vertices)
        if len(ids) != 3:
            raise RuntimeError(f"polygon {p.index}: non-triangle")
        h.update(struct.pack("<3I", *ids))
        h.update(struct.pack("<I?", int(p.material_index), bool(p.use_smooth)))

        uvs = polygon_uvs(mesh, p, uv)
        if len(uvs) != 3:
            raise RuntimeError(f"polygon {p.index}: expected 3 UV corners")
        for u, v in uvs:
            h.update(struct.pack("<2f", u, v))

    return h.hexdigest()


def main(argv):
    args = parse_args(argv)

    if args.object != TARGET:
        raise RuntimeError("V1 supports only MshC01")

    source = Path(args.source_smd).expanduser().resolve()
    if not source.is_file():
        raise RuntimeError(f"source SMD missing: {source}")

    source_sha = hashlib.sha256(source.read_bytes()).hexdigest()
    if source_sha != SOURCE_SHA:
        raise RuntimeError(f"unexpected source SMD SHA256: {source_sha}")

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

    for p in mesh.polygons:
        if len(p.vertices) != 3:
            raise RuntimeError(f"polygon {p.index}: source is not triangular")

    vertex_shift = args.vertex_shift % SOURCE_V
    face_shift = args.face_shift % SOURCE_F

    if vertex_shift == 0:
        raise RuntimeError("vertex shift must be non-zero modulo 173")
    if face_shift == 0:
        raise RuntimeError("face shift must be non-zero modulo 331")

    # new index j receives old vertex (j + shift) mod N.
    new_to_old = [(j + vertex_shift) % SOURCE_V for j in range(SOURCE_V)]
    old_to_new = [None] * SOURCE_V
    for new_i, old_i in enumerate(new_to_old):
        old_to_new[old_i] = new_i

    if sorted(new_to_old) != list(range(SOURCE_V)):
        raise RuntimeError("vertex permutation is not bijective")

    fixed_vertices = [i for i in range(SOURCE_V) if old_to_new[i] == i]
    if fixed_vertices:
        raise RuntimeError(f"vertex permutation unexpectedly has fixed points: {fixed_vertices}")

    new_face_to_old = [(j + face_shift) % SOURCE_F for j in range(SOURCE_F)]
    old_face_to_new = [None] * SOURCE_F
    for new_i, old_i in enumerate(new_face_to_old):
        old_face_to_new[old_i] = new_i

    if sorted(new_face_to_old) != list(range(SOURCE_F)):
        raise RuntimeError("face permutation is not bijective")

    fixed_faces = [i for i in range(SOURCE_F) if old_face_to_new[i] == i]
    if fixed_faces:
        raise RuntimeError(f"face permutation unexpectedly has fixed points: {fixed_faces}")

    old_vertices = [
        (float(v.co.x), float(v.co.y), float(v.co.z))
        for v in mesh.vertices
    ]
    old_faces = [tuple(int(x) for x in p.vertices) for p in mesh.polygons]
    old_materials = [int(p.material_index) for p in mesh.polygons]
    old_smooth = [bool(p.use_smooth) for p in mesh.polygons]
    uv_layer = mesh.uv_layers.active
    old_uvs = [polygon_uvs(mesh, p, uv_layer) for p in mesh.polygons]

    group_names, old_memberships = capture_groups(obj)
    old_bones = [rigid_bone(obj, i) for i in range(SOURCE_V)]

    new_vertices = [old_vertices[old_i] for old_i in new_to_old]
    new_faces = []
    new_materials = []
    new_smooth = []
    new_uvs = []

    for new_fi, old_fi in enumerate(new_face_to_old):
        old_face = old_faces[old_fi]
        new_face = tuple(old_to_new[old_vi] for old_vi in old_face)
        new_faces.append(new_face)
        new_materials.append(old_materials[old_fi])
        new_smooth.append(old_smooth[old_fi])
        new_uvs.append(old_uvs[old_fi])

    old_mesh = mesh
    new_mesh = bpy.data.meshes.new(old_mesh.name + "_FreeTopologyReindexV1")

    for mat in old_mesh.materials:
        new_mesh.materials.append(mat)

    new_mesh.from_pydata(new_vertices, [], new_faces)
    new_mesh.update(calc_edges=True)

    new_uv_layer = new_mesh.uv_layers.new(name=uv_layer.name)

    for pi, p in enumerate(new_mesh.polygons):
        p.material_index = new_materials[pi]
        p.use_smooth = new_smooth[pi]
        for li, uvco in zip(p.loop_indices, new_uvs[pi]):
            new_uv_layer.data[li].uv = uvco

    obj.data = new_mesh
    restore_groups_reindexed(obj, group_names, old_memberships, new_to_old)

    # Strong semantic validation after swap.
    if (len(obj.data.vertices), len(obj.data.polygons)) != (SOURCE_V, SOURCE_F):
        raise RuntimeError("postbuild count mismatch")

    max_position_error = 0.0
    for new_i, old_i in enumerate(new_to_old):
        got = obj.data.vertices[new_i].co
        ref = old_vertices[old_i]
        err = max(abs(float(got[k]) - ref[k]) for k in range(3))
        max_position_error = max(max_position_error, err)

        got_bone = rigid_bone(obj, new_i)
        if got_bone != old_bones[old_i]:
            raise RuntimeError(
                f"vertex {new_i}: bone mismatch {got_bone!r} != {old_bones[old_i]!r}"
            )

    if max_position_error != 0.0:
        raise RuntimeError(f"reindexed geometry changed: max error={max_position_error}")

    for new_fi, old_fi in enumerate(new_face_to_old):
        got = tuple(int(x) for x in obj.data.polygons[new_fi].vertices)
        expected = tuple(old_to_new[x] for x in old_faces[old_fi])
        if got != expected:
            raise RuntimeError(
                f"face {new_fi}: topology mismatch {got} != {expected}"
            )

    state_sha = canonical_mesh_digest(obj)

    obj["pt_free_topology_operation"] = "FULL_REINDEX_VISUAL_EQUIVALENCE"
    obj["pt_free_topology_version"] = 1
    obj["pt_free_topology_source_smd_sha256"] = source_sha
    obj["pt_free_topology_source_vertices"] = SOURCE_V
    obj["pt_free_topology_source_faces"] = SOURCE_F
    obj["pt_free_topology_vertex_shift"] = vertex_shift
    obj["pt_free_topology_face_shift"] = face_shift
    obj["pt_free_topology_new_to_old_vertices"] = ",".join(map(str, new_to_old))
    obj["pt_free_topology_old_to_new_vertices"] = ",".join(map(str, old_to_new))
    obj["pt_free_topology_new_to_old_faces"] = ",".join(map(str, new_face_to_old))
    obj["pt_free_topology_old_to_new_faces"] = ",".join(map(str, old_face_to_new))
    obj["pt_free_topology_state_sha256"] = state_sha

    out = Path(args.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    # Recompute immediately after save; same in-memory state must remain valid.
    post_save_sha = canonical_mesh_digest(obj)
    if post_save_sha != state_sha:
        raise RuntimeError("mesh state changed while saving")

    print("PT_C01_HIGH_FREE_TOPOLOGY_REINDEX_HELPER_V1")
    print("object=MshC01")
    print("operation=FULL_REINDEX_VISUAL_EQUIVALENCE")
    print(f"source_vertices={SOURCE_V}")
    print(f"source_faces={SOURCE_F}")
    print(f"vertex_shift={vertex_shift}")
    print(f"face_shift={face_shift}")
    print(f"fixed_vertex_indices={fixed_vertices}")
    print(f"fixed_face_indices={fixed_faces}")
    print(f"reindexed_vertices={SOURCE_V}")
    print(f"reordered_faces={SOURCE_F}")
    print(f"max_position_error={max_position_error:.12g}")
    print(f"vertex_groups={len(group_names)}")
    print(f"source_smd_sha256={source_sha}")
    print(f"mesh_state_sha256={state_sha}")
    print(f"blend={out}")
    print("free_topology_reindex_helper_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"free_topology_reindex_helper_check=FAIL: {type(exc).__name__}: {exc}")
        raise

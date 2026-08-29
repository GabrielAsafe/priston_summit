#!/usr/bin/env python3
"""
PT HIGH MESH FULL REINDEX GATE V2

Generic controlled reindex helper for an already-valid MshC01 Blender mesh.

Unlike V1, this does NOT require pristine 173/331 counts. It accepts any
triangular MshC01 mesh with:
  - exactly one active UV layer
  - rigid vertex groups (one bone at weight 1.0 per vertex)
  - existing Blender materials

It changes only storage identity:
  - every vertex index is changed
  - every face-array position is changed
  - geometry, UVs, materials and rigid assignments stay equivalent

Writes a new .blend only.
"""

import argparse
import hashlib
import struct
import sys
from pathlib import Path

import bpy

TARGET = "MshC01"


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default=TARGET)
    p.add_argument("--vertex-shift", type=int, default=47)
    p.add_argument("--face-shift", type=int, default=89)
    p.add_argument("--out", required=True)
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
            f"vertex {vi}: expected exactly one rigid group at 1.0; got {rows}"
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


def restore_groups(obj, names, memberships, new_to_old):
    for g in list(obj.vertex_groups):
        obj.vertex_groups.remove(g)
    groups = [obj.vertex_groups.new(name=n) for n in names]
    for new_vi, old_vi in enumerate(new_to_old):
        for gi, weight in memberships[old_vi]:
            groups[gi].add([new_vi], weight, "REPLACE")


def poly_uvs(mesh, poly, uv_layer):
    return [
        (float(uv_layer.data[li].uv.x), float(uv_layer.data[li].uv.y))
        for li in poly.loop_indices
    ]


def state_digest(obj):
    mesh = obj.data
    uv = mesh.uv_layers.active
    h = hashlib.sha256()
    h.update(b"PT_HIGH_FULL_REINDEX_GATE_V2\0")
    h.update(struct.pack("<II", len(mesh.vertices), len(mesh.polygons)))

    for v in mesh.vertices:
        h.update(struct.pack("<3f", float(v.co.x), float(v.co.y), float(v.co.z)))
        rows = []
        for e in v.groups:
            if e.weight > 1e-8:
                rows.append((obj.vertex_groups[e.group].name, float(e.weight)))
        rows.sort()
        h.update(struct.pack("<I", len(rows)))
        for name, weight in rows:
            raw = name.encode("utf-8")
            h.update(struct.pack("<I", len(raw)))
            h.update(raw)
            h.update(struct.pack("<f", weight))

    for p in mesh.polygons:
        ids = tuple(int(x) for x in p.vertices)
        h.update(struct.pack("<3I", *ids))
        h.update(struct.pack("<I?", int(p.material_index), bool(p.use_smooth)))
        for u, v in poly_uvs(mesh, p, uv):
            h.update(struct.pack("<2f", u, v))
    return h.hexdigest()


def main(argv):
    a = parse_args(argv)
    if a.object != TARGET:
        raise RuntimeError("V2 gate currently supports only MshC01")

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    mesh = obj.data
    mesh.update()
    nv = len(mesh.vertices)
    nf = len(mesh.polygons)

    if nv < 3 or nf < 1:
        raise RuntimeError("empty/invalid mesh")
    if nv > 30000 or nf > 30000:
        raise RuntimeError(
            f"conservative runtime gate exceeded: vertices={nv}, faces={nf}"
        )
    if len(mesh.uv_layers) != 1 or mesh.uv_layers.active is None:
        raise RuntimeError("requires exactly one active UV layer")
    for p in mesh.polygons:
        if len(p.vertices) != 3:
            raise RuntimeError(f"polygon {p.index}: triangulate first")

    old_bones = [rigid_bone(obj, vi) for vi in range(nv)]
    uv_layer = mesh.uv_layers.active

    vshift = a.vertex_shift % nv
    fshift = a.face_shift % nf
    if vshift == 0:
        raise RuntimeError("vertex shift is zero modulo vertex count")
    if fshift == 0:
        raise RuntimeError("face shift is zero modulo face count")

    new_to_old_v = [(j + vshift) % nv for j in range(nv)]
    old_to_new_v = [0] * nv
    for new_i, old_i in enumerate(new_to_old_v):
        old_to_new_v[old_i] = new_i

    new_to_old_f = [(j + fshift) % nf for j in range(nf)]
    old_to_new_f = [0] * nf
    for new_i, old_i in enumerate(new_to_old_f):
        old_to_new_f[old_i] = new_i

    fixed_v = [i for i in range(nv) if old_to_new_v[i] == i]
    fixed_f = [i for i in range(nf) if old_to_new_f[i] == i]
    if fixed_v or fixed_f:
        raise RuntimeError(
            f"unexpected fixed points: vertices={fixed_v}, faces={fixed_f}"
        )

    old_vertices = [
        (float(v.co.x), float(v.co.y), float(v.co.z))
        for v in mesh.vertices
    ]
    old_faces = [tuple(int(x) for x in p.vertices) for p in mesh.polygons]
    old_materials = [int(p.material_index) for p in mesh.polygons]
    old_smooth = [bool(p.use_smooth) for p in mesh.polygons]
    old_uvs = [poly_uvs(mesh, p, uv_layer) for p in mesh.polygons]
    group_names, memberships = capture_groups(obj)

    new_vertices = [old_vertices[old_i] for old_i in new_to_old_v]
    new_faces = []
    new_materials = []
    new_smooth = []
    new_uvs = []

    for old_fi in new_to_old_f:
        old_face = old_faces[old_fi]
        new_faces.append(tuple(old_to_new_v[v] for v in old_face))
        new_materials.append(old_materials[old_fi])
        new_smooth.append(old_smooth[old_fi])
        new_uvs.append(old_uvs[old_fi])

    new_mesh = bpy.data.meshes.new(mesh.name + "_FullReindexGateV2")
    for mat in mesh.materials:
        new_mesh.materials.append(mat)
    new_mesh.from_pydata(new_vertices, [], new_faces)
    new_mesh.update(calc_edges=True)
    new_uv = new_mesh.uv_layers.new(name=uv_layer.name)

    for pi, p in enumerate(new_mesh.polygons):
        p.material_index = new_materials[pi]
        p.use_smooth = new_smooth[pi]
        for li, uvco in zip(p.loop_indices, new_uvs[pi]):
            new_uv.data[li].uv = uvco

    obj.data = new_mesh
    restore_groups(obj, group_names, memberships, new_to_old_v)

    max_pos_error = 0.0
    for new_i, old_i in enumerate(new_to_old_v):
        co = obj.data.vertices[new_i].co
        ref = old_vertices[old_i]
        err = max(abs(float(co[k]) - ref[k]) for k in range(3))
        max_pos_error = max(max_pos_error, err)
        if rigid_bone(obj, new_i) != old_bones[old_i]:
            raise RuntimeError(f"vertex {new_i}: rigid bone changed")

    if max_pos_error != 0.0:
        raise RuntimeError(f"geometry changed while reindexing: {max_pos_error}")

    for new_fi, old_fi in enumerate(new_to_old_f):
        got = tuple(int(x) for x in obj.data.polygons[new_fi].vertices)
        expected = tuple(old_to_new_v[x] for x in old_faces[old_fi])
        if got != expected:
            raise RuntimeError(
                f"face {new_fi}: topology mismatch {got} != {expected}"
            )

    digest = state_digest(obj)

    obj["pt_free_mesh_gate"] = "FULL_REINDEX_ANY_COUNTS"
    obj["pt_free_mesh_gate_version"] = 2
    obj["pt_free_mesh_gate_vertices"] = nv
    obj["pt_free_mesh_gate_faces"] = nf
    obj["pt_free_mesh_gate_vertex_shift"] = vshift
    obj["pt_free_mesh_gate_face_shift"] = fshift
    obj["pt_free_mesh_gate_state_sha256"] = digest

    out = Path(a.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    if state_digest(obj) != digest:
        raise RuntimeError("mesh state changed during save")

    print("PT_HIGH_MESH_FULL_REINDEX_GATE_V2")
    print("object=MshC01")
    print("operation=FULL_REINDEX_ANY_COUNTS")
    print(f"vertices={nv}")
    print(f"faces={nf}")
    print(f"vertex_shift={vshift}")
    print(f"face_shift={fshift}")
    print(f"fixed_vertex_indices={fixed_v}")
    print(f"fixed_face_indices={fixed_f}")
    print(f"reindexed_vertices={nv}")
    print(f"reordered_faces={nf}")
    print(f"max_position_error={max_pos_error:.12g}")
    print(f"rigid_vertices={nv}")
    print(f"mesh_state_sha256={digest}")
    print(f"blend={out}")
    print("full_reindex_gate_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"full_reindex_gate_check=FAIL: {type(exc).__name__}: {exc}")
        raise

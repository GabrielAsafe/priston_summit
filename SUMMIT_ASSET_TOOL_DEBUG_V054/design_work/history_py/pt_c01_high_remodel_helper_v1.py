#!/usr/bin/env python3
"""
Priestess C01 HIGH remodel helper V1.

Input:
  MfbC01_high_smooth_patch_v1.blend
  MshC01 = 197 vertices / 367 faces

Feature under test:
  move ORIGINAL HIGH vertices while retaining/re-fitting the already proven
  appended smooth topology.

Method:
  1. Read the original 331-face MshC01 topology from pristine MfbC01.smd.
  2. Read the selected source-face patch stored by high_smooth_patch_v1.
  3. Laplacian-relax original vertices used by that patch.
     - interior patch vertices receive full amount
     - patch boundary vertices receive reduced amount
     - per-step movement is capped
  4. Recompute all 24 appended edge vertices from the moved old vertices using
     the same PN-edge rule used by high_smooth_patch_v1.
  5. Preserve faces, UVs, materials, vertex groups and all non-target vertices.

This modifies only a NEW .blend. It does not touch SMD/client files.
"""

import argparse
import hashlib
import math
import struct
import sys
from collections import defaultdict
from pathlib import Path

import bpy
from mathutils import Vector


SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
TARGET = "MshC01"
SOURCE_V = 173
SOURCE_F = 331
SMOOTH_V = 197
SMOOTH_F = 367

SZ_HDR = 556
SZ_INFO = 40
SZ_OBJ = 2236
SZ_VERT = 24
SZ_FACE = 36
O_NV = 84
O_NF = 88
O_NAME = 172


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default=TARGET)
    p.add_argument("--source-smd", default="MfbC01.smd")
    p.add_argument("--amount", type=float, default=0.32)
    p.add_argument("--boundary-factor", type=float, default=0.45)
    p.add_argument("--iterations", type=int, default=2)
    p.add_argument("--max-step", type=float, default=0.18)
    p.add_argument("--out", default="blender/MfbC01_high_remodel_v1.blend")
    return p.parse_args(argv)


def i32(buf, off):
    return struct.unpack_from("<i", buf, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def source_faces_from_smd(data, object_name):
    if len(data) < SZ_HDR or not data.startswith(b"SMD Model data Ver 0.62"):
        raise RuntimeError("unexpected source SMD header")

    nobj = i32(data, 24)
    target = None
    for i in range(nobj):
        off = SZ_HDR + i * SZ_INFO
        raw = data[off:off + SZ_INFO]
        name = cstr(raw[:32])
        if name == object_name:
            target = (i32(raw, 36), i32(raw, 32))
            break
    if target is None:
        raise RuntimeError(f"{object_name} not found in source SMD")

    start, length = target
    obj = data[start:start + SZ_OBJ]
    if cstr(obj[O_NAME:O_NAME + 32]) != object_name:
        raise RuntimeError("source object header/name mismatch")

    nv = i32(obj, O_NV)
    nf = i32(obj, O_NF)
    if (nv, nf) != (SOURCE_V, SOURCE_F):
        raise RuntimeError(f"unexpected source MshC01 counts {nv}/{nf}")

    face_start = start + SZ_OBJ + nv * SZ_VERT
    faces = []
    for fi in range(nf):
        off = face_start + fi * SZ_FACE
        a, b, c = struct.unpack_from("<3H", data, off)
        faces.append((int(a), int(b), int(c)))
    return faces


def edge(a, b):
    return (a, b) if a < b else (b, a)


def build_source_topology(faces):
    neighbors = [set() for _ in range(SOURCE_V)]
    edge_faces = defaultdict(list)

    for fi, tri in enumerate(faces):
        a, b, c = tri
        for u, v in ((a, b), (b, c), (c, a)):
            neighbors[u].add(v)
            neighbors[v].add(u)
            edge_faces[edge(u, v)].append(fi)

    return neighbors, edge_faces


def patch_edges_from_faces(faces, selected):
    edges = set()
    vertices = set()
    for fi in selected:
        if not 0 <= fi < len(faces):
            raise RuntimeError(f"selected source face {fi} out of range")
        tri = faces[fi]
        vertices.update(tri)
        a, b, c = tri
        edges.update((edge(a, b), edge(b, c), edge(c, a)))
    return edges, vertices


def patch_boundary(edges, edge_faces, selected_set):
    internal = set()
    boundary = set()

    for e in edges:
        incident = edge_faces[e]
        if len(incident) == 2 and all(fi in selected_set for fi in incident):
            internal.add(e)
        else:
            boundary.add(e)

    boundary_vertices = {v for e in boundary for v in e}
    return internal, boundary, boundary_vertices


def geometric_normals(positions, faces):
    acc = [Vector((0.0, 0.0, 0.0)) for _ in range(SOURCE_V)]
    counts = [0] * SOURCE_V

    for fi, (a, b, c) in enumerate(faces):
        pa, pb, pc = positions[a], positions[b], positions[c]
        n = (pb - pa).cross(pc - pa)
        if n.length <= 1e-12:
            raise RuntimeError(f"source face {fi} became degenerate")
        n.normalize()
        for vi in (a, b, c):
            acc[vi] += n
            counts[vi] += 1

    result = []
    for vi, n in enumerate(acc):
        if counts[vi] == 0 or n.length <= 1e-12:
            raise RuntimeError(f"source vertex {vi}: invalid geometric normal")
        n.normalize()
        result.append(n)
    return result


def pn_edge_midpoint(p0, p1, n0, n1, strength):
    linear = (p0 + p1) * 0.5
    w01 = (p1 - p0).dot(n0)
    w10 = (p0 - p1).dot(n1)
    b1 = (2.0 * p0 + p1 - w01 * n0) / 3.0
    b2 = (2.0 * p1 + p0 - w10 * n1) / 3.0
    curved = (p0 + 3.0 * b1 + 3.0 * b2 + p1) / 8.0
    return linear + (curved - linear) * float(strength)


def main(argv):
    args = parse_args(argv)

    if args.object != TARGET:
        raise RuntimeError("remodel helper V1 supports only MshC01")
    if not (0.0 < args.amount <= 1.0):
        raise RuntimeError("--amount must be in (0,1]")
    if not (0.0 <= args.boundary_factor <= 1.0):
        raise RuntimeError("--boundary-factor must be in [0,1]")
    if not (1 <= args.iterations <= 10):
        raise RuntimeError("--iterations must be 1..10")
    if not (0.0 < args.max_step <= 1.0):
        raise RuntimeError("--max-step must be in (0,1]")

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")
    mesh = obj.data
    mesh.update()

    if (len(mesh.vertices), len(mesh.polygons)) != (SMOOTH_V, SMOOTH_F):
        raise RuntimeError(
            f"requires proven smooth blend MshC01 {SMOOTH_V}/{SMOOTH_F}; "
            f"got {len(mesh.vertices)}/{len(mesh.polygons)}"
        )

    if obj.get("pt_topology_operation") != "HIGH_PN_EDGE_SUBDIVIDE_1_TO_4":
        raise RuntimeError("input blend lacks proven HIGH smooth-patch metadata")

    src_path = Path(args.source_smd).expanduser().resolve()
    if not src_path.is_file():
        raise RuntimeError(f"source SMD missing: {src_path}")
    src_data = src_path.read_bytes()
    src_sha = hashlib.sha256(src_data).hexdigest()
    if src_sha != SOURCE_SHA:
        raise RuntimeError(f"unexpected pristine source SHA256: {src_sha}")

    meta_sha = str(obj.get("pt_topology_source_smd_sha256", ""))
    if meta_sha != SOURCE_SHA:
        raise RuntimeError("smooth blend source SHA metadata mismatch")

    selected_raw = str(obj.get("pt_topology_patch_faces", ""))
    selected = [int(x) for x in selected_raw.split(",") if x.strip()]
    if not selected:
        raise RuntimeError("smooth blend has no selected source-face metadata")
    selected_set = set(selected)

    bone = str(obj.get("pt_topology_bone", ""))
    if not bone:
        raise RuntimeError("smooth blend missing topology bone metadata")
    pn_strength = float(obj.get("pt_topology_strength", 1.0))

    source_faces = source_faces_from_smd(src_data, TARGET)
    neighbors, edge_faces = build_source_topology(source_faces)
    patch_edges, patch_vertices = patch_edges_from_faces(source_faces, selected)
    internal_edges, boundary_edges, boundary_vertices = patch_boundary(
        patch_edges, edge_faces, selected_set
    )

    # Deterministic smooth-helper mapping:
    # appended vertex 173+j corresponds to sorted(patch_edges)[j].
    sorted_edges = sorted(patch_edges)
    expected_new = len(sorted_edges)
    if SOURCE_V + expected_new != SMOOTH_V:
        raise RuntimeError(
            f"edge/new-vertex mapping mismatch: expected {SOURCE_V + expected_new} vertices"
        )

    old_start = [Vector(mesh.vertices[i].co) for i in range(SOURCE_V)]
    pos = [v.copy() for v in old_start]

    moved_set = set(patch_vertices)
    iteration_max_steps = []

    for _ in range(args.iterations):
        nxt = [p.copy() for p in pos]
        step_max = 0.0

        for vi in sorted(moved_set):
            ns = sorted(neighbors[vi])
            if not ns:
                continue

            avg = Vector((0.0, 0.0, 0.0))
            for n in ns:
                avg += pos[n]
            avg /= len(ns)

            delta = avg - pos[vi]
            factor = args.amount
            if vi in boundary_vertices:
                factor *= args.boundary_factor

            step = delta * factor
            if step.length > args.max_step:
                step.normalize()
                step *= args.max_step

            nxt[vi] = pos[vi] + step
            step_max = max(step_max, float(step.length))

        pos = nxt
        iteration_max_steps.append(step_max)

    # Guard against excessive cumulative deformation.
    old_displacements = {}
    for vi in sorted(moved_set):
        d = (pos[vi] - old_start[vi]).length
        old_displacements[vi] = float(d)
        if d > 0.50:
            raise RuntimeError(
                f"original vertex {vi}: cumulative remodel displacement {d:.6g} > 0.50"
            )

    # Recompute the 24 smooth vertices after the original vertices move.
    normals = geometric_normals(pos, source_faces)
    new_positions = {}
    new_displacements = {}

    for j, e in enumerate(sorted_edges):
        vi = SOURCE_V + j
        a, b = e
        pa, pb = pos[a], pos[b]
        linear = (pa + pb) * 0.5

        if e in internal_edges:
            newp = pn_edge_midpoint(pa, pb, normals[a], normals[b], pn_strength)
            mode = "PN_INTERNAL"
        else:
            newp = linear
            mode = "LINEAR_BOUNDARY"

        before = Vector(mesh.vertices[vi].co)
        new_positions[vi] = newp
        new_displacements[vi] = (
            mode,
            float((newp - before).length),
        )

    # Apply all positions only after every validation/computation succeeds.
    for vi in sorted(moved_set):
        mesh.vertices[vi].co = pos[vi]
    for vi, p in new_positions.items():
        mesh.vertices[vi].co = p
    mesh.update()

    # Non-patch original vertices must remain exact.
    for vi in range(SOURCE_V):
        if vi in moved_set:
            continue
        after = Vector(mesh.vertices[vi].co)
        if (after - old_start[vi]).length > 1e-12:
            raise RuntimeError(f"non-patch original vertex {vi} changed")

    # Faces/topology must remain exactly the proven smooth topology.
    if (len(mesh.vertices), len(mesh.polygons)) != (SMOOTH_V, SMOOTH_F):
        raise RuntimeError("topology changed unexpectedly during remodel")

    old_vals = list(old_displacements.values())
    new_vals = [x[1] for x in new_displacements.values()]

    obj["pt_remodel_operation"] = "HIGH_PATCH_LAPLACIAN_RELAX"
    obj["pt_remodel_version"] = 1
    obj["pt_remodel_amount"] = float(args.amount)
    obj["pt_remodel_boundary_factor"] = float(args.boundary_factor)
    obj["pt_remodel_iterations"] = int(args.iterations)
    obj["pt_remodel_max_step"] = float(args.max_step)
    obj["pt_remodel_old_vertex_count"] = len(moved_set)

    out = Path(args.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    print("PT_C01_HIGH_REMODEL_HELPER_V1")
    print("object=MshC01")
    print("operation=HIGH_PATCH_LAPLACIAN_RELAX")
    print(f"source_vertices={SOURCE_V}")
    print(f"smooth_vertices={SMOOTH_V}")
    print(f"smooth_faces={SMOOTH_F}")
    print(f"selected_source_faces={selected}")
    print(f"patch_original_vertices={sorted(moved_set)}")
    print(f"patch_original_vertex_count={len(moved_set)}")
    print(f"patch_edges={len(patch_edges)}")
    print(f"internal_edges={len(internal_edges)}")
    print(f"boundary_edges={len(boundary_edges)}")
    print(f"amount={args.amount:.9g}")
    print(f"boundary_factor={args.boundary_factor:.9g}")
    print(f"iterations={args.iterations}")
    print(f"max_step={args.max_step:.9g}")
    print(f"iteration_max_steps={iteration_max_steps}")
    print(f"old_displacement_min={(min(old_vals) if old_vals else 0.0):.9g}")
    print(f"old_displacement_mean={(sum(old_vals)/len(old_vals) if old_vals else 0.0):.9g}")
    print(f"old_displacement_max={(max(old_vals) if old_vals else 0.0):.9g}")
    print(f"refit_new_vertex_count={len(new_positions)}")
    print(f"new_vertex_refit_min={(min(new_vals) if new_vals else 0.0):.9g}")
    print(f"new_vertex_refit_mean={(sum(new_vals)/len(new_vals) if new_vals else 0.0):.9g}")
    print(f"new_vertex_refit_max={(max(new_vals) if new_vals else 0.0):.9g}")
    print(f"source_smd_sha256={src_sha}")
    print(f"blend={out}")
    print("high_remodel_helper_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"high_remodel_helper_check=FAIL: {type(exc).__name__}: {exc}")
        raise

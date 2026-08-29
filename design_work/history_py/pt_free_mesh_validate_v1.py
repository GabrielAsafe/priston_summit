#!/usr/bin/env python3
"""
PT FREE MESH VALIDATOR V1
Validation-only gate for an artist-edited Summit Tale MshC01 Blender mesh.

No file is modified except the optional JSON report.
No SMD is written.
No client file is touched.

Checks:
  - object exists and is a mesh
  - applied/identity object transform
  - 3..30000 vertices
  - 1..30000 triangular faces
  - exactly one active UV layer
  - finite coordinates / UVs
  - no degenerate triangles
  - exactly one rigid vertex group at weight 1.0 per vertex
  - every used bone exists in supplied SMB
  - material IDs are supported by pristine MshC01 (V1)
  - vertex positions can be represented in bone-local fixed-point within tolerance

Reports but does NOT reject:
  - boundary edges
  - non-manifold edges
  - edge-degree histogram

This intentionally does not reject non-manifold topology because the original
MshC01 itself contains valid legacy degree-3 edges.
"""

import argparse
import hashlib
import json
import math
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

TARGET = "MshC01"
SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"

SZ_HDR = 556
SZ_INFO = 40
SZ_OBJ = 2236
SZ_VERT = 24
SZ_FACE = 36
SZ_TEX = 32
SZ_ROT = 20
SZ_POS = 16
SZ_SCALE = 16
SZ_MATRIX = 64

O_PHYS_PTR = 16
O_NV = 84
O_NF = 88
O_NT = 92
O_NAME = 172
O_TM = 240
O_NR = 684
O_NP = 688
O_NS = 692

F_TEX_PTR = 32

MAX_VERTICES = 30000
MAX_FACES = 30000


def i32(b, o):
    return struct.unpack_from("<i", b, o)[0]


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def cstr(b):
    return b.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def sha(data):
    return hashlib.sha256(data).hexdigest()


def parse_file(data):
    if len(data) < SZ_HDR:
        raise RuntimeError("file too small")
    n = i32(data, 24)
    if not 1 <= n <= 128:
        raise RuntimeError(f"implausible object count {n}")

    infos = []
    for idx in range(n):
        off = SZ_HDR + idx * SZ_INFO
        raw = data[off:off + SZ_INFO]
        if len(raw) != SZ_INFO:
            raise RuntimeError("truncated object info")
        infos.append({
            "index": idx,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })
    return infos


def object_layout(start, nv, nf, nt, nr, np, ns, phys):
    v = start + SZ_OBJ
    f = v + nv * SZ_VERT
    t = f + nf * SZ_FACE
    r = t + nt * SZ_TEX
    p = r + nr * SZ_ROT
    s = p + np * SZ_POS
    prev = s + ns * SZ_SCALE
    phy = prev + nr * SZ_MATRIX if phys else None
    end = phy + nv * 32 if phys else prev + nr * SZ_MATRIX
    return {
        "vertices": v, "faces": f, "tex": t,
        "rot": r, "pos": p, "scale": s,
        "prev": prev, "physique": phy, "end": end,
    }


def parse_mshc01_materials(data):
    infos = parse_file(data)
    inf = next((x for x in infos if x["name"] == TARGET), None)
    if inf is None:
        raise RuntimeError("MshC01 missing from pristine SMD")

    raw = data[inf["start"]:inf["start"] + SZ_OBJ]
    nv = i32(raw, O_NV)
    nf = i32(raw, O_NF)
    nt = i32(raw, O_NT)
    nr = i32(raw, O_NR)
    np = i32(raw, O_NP)
    ns = i32(raw, O_NS)
    phys = u32(raw, O_PHYS_PTR) != 0
    lay = object_layout(inf["start"], nv, nf, nt, nr, np, ns, phys)

    mats = set()
    for fi in range(nf):
        off = lay["faces"] + fi * SZ_FACE
        mats.add(struct.unpack_from("<H", data, off + 6)[0])
    return sorted(mats)


def parse_skeleton(data):
    infos = parse_file(data)
    bones = {}
    for inf in infos:
        raw = data[inf["start"]:inf["start"] + SZ_OBJ]
        name = cstr(raw[O_NAME:O_NAME + 32])
        if not name:
            raise RuntimeError("empty skeleton NodeName")
        if name in bones:
            raise RuntimeError(f"duplicate skeleton NodeName {name!r}")
        bones[name] = struct.unpack_from("<16i", raw, O_TM)
    return bones


def m(tm, r, c):
    return tm[(r - 1) * 4 + (c - 1)]


def world_fixed(local, tm):
    x, y, z = map(int, local)
    rx = x * m(tm, 1, 1) + y * m(tm, 2, 1) + z * m(tm, 3, 1)
    ry = x * m(tm, 1, 2) + y * m(tm, 2, 2) + z * m(tm, 3, 2)
    rz = x * m(tm, 1, 3) + y * m(tm, 2, 3) + z * m(tm, 3, 3)
    return (
        (rx >> 8) + m(tm, 4, 1),
        (ry >> 8) + m(tm, 4, 2),
        (rz >> 8) + m(tm, 4, 3),
    )


def inv_row3(vec, tm):
    a, b, c = m(tm,1,1)/256.0, m(tm,1,2)/256.0, m(tm,1,3)/256.0
    d, e, f = m(tm,2,1)/256.0, m(tm,2,2)/256.0, m(tm,2,3)/256.0
    g, h, i = m(tm,3,1)/256.0, m(tm,3,2)/256.0, m(tm,3,3)/256.0

    det = a*(e*i-f*h) - b*(d*i-f*g) + c*(d*h-e*g)
    if abs(det) < 1e-12:
        raise RuntimeError("singular bone matrix")

    inv = (
        ((e*i-f*h)/det, (c*h-b*i)/det, (b*f-c*e)/det),
        ((f*g-d*i)/det, (a*i-c*g)/det, (c*d-a*f)/det),
        ((d*h-e*g)/det, (b*g-a*h)/det, (a*e-b*d)/det),
    )

    x, y, z = vec
    return (
        x*inv[0][0] + y*inv[1][0] + z*inv[2][0],
        x*inv[0][1] + y*inv[1][1] + z*inv[2][1],
        x*inv[0][2] + y*inv[1][2] + z*inv[2][2],
    )


def inverse_point(world, tm):
    return inv_row3((
        world[0] - m(tm,4,1)/256.0,
        world[1] - m(tm,4,2)/256.0,
        world[2] - m(tm,4,3)/256.0,
    ), tm)


def fit_error(world, tm, radius=4):
    q = inverse_point(world, tm)
    center = tuple(int(round(v * 256.0)) for v in q)
    best = None

    for dx in range(-radius, radius + 1):
        for dy in range(-radius, radius + 1):
            for dz in range(-radius, radius + 1):
                cand = (center[0]+dx, center[1]+dy, center[2]+dz)
                wf = world_fixed(cand, tm)
                represented = tuple(v/256.0 for v in wf)
                err2 = sum((represented[k]-world[k])**2 for k in range(3))
                if best is None or err2 < best:
                    best = err2
    return math.sqrt(best)


def rigid_bone(obj, vi):
    rows = []
    for e in obj.data.vertices[vi].groups:
        if e.weight <= 1e-8:
            continue
        if not (0 <= e.group < len(obj.vertex_groups)):
            raise RuntimeError(f"vertex {vi}: invalid group index {e.group}")
        rows.append((obj.vertex_groups[e.group].name, float(e.weight)))

    if len(rows) != 1:
        raise RuntimeError(
            f"vertex {vi}: expected exactly one rigid group; got {rows}"
        )
    name, weight = rows[0]
    if abs(weight - 1.0) > 1e-5:
        raise RuntimeError(
            f"vertex {vi}: rigid group {name!r} has weight {weight}, expected 1.0"
        )
    return name


def object_identity(obj, eps=1e-7):
    ident = Matrix.Identity(4)
    for r in range(4):
        for c in range(4):
            if abs(float(obj.matrix_world[r][c] - ident[r][c])) > eps:
                return False
    return True


def finite3(v):
    return all(math.isfinite(float(x)) for x in v)


def edge_stats(mesh):
    edges = defaultdict(list)
    for p in mesh.polygons:
        a, b, c = [int(v) for v in p.vertices]
        for x, y in ((a,b),(b,c),(c,a)):
            e = (x,y) if x < y else (y,x)
            edges[e].append(int(p.index))

    hist = Counter(len(fs) for fs in edges.values())
    boundary = [e for e, fs in edges.items() if len(fs) == 1]
    nonmanifold = [
        {"edge": list(e), "faces": fs}
        for e, fs in edges.items() if len(fs) > 2
    ]
    return hist, boundary, nonmanifold


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", default="MfbC01.smd")
    p.add_argument("--skeleton", default="m5.smb")
    p.add_argument("--object", default=TARGET)
    p.add_argument("--max-fit-error", type=float, default=0.02)
    p.add_argument("--report", default="logs/MshC01_free_mesh_validate_v1.json")
    return p.parse_args(argv)


def main(argv):
    a = parse_args(argv)

    if a.object != TARGET:
        raise RuntimeError("validator V1 currently supports only MshC01")

    src_path = Path(a.smd).expanduser().resolve()
    src = src_path.read_bytes()
    if sha(src) != SOURCE_SHA:
        raise RuntimeError("source SMD is not the pristine approved MfbC01.smd")

    allowed_materials = parse_mshc01_materials(src)

    skel_path = Path(a.skeleton).expanduser().resolve()
    skeleton = parse_skeleton(skel_path.read_bytes())

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    if not object_identity(obj):
        raise RuntimeError(
            "MshC01 object transform is not identity. "
            "Apply Location/Rotation/Scale before export."
        )

    mesh = obj.data
    mesh.update()
    nv = len(mesh.vertices)
    nf = len(mesh.polygons)

    if not 3 <= nv <= MAX_VERTICES:
        raise RuntimeError(f"vertex count {nv} outside 3..{MAX_VERTICES}")
    if not 1 <= nf <= MAX_FACES:
        raise RuntimeError(f"face count {nf} outside 1..{MAX_FACES}")
    if nv > 65535:
        raise RuntimeError("vertex count exceeds uint16 index format")

    if len(mesh.uv_layers) != 1 or mesh.uv_layers.active is None:
        raise RuntimeError("requires exactly one active UV layer")

    uv_layer = mesh.uv_layers.active
    degenerate_faces = []
    used_materials = Counter()
    uv_nonfinite = []

    for p in mesh.polygons:
        if len(p.vertices) != 3:
            raise RuntimeError(f"polygon {p.index}: mesh must be triangulated")

        ids = [int(v) for v in p.vertices]
        if len(set(ids)) != 3:
            degenerate_faces.append(int(p.index))
            continue

        a0, b0, c0 = [Vector(mesh.vertices[v].co) for v in ids]
        area2 = (b0-a0).cross(c0-a0).length
        if area2 <= 1e-10:
            degenerate_faces.append(int(p.index))

        mat = int(p.material_index)
        used_materials[mat] += 1
        if mat not in allowed_materials:
            raise RuntimeError(
                f"polygon {p.index}: material {mat} unsupported by V1; "
                f"allowed={allowed_materials}"
            )

        for li in p.loop_indices:
            uv = uv_layer.data[li].uv
            if not finite3((uv.x, uv.y, 0.0)):
                uv_nonfinite.append((int(p.index), int(li)))

    if degenerate_faces:
        raise RuntimeError(
            f"degenerate faces found: count={len(degenerate_faces)} "
            f"sample={degenerate_faces[:20]}"
        )
    if uv_nonfinite:
        raise RuntimeError(
            f"non-finite UVs found: count={len(uv_nonfinite)} "
            f"sample={uv_nonfinite[:20]}"
        )

    nonfinite_vertices = []
    bone_hist = Counter()
    fit_errors = []

    for vi, v in enumerate(mesh.vertices):
        world = (float(v.co.x), float(v.co.y), float(v.co.z))
        if not finite3(world):
            nonfinite_vertices.append(vi)
            continue

        bone = rigid_bone(obj, vi)
        if bone not in skeleton:
            raise RuntimeError(
                f"vertex {vi}: bone {bone!r} not found in skeleton"
            )
        bone_hist[bone] += 1
        err = fit_error(world, skeleton[bone])
        fit_errors.append(err)
        if err > a.max_fit_error:
            raise RuntimeError(
                f"vertex {vi}: fixed-point bind fit error {err} exceeds "
                f"{a.max_fit_error}"
            )

    if nonfinite_vertices:
        raise RuntimeError(
            f"non-finite vertex coordinates: {nonfinite_vertices[:20]}"
        )

    hist, boundary_edges, nonmanifold_edges = edge_stats(mesh)

    bbox = {
        "x": [min(float(v.co.x) for v in mesh.vertices),
              max(float(v.co.x) for v in mesh.vertices)],
        "y": [min(float(v.co.y) for v in mesh.vertices),
              max(float(v.co.y) for v in mesh.vertices)],
        "z": [min(float(v.co.z) for v in mesh.vertices),
              max(float(v.co.z) for v in mesh.vertices)],
    }

    report = {
        "tool": "PT_FREE_MESH_VALIDATE_V1",
        "blend": bpy.data.filepath,
        "object": TARGET,
        "vertices": nv,
        "faces": nf,
        "uv_layers": len(mesh.uv_layers),
        "used_materials": dict(used_materials),
        "allowed_materials": allowed_materials,
        "rigid_bone_count": len(bone_hist),
        "rigid_bone_histogram": dict(bone_hist),
        "max_vertex_fit_error": max(fit_errors) if fit_errors else 0.0,
        "mean_vertex_fit_error": (
            sum(fit_errors)/len(fit_errors) if fit_errors else 0.0
        ),
        "bbox": bbox,
        "edge_degree_histogram": dict(hist),
        "boundary_edge_count": len(boundary_edges),
        "nonmanifold_edge_count": len(nonmanifold_edges),
        "nonmanifold_edges": nonmanifold_edges,
        "validation_check": "PASS",
    }

    rp = Path(a.report).expanduser().resolve()
    rp.parent.mkdir(parents=True, exist_ok=True)
    rp.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print("PT_FREE_MESH_VALIDATE_V1")
    print(f"object={TARGET}")
    print(f"vertices={nv}")
    print(f"faces={nf}")
    print(f"uv_layers={len(mesh.uv_layers)}")
    print(f"used_materials={dict(used_materials)}")
    print(f"allowed_materials={allowed_materials}")
    print(f"rigid_bone_count={len(bone_hist)}")
    print(f"max_vertex_fit_error={(max(fit_errors) if fit_errors else 0.0):.12g}")
    print(f"mean_vertex_fit_error={(sum(fit_errors)/len(fit_errors) if fit_errors else 0.0):.12g}")
    print(f"edge_degree_histogram={dict(hist)}")
    print(f"boundary_edge_count={len(boundary_edges)}")
    print(f"nonmanifold_edge_count={len(nonmanifold_edges)}")
    print(f"report={rp}")
    print("free_mesh_validation_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"free_mesh_validation_check=FAIL: {type(exc).__name__}: {exc}")
        raise

#!/usr/bin/env python3
"""
Priestess C01 HIGH anatomy probe V1.

Purpose:
  map the proven MshC01 HIGH torso/chest patch before the next remodel feature.

This probe DOES NOT modify or save the Blender file.

It validates pristine MshC01 topology against MfbC01.smd, then expands outward
from the already runtime-proven chest/torso seed faces:

  [154, 152, 328, 165, 325, 329, 162, 166, 183, 324, 323, 330]

For each topological face ring it reports:
  - face/vertex counts
  - XYZ bounding box
  - vertex Physique bone histogram
  - common-rigid-bone face histogram
  - mixed-bone face count
  - material histogram
  - boundary edge count

The output will be used to define an anatomical "upper_chest" region without
guessing face indices.
"""

import argparse
import hashlib
import json
import struct
import sys
from collections import Counter, defaultdict, deque
from pathlib import Path

import bpy


SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
TARGET = "MshC01"
SOURCE_V = 173
SOURCE_F = 331
PROVEN_SEED = [154, 152, 328, 165, 325, 329, 162, 166, 183, 324, 323, 330]

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
    p.add_argument("--rings", type=int, default=3)
    p.add_argument(
        "--seed-faces",
        default=",".join(str(x) for x in PROVEN_SEED),
        help="Comma-separated original MshC01 face indices."
    )
    p.add_argument("--report", default="logs/MshC01_high_anatomy_probe_v1.json")
    return p.parse_args(argv)


def i32(buf, off):
    return struct.unpack_from("<i", buf, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def parse_source_faces(data, name):
    if len(data) < SZ_HDR or not data.startswith(b"SMD Model data Ver 0.62"):
        raise RuntimeError("unexpected source SMD header")

    nobj = i32(data, 24)
    start = length = None
    for idx in range(nobj):
        off = SZ_HDR + idx * SZ_INFO
        raw = data[off:off + SZ_INFO]
        if cstr(raw[:32]) == name:
            length = i32(raw, 32)
            start = i32(raw, 36)
            break
    if start is None:
        raise RuntimeError(f"{name!r} not found in source SMD")

    obj = data[start:start + SZ_OBJ]
    if cstr(obj[O_NAME:O_NAME + 32]) != name:
        raise RuntimeError("source object NodeName mismatch")

    nv = i32(obj, O_NV)
    nf = i32(obj, O_NF)
    if (nv, nf) != (SOURCE_V, SOURCE_F):
        raise RuntimeError(f"unexpected source {name} counts {nv}/{nf}")

    face_start = start + SZ_OBJ + nv * SZ_VERT
    faces = []
    materials = []
    for fi in range(nf):
        off = face_start + fi * SZ_FACE
        a, b, c, mat = struct.unpack_from("<4H", data, off)
        faces.append((int(a), int(b), int(c)))
        materials.append(int(mat))
    return faces, materials


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
            f"vertex {vi}: expected exactly one rigid group weight 1.0; got {rows}"
        )
    return rows[0][0]


def edge(a, b):
    return (a, b) if a < b else (b, a)


def topology(faces):
    edge_faces = defaultdict(list)
    face_adj = [set() for _ in faces]

    for fi, (a, b, c) in enumerate(faces):
        for u, v in ((a, b), (b, c), (c, a)):
            edge_faces[edge(u, v)].append(fi)

    # Legacy MshC01 is not strictly manifold: some edges may be shared by
    # 3+ faces. For anatomy traversal this is valid connectivity, not corruption.
    # Connect every pair of faces incident to the same edge.
    for e, fs in edge_faces.items():
        if len(fs) >= 2:
            for i in range(len(fs)):
                for j in range(i + 1, len(fs)):
                    a, b = fs[i], fs[j]
                    face_adj[a].add(b)
                    face_adj[b].add(a)

    return edge_faces, face_adj


def bbox_for_vertices(mesh, vertices):
    if not vertices:
        return None
    coords = [tuple(float(x) for x in mesh.vertices[vi].co) for vi in vertices]
    return {
        "x": [min(p[0] for p in coords), max(p[0] for p in coords)],
        "y": [min(p[1] for p in coords), max(p[1] for p in coords)],
        "z": [min(p[2] for p in coords), max(p[2] for p in coords)],
    }


def rounded_bbox(box):
    if box is None:
        return None
    return {k: [round(v[0], 6), round(v[1], 6)] for k, v in box.items()}


def face_set_stats(mesh, face_ids, source_faces, source_materials, bones, edge_faces):
    face_ids = sorted(set(face_ids))
    fset = set(face_ids)
    vertices = sorted({v for fi in face_ids for v in source_faces[fi]})

    vertex_bones = Counter(bones[v] for v in vertices)
    face_vertex_bones = Counter()
    common_face_bones = Counter()
    mixed_faces = []

    for fi in face_ids:
        bs = tuple(bones[v] for v in source_faces[fi])
        for b in bs:
            face_vertex_bones[b] += 1
        if bs[0] == bs[1] == bs[2]:
            common_face_bones[bs[0]] += 1
        else:
            mixed_faces.append({"face": fi, "bones": list(bs)})

    material_hist = Counter(source_materials[fi] for fi in face_ids)

    boundary_edges = []
    internal_edges = []
    nonmanifold_edges = []
    for e, fs in edge_faces.items():
        inside = [fi for fi in fs if fi in fset]
        if not inside:
            continue
        if len(fs) > 2:
            nonmanifold_edges.append({
                "edge": list(e),
                "incident_faces": list(fs),
                "inside_faces": inside,
            })

        # For arbitrary edge degree, an edge is internal to the selected region
        # only when every incident face belongs to the region.
        if len(fs) >= 2 and len(inside) == len(fs):
            internal_edges.append(e)
        else:
            boundary_edges.append(e)

    return {
        "faces": face_ids,
        "face_count": len(face_ids),
        "vertices": vertices,
        "vertex_count": len(vertices),
        "bbox": rounded_bbox(bbox_for_vertices(mesh, vertices)),
        "vertex_bone_hist": dict(vertex_bones.most_common()),
        "face_corner_bone_hist": dict(face_vertex_bones.most_common()),
        "common_face_bone_hist": dict(common_face_bones.most_common()),
        "mixed_bone_face_count": len(mixed_faces),
        "mixed_bone_faces": mixed_faces,
        "material_hist": {str(k): v for k, v in sorted(material_hist.items())},
        "internal_edge_count": len(internal_edges),
        "boundary_edge_count": len(boundary_edges),
        "nonmanifold_edge_count": len(nonmanifold_edges),
        "nonmanifold_edges": nonmanifold_edges,
    }


def make_rings(seed, adjacency, ring_count):
    cumulative = set(seed)
    frontier = set(seed)
    rings = [sorted(frontier)]

    for _ in range(ring_count):
        nxt = set()
        for fi in frontier:
            nxt.update(adjacency[fi])
        nxt -= cumulative
        rings.append(sorted(nxt))
        cumulative |= nxt
        frontier = nxt
        if not frontier:
            break

    return rings


def main(argv):
    args = parse_args(argv)

    if args.object != TARGET:
        raise RuntimeError("anatomy probe V1 supports only MshC01")
    if not (0 <= args.rings <= 8):
        raise RuntimeError("--rings must be between 0 and 8")

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    mesh = obj.data
    mesh.update()

    if (len(mesh.vertices), len(mesh.polygons)) != (SOURCE_V, SOURCE_F):
        raise RuntimeError(
            f"probe requires pristine MshC01 {SOURCE_V}/{SOURCE_F}; "
            f"got {len(mesh.vertices)}/{len(mesh.polygons)}"
        )

    source_path = Path(args.source_smd).expanduser().resolve()
    if not source_path.is_file():
        raise RuntimeError(f"source SMD missing: {source_path}")
    source_data = source_path.read_bytes()
    source_sha = hashlib.sha256(source_data).hexdigest()
    if source_sha != SOURCE_SHA:
        raise RuntimeError(f"unexpected source SMD SHA256: {source_sha}")

    source_faces, source_materials = parse_source_faces(source_data, TARGET)

    # Strong pristine topology check.
    blender_faces = [tuple(int(v) for v in p.vertices) for p in mesh.polygons]
    if blender_faces != source_faces:
        for fi, (a, b) in enumerate(zip(blender_faces, source_faces)):
            if a != b:
                raise RuntimeError(
                    f"Blender/source topology mismatch at face {fi}: {a} != {b}"
                )
        raise RuntimeError("Blender/source topology mismatch")

    bones = [rigid_bone(obj, vi) for vi in range(SOURCE_V)]
    edge_faces, adjacency = topology(source_faces)

    seed = [int(x.strip()) for x in args.seed_faces.split(",") if x.strip()]
    if not seed:
        raise RuntimeError("empty seed face list")
    if len(set(seed)) != len(seed):
        raise RuntimeError("duplicate seed face indices")
    if any(fi < 0 or fi >= SOURCE_F for fi in seed):
        raise RuntimeError("seed face index outside source range")

    # Ensure the seed itself is edge-connected.
    remaining = set(seed)
    q = deque([seed[0]])
    seen = {seed[0]}
    while q:
        fi = q.popleft()
        for n in adjacency[fi]:
            if n in remaining and n not in seen:
                seen.add(n)
                q.append(n)
    if seen != set(seed):
        raise RuntimeError(
            f"seed faces are not one edge-connected component; "
            f"connected={len(seen)}/{len(seed)}"
        )

    rings = make_rings(seed, adjacency, args.rings)

    report = {
        "tool": "PT_C01_HIGH_ANATOMY_PROBE_V1_1",
        "object": TARGET,
        "source_smd": str(source_path),
        "source_sha256": source_sha,
        "source_vertices": SOURCE_V,
        "source_faces": SOURCE_F,
        "seed_faces": seed,
        "rings_requested": args.rings,
        "global_bone_vertex_hist": dict(Counter(bones).most_common()),
        "global_edge_degree_hist": dict(
            Counter(len(fs) for fs in edge_faces.values()).most_common()
        ),
        "global_nonmanifold_edges": [
            {"edge": list(e), "incident_faces": list(fs)}
            for e, fs in sorted(edge_faces.items())
            if len(fs) > 2
        ],
        "ring_stats": [],
        "cumulative_stats": [],
        "anatomy_probe_check": "PASS",
    }

    cumulative = set()
    for ridx, ring in enumerate(rings):
        cumulative.update(ring)
        report["ring_stats"].append(
            {"ring": ridx, **face_set_stats(
                mesh, ring, source_faces, source_materials, bones, edge_faces
            )}
        )
        report["cumulative_stats"].append(
            {"through_ring": ridx, **face_set_stats(
                mesh, cumulative, source_faces, source_materials, bones, edge_faces
            )}
        )

    report_path = Path(args.report).expanduser().resolve()
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print("PT_C01_HIGH_ANATOMY_PROBE_V1_1")
    print("object=MshC01")
    print(f"source_vertices={SOURCE_V}")
    print(f"source_faces={SOURCE_F}")
    print(f"seed_faces={seed}")
    print(f"seed_face_count={len(seed)}")
    print(f"rings_requested={args.rings}")
    edge_degree_hist = dict(Counter(len(fs) for fs in edge_faces.values()).most_common())
    nonmanifold = [(e, fs) for e, fs in sorted(edge_faces.items()) if len(fs) > 2]
    print(f"global_bones={dict(Counter(bones).most_common())}")
    print(f"edge_degree_hist={edge_degree_hist}")
    print(f"nonmanifold_edge_count={len(nonmanifold)}")
    if nonmanifold:
        print(f"nonmanifold_edges={nonmanifold}")
    print()

    cumulative = set()
    for ridx, ring in enumerate(rings):
        cumulative.update(ring)
        rs = face_set_stats(
            mesh, ring, source_faces, source_materials, bones, edge_faces
        )
        cs = face_set_stats(
            mesh, cumulative, source_faces, source_materials, bones, edge_faces
        )

        print(f"RING {ridx}")
        print(f"  faces={rs['faces']}")
        print(f"  face_count={rs['face_count']}")
        print(f"  vertex_count={rs['vertex_count']}")
        print(f"  bbox={rs['bbox']}")
        print(f"  vertex_bones={rs['vertex_bone_hist']}")
        print(f"  common_face_bones={rs['common_face_bone_hist']}")
        print(f"  mixed_bone_faces={rs['mixed_bone_face_count']}")
        print(f"  boundary_edges={rs['boundary_edge_count']}")
        print(f"  nonmanifold_edges={rs['nonmanifold_edge_count']}")
        print(f"CUMULATIVE through ring {ridx}")
        print(f"  face_count={cs['face_count']}")
        print(f"  vertex_count={cs['vertex_count']}")
        print(f"  bbox={cs['bbox']}")
        print(f"  vertex_bones={cs['vertex_bone_hist']}")
        print(f"  common_face_bones={cs['common_face_bone_hist']}")
        print(f"  mixed_bone_faces={cs['mixed_bone_face_count']}")
        print(f"  boundary_edges={cs['boundary_edge_count']}")
        print(f"  nonmanifold_edges={cs['nonmanifold_edge_count']}")
        print()

    print(f"report={report_path}")
    print("anatomy_probe_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"anatomy_probe_check=FAIL: {type(exc).__name__}: {exc}")
        raise

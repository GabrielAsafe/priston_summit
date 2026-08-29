#!/usr/bin/env python3
"""
Priston Tale / Summit Tale - restricted Blender -> SMD vertex patcher V1.

Purpose:
  Patch only XYZ of existing vertices in one smOBJ3D, using the original SMD
  as a byte template. Intended as a conservative runtime gate for a mesh that
  was imported by pt_smd_blender_import_v3_static_armature.py and edited only
  in Blender Edit Mode.

Non-goals:
  - no topology changes
  - no face/material/UV/Physique changes
  - no normal recomputation
  - no armature/skeleton/animation serialization
  - not a universal SMD exporter

Run inside Blender, after opening the target .blend:
  blender file.blend --background --python this_script.py -- \\
    --smd MfbC01.smd --skeleton m5.smb --object MsmC01 --out out.smd
"""

import argparse
import hashlib
import json
import math
import struct
import sys
from pathlib import Path

import bpy

SMD_HEADER_TEXT = b"SMD Model data Ver 0.62"

SIZE_DFILE_HEADER = 556
SIZE_DFILE_OBJINFO = 40
SIZE_OBJ3D = 2236
SIZE_VERTEX = 24
SIZE_FACE = 36
SIZE_TEXLINK = 32
SIZE_TM_ROT = 20
SIZE_TM_POS = 16
SIZE_TM_SCALE = 16
SIZE_MATRIX = 64

O_OFF_TEXLINK_PTR = 12
O_OFF_PHYSIQUE_PTR = 16
O_OFF_NVERTEX = 84
O_OFF_NFACE = 88
O_OFF_NTEXLINK = 92
O_OFF_NODENAME = 172
O_OFF_NODEPARENT = 204
O_OFF_TM = 240
O_OFF_TMROTCNT = 684
O_OFF_TMPOSCNT = 688
O_OFF_TMSCALECNT = 692


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def parse_header_and_infos(data):
    if len(data) < SIZE_DFILE_HEADER:
        raise ValueError("file smaller than smDFILE_HEADER")
    header = data[:SIZE_DFILE_HEADER]
    if not header.startswith(SMD_HEADER_TEXT):
        raise ValueError(f"unexpected SMD header: {cstr(header[:24])!r}")

    obj_counter = i32(header, 24)
    mat_counter = i32(header, 28)
    mat_file_point = i32(header, 32)
    first_obj_point = i32(header, 36)

    if not (0 <= obj_counter <= 128):
        raise ValueError(f"implausible ObjCounter={obj_counter}")
    if not (0 <= mat_counter <= 4096):
        raise ValueError(f"implausible MatCounter={mat_counter}")
    if not (SIZE_DFILE_HEADER <= mat_file_point <= first_obj_point <= len(data)):
        raise ValueError("invalid material/object region offsets")

    table_end = SIZE_DFILE_HEADER + obj_counter * SIZE_DFILE_OBJINFO
    if table_end != mat_file_point:
        raise ValueError(
            f"object table end {table_end} != MatFilePoint {mat_file_point}"
        )

    infos = []
    for index in range(obj_counter):
        off = SIZE_DFILE_HEADER + index * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        infos.append({
            "index": index,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })

    return {
        "obj_counter": obj_counter,
        "mat_counter": mat_counter,
        "mat_file_point": mat_file_point,
        "first_obj_point": first_obj_point,
        "infos": infos,
    }


def object_layout(obj_start, nv, nf, nt, nr, np, ns, has_physique):
    vertices = obj_start + SIZE_OBJ3D
    faces = vertices + SIZE_VERTEX * nv
    texlinks = faces + SIZE_FACE * nf
    tmrot = texlinks + SIZE_TEXLINK * nt
    tmpos = tmrot + SIZE_TM_ROT * nr
    tmscale = tmpos + SIZE_TM_POS * np
    prevrot = tmscale + SIZE_TM_SCALE * ns
    physique = prevrot + SIZE_MATRIX * nr if has_physique else None
    end = physique + 32 * nv if has_physique else prevrot + SIZE_MATRIX * nr
    return {
        "vertices": vertices,
        "faces": faces,
        "texlinks": texlinks,
        "physique": physique,
        "end": end,
    }


def parse_object(data, object_name):
    meta = parse_header_and_infos(data)
    info = next((x for x in meta["infos"] if x["name"] == object_name), None)
    if info is None:
        raise ValueError(f"SMD object {object_name!r} not found")

    start = info["start"]
    raw = data[start:start + SIZE_OBJ3D]
    if len(raw) != SIZE_OBJ3D:
        raise ValueError(f"{object_name}: truncated smOBJ3D")

    node_name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
    if node_name != object_name:
        raise ValueError(
            f"object table name {object_name!r} != serialized NodeName {node_name!r}"
        )

    nv = i32(raw, O_OFF_NVERTEX)
    nf = i32(raw, O_OFF_NFACE)
    nt = i32(raw, O_OFF_NTEXLINK)
    nr = i32(raw, O_OFF_TMROTCNT)
    np = i32(raw, O_OFF_TMPOSCNT)
    ns = i32(raw, O_OFF_TMSCALECNT)
    has_physique = u32(raw, O_OFF_PHYSIQUE_PTR) != 0

    if min(nv, nf, nt, nr, np, ns) < 0:
        raise ValueError(f"{object_name}: negative serialized count")
    if not has_physique:
        raise ValueError(f"{object_name}: no serialized Physique; V1 refuses to patch")

    layout = object_layout(start, nv, nf, nt, nr, np, ns, has_physique)
    if layout["end"] != start + info["length"]:
        raise ValueError(
            f"{object_name}: serialized length mismatch: "
            f"computed={layout['end'] - start} table={info['length']}"
        )

    vertices = []
    for idx in range(nv):
        off = layout["vertices"] + idx * SIZE_VERTEX
        vertices.append(struct.unpack_from("<6i", data, off))

    bones = []
    p = layout["physique"]
    for idx in range(nv):
        bones.append(cstr(data[p + idx * 32:p + (idx + 1) * 32]))

    return {
        "meta": meta,
        "info": info,
        "layout": layout,
        "name": object_name,
        "vertex_count": nv,
        "face_count": nf,
        "texlink_count": nt,
        "vertices": vertices,
        "bones": bones,
    }


def parse_skeleton_tms(data):
    meta = parse_header_and_infos(data)
    if meta["mat_counter"] != 0:
        raise ValueError(
            f"unexpected skeleton MatCounter={meta['mat_counter']}; expected 0"
        )

    bones = {}
    for info in meta["infos"]:
        start = info["start"]
        raw = data[start:start + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise ValueError(f"skeleton object {info['index']}: truncated smOBJ3D")

        name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        if name != info["name"]:
            raise ValueError(
                f"skeleton table name {info['name']!r} != NodeName {name!r}"
            )
        if name in bones:
            raise ValueError(f"duplicate skeleton NodeName {name!r}")

        tm = struct.unpack_from("<16i", raw, O_OFF_TM)
        bones[name] = tm

    return bones


def mval(tm, row, col):
    return tm[(row - 1) * 4 + (col - 1)]


def transform_point_fixed(local_xyz, tm):
    """Exact integer convention used by the validated importer/ReformPhysique."""
    x, y, z = (int(local_xyz[0]), int(local_xyz[1]), int(local_xyz[2]))
    rx = x * mval(tm, 1, 1) + y * mval(tm, 2, 1) + z * mval(tm, 3, 1)
    ry = x * mval(tm, 1, 2) + y * mval(tm, 2, 2) + z * mval(tm, 3, 2)
    rz = x * mval(tm, 1, 3) + y * mval(tm, 2, 3) + z * mval(tm, 3, 3)
    return (
        (rx >> 8) + mval(tm, 4, 1),
        (ry >> 8) + mval(tm, 4, 2),
        (rz >> 8) + mval(tm, 4, 3),
    )


def inverse_3x3_row_vector(world_xyz, tm):
    """Float inverse of local(row) * R + T = world; returns local in Blender units."""
    # R rows, as used by transform_point_fixed.
    a, b, c = mval(tm, 1, 1) / 256.0, mval(tm, 1, 2) / 256.0, mval(tm, 1, 3) / 256.0
    d, e, f = mval(tm, 2, 1) / 256.0, mval(tm, 2, 2) / 256.0, mval(tm, 2, 3) / 256.0
    g, h, i = mval(tm, 3, 1) / 256.0, mval(tm, 3, 2) / 256.0, mval(tm, 3, 3) / 256.0

    det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g)
    if abs(det) < 1e-12:
        raise ValueError("singular bone 3x3 matrix")

    inv = (
        ((e * i - f * h) / det, (c * h - b * i) / det, (b * f - c * e) / det),
        ((f * g - d * i) / det, (a * i - c * g) / det, (c * d - a * f) / det),
        ((d * h - e * g) / det, (b * g - a * h) / det, (a * e - b * d) / det),
    )

    qx = world_xyz[0] - mval(tm, 4, 1) / 256.0
    qy = world_xyz[1] - mval(tm, 4, 2) / 256.0
    qz = world_xyz[2] - mval(tm, 4, 3) / 256.0

    # q(row) * inv(R)
    return (
        qx * inv[0][0] + qy * inv[1][0] + qz * inv[2][0],
        qx * inv[0][1] + qy * inv[1][1] + qz * inv[2][1],
        qx * inv[0][2] + qy * inv[1][2] + qz * inv[2][2],
    )


def choose_local_fixed(target_world, tm, original_local, radius):
    """
    Find nearby integer local XYZ whose exact fixed-point forward transform is
    closest to the edited Blender coordinate. Tie-break toward original local.
    """
    lf = inverse_3x3_row_vector(target_world, tm)
    center = tuple(int(round(v * 256.0)) for v in lf)

    best = None
    for dx in range(-radius, radius + 1):
        for dy in range(-radius, radius + 1):
            for dz in range(-radius, radius + 1):
                cand = (center[0] + dx, center[1] + dy, center[2] + dz)
                wf = transform_point_fixed(cand, tm)
                world = (wf[0] / 256.0, wf[1] / 256.0, wf[2] / 256.0)
                err2 = sum((world[k] - target_world[k]) ** 2 for k in range(3))
                delta2 = sum((cand[k] - original_local[k]) ** 2 for k in range(3))
                score = (err2, delta2, cand)
                if best is None or score < best[0]:
                    best = (score, cand, world)

    err = math.sqrt(best[0][0])
    return best[1], best[2], err


def matrix_identity_error(matrix_world):
    mx = 0.0
    for r in range(4):
        for c in range(4):
            expected = 1.0 if r == c else 0.0
            mx = max(mx, abs(float(matrix_world[r][c]) - expected))
    return mx


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True, help="Original/template MfbC01.smd")
    p.add_argument("--skeleton", required=True, help="Companion m5.smb")
    p.add_argument("--object", default="MsmC01")
    p.add_argument("--out", required=True)
    p.add_argument("--epsilon", type=float, default=1e-6,
                   help="World-space change threshold in Blender units")
    p.add_argument("--search-radius", type=int, default=3,
                   help="Integer local-coordinate search radius around inverse solution")
    p.add_argument("--max-fit-error", type=float, default=0.02,
                   help="Reject a changed vertex if SMD fixed-point fit exceeds this")
    p.add_argument("--expect-changed", type=int,
                   help="Require exactly N changed vertices (use 0 for no-op gate)")
    p.add_argument("--report", help="Optional JSON report path")
    return p.parse_args(argv)


def main(argv):
    args = parse_args(argv)

    smd_path = Path(args.smd).expanduser().resolve()
    skeleton_path = Path(args.skeleton).expanduser().resolve()
    out_path = Path(args.out).expanduser().resolve()

    if out_path == smd_path:
        raise ValueError("refusing in-place SMD overwrite; choose a different --out")
    if args.search_radius < 0 or args.search_radius > 10:
        raise ValueError("--search-radius must be between 0 and 10")

    src = smd_path.read_bytes()
    skeleton_data = skeleton_path.read_bytes()
    parsed = parse_object(src, args.object)
    skeleton = parse_skeleton_tms(skeleton_data)

    obj = bpy.data.objects.get(args.object)
    if obj is None:
        raise ValueError(f"Blender object {args.object!r} not found")
    if obj.type != "MESH":
        raise ValueError(f"Blender object {args.object!r} is {obj.type}, expected MESH")

    mesh = obj.data
    if len(mesh.vertices) != parsed["vertex_count"]:
        raise ValueError(
            f"vertex count changed: Blender={len(mesh.vertices)} "
            f"SMD={parsed['vertex_count']}; V1 only accepts existing topology"
        )
    if len(mesh.polygons) != parsed["face_count"]:
        raise ValueError(
            f"face count changed: Blender={len(mesh.polygons)} "
            f"SMD={parsed['face_count']}; V1 only accepts existing topology"
        )

    identity_err = matrix_identity_error(obj.matrix_world)
    if identity_err > 1e-7:
        raise ValueError(
            f"{args.object}: object matrix is not identity "
            f"(max error {identity_err:.9g}). V1 expects Edit Mode vertex edits only."
        )

    # Validate importer metadata when available.
    saved_v = obj.get("pt_smd_vertex_count")
    saved_f = obj.get("pt_smd_face_count")
    if saved_v is not None and int(saved_v) != parsed["vertex_count"]:
        raise ValueError("pt_smd_vertex_count metadata does not match original SMD")
    if saved_f is not None and int(saved_f) != parsed["face_count"]:
        raise ValueError("pt_smd_face_count metadata does not match original SMD")

    patched = bytearray(src)
    changes = []
    max_fit_error = 0.0

    for idx, vertex in enumerate(mesh.vertices):
        serialized = parsed["vertices"][idx]
        original_local = serialized[:3]
        bone_name = parsed["bones"][idx]
        if not bone_name:
            raise ValueError(f"vertex {idx}: empty Physique bone")
        tm = skeleton.get(bone_name)
        if tm is None:
            raise ValueError(f"vertex {idx}: Physique bone {bone_name!r} missing from SMB")

        original_world_fixed = transform_point_fixed(original_local, tm)
        original_world = tuple(v / 256.0 for v in original_world_fixed)
        current_world = tuple(float(x) for x in vertex.co)

        move = math.sqrt(sum((current_world[k] - original_world[k]) ** 2 for k in range(3)))
        if move <= args.epsilon:
            continue

        new_local, represented_world, fit_error = choose_local_fixed(
            current_world, tm, original_local, args.search_radius
        )
        max_fit_error = max(max_fit_error, fit_error)
        if fit_error > args.max_fit_error:
            raise ValueError(
                f"vertex {idx}: fixed-point fit error {fit_error:.9g} exceeds "
                f"--max-fit-error {args.max_fit_error}"
            )

        off = parsed["layout"]["vertices"] + idx * SIZE_VERTEX
        struct.pack_into("<3i", patched, off, *new_local)
        changes.append({
            "index": idx,
            "bone": bone_name,
            "move_from_original": move,
            "original_world": original_world,
            "edited_world": current_world,
            "represented_world": represented_world,
            "fit_error": fit_error,
            "original_local_fixed": original_local,
            "new_local_fixed": new_local,
        })

    if args.expect_changed is not None and len(changes) != args.expect_changed:
        raise ValueError(
            f"changed vertex count {len(changes)} != --expect-changed {args.expect_changed}"
        )

    # Byte audit: all differences must be within XYZ fields of changed vertices.
    allowed = set()
    for ch in changes:
        off = parsed["layout"]["vertices"] + ch["index"] * SIZE_VERTEX
        allowed.update(range(off, off + 12))

    diff_offsets = [i for i, (a, b) in enumerate(zip(src, patched)) if a != b]
    illegal = [i for i in diff_offsets if i not in allowed]
    if len(src) != len(patched):
        raise ValueError("internal error: output size changed")
    if illegal:
        raise ValueError(f"internal error: {len(illegal)} changed bytes outside allowed XYZ fields")

    # Structural reparse after patch.
    reparsed = parse_object(bytes(patched), args.object)
    if reparsed["vertex_count"] != parsed["vertex_count"] or reparsed["face_count"] != parsed["face_count"]:
        raise ValueError("post-patch structural count mismatch")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(patched)

    report = {
        "tool": "pt_blender_patch_smd_vertices_v1",
        "blend": bpy.data.filepath,
        "object": args.object,
        "input_smd": str(smd_path),
        "output_smd": str(out_path),
        "input_size": len(src),
        "output_size": len(patched),
        "input_sha256": sha256_bytes(src),
        "output_sha256": sha256_bytes(patched),
        "byte_identical": src == bytes(patched),
        "vertex_count": parsed["vertex_count"],
        "face_count": parsed["face_count"],
        "changed_vertices": len(changes),
        "changed_bytes": len(diff_offsets),
        "max_fit_error": max_fit_error,
        "object_matrix_identity_error": identity_err,
        "changes": changes,
    }

    if args.report:
        report_path = Path(args.report).expanduser().resolve()
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print("PT_BLENDER_PATCH_SMD_VERTICES_V1")
    print("blend=", bpy.data.filepath)
    print("object=", args.object)
    print("vertices=", parsed["vertex_count"])
    print("faces=", parsed["face_count"])
    print("changed_vertices=", len(changes))
    print("changed_bytes=", len(diff_offsets))
    print("max_fit_error=", f"{max_fit_error:.9g}")
    print("input_size=", len(src))
    print("output_size=", len(patched))
    print("input_sha256=", report["input_sha256"])
    print("output_sha256=", report["output_sha256"])
    print("byte_identical=", report["byte_identical"])

    for ch in changes:
        print(
            "PATCH",
            f"v={ch['index']}",
            f"bone={ch['bone']!r}",
            f"move={ch['move_from_original']:.9g}",
            f"fit={ch['fit_error']:.9g}",
            f"local={tuple(ch['original_local_fixed'])}->{tuple(ch['new_local_fixed'])}",
        )

    print("patch_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"patch_check=FAIL: {type(exc).__name__}: {exc}")
        raise

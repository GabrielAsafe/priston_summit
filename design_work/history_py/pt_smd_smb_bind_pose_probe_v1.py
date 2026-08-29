#!/usr/bin/env python3
# Read-only probe:
# reconstruct MfbC01.smd bind/world coordinates by reversing ReformPhysique
# with the serialized bone Tm from m5.smb.
#
# No Blender, no asset writes, no sys.exit/exit.

import argparse
import struct
from collections import defaultdict
from pathlib import Path

HEADER = b"SMD Model data Ver 0.62"

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

FLOATNS = 8
FONE = 256

# smOBJ3D offsets, already ABI-proved for current i686 build.
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


def trunc_div(n, d):
    if d == 0:
        raise ZeroDivisionError
    sign = -1 if (n < 0) ^ (d < 0) else 1
    return sign * (abs(n) // abs(d))


def read_matrix_i32(data, off):
    return list(struct.unpack_from("<16i", data, off))


def mget(m, row, col):
    return m[(row - 1) * 4 + (col - 1)]


def parse_meta(data, label):
    if len(data) < SIZE_DFILE_HEADER:
        raise RuntimeError(f"{label}: file too small")
    if not data.startswith(HEADER):
        raise RuntimeError(f"{label}: invalid header")

    obj_count = i32(data, 24)
    mat_count = i32(data, 28)
    mat_point = i32(data, 32)
    first_obj = i32(data, 36)

    table_end = SIZE_DFILE_HEADER + obj_count * SIZE_DFILE_OBJINFO
    if table_end != mat_point:
        raise RuntimeError(
            f"{label}: table_end={table_end} != MatFilePoint={mat_point}"
        )

    infos = []
    for idx in range(obj_count):
        off = SIZE_DFILE_HEADER + idx * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        infos.append({
            "index": idx,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })

    return {
        "obj_count": obj_count,
        "mat_count": mat_count,
        "mat_point": mat_point,
        "first_obj": first_obj,
        "infos": infos,
    }


def object_layout(start, nv, nf, nt, nr, np, ns, has_physique):
    vertices = start + SIZE_OBJ3D
    faces = vertices + nv * SIZE_VERTEX
    texlinks = faces + nf * SIZE_FACE
    tmrot = texlinks + nt * SIZE_TEXLINK
    tmpos = tmrot + nr * SIZE_TM_ROT
    tmscale = tmpos + np * SIZE_TM_POS
    prevrot = tmscale + ns * SIZE_TM_SCALE
    physique = prevrot + nr * SIZE_MATRIX if has_physique else None
    end = (physique + nv * 32) if has_physique else (prevrot + nr * SIZE_MATRIX)
    return {
        "vertices": vertices,
        "faces": faces,
        "texlinks": texlinks,
        "physique": physique,
        "end": end,
    }


def transform_point_fixed(v, m):
    x, y, z = v
    rx = x * mget(m, 1, 1) + y * mget(m, 2, 1) + z * mget(m, 3, 1)
    ry = x * mget(m, 1, 2) + y * mget(m, 2, 2) + z * mget(m, 3, 2)
    rz = x * mget(m, 1, 3) + y * mget(m, 2, 3) + z * mget(m, 3, 3)

    ox = (rx >> FLOATNS) + mget(m, 4, 1)
    oy = (ry >> FLOATNS) + mget(m, 4, 2)
    oz = (rz >> FLOATNS) + mget(m, 4, 3)
    return (ox, oy, oz)


def invert_matrix_source(a):
    # Exact integer structure of smMatrixInvert() from smmatrix.cpp.
    if abs(mget(a, 4, 4) - FONE) > 1:
        raise RuntimeError(f"matrix _44 invalid: {mget(a,4,4)}")
    if any(abs(mget(a, r, 4)) > 1 for r in (1, 2, 3)):
        raise RuntimeError(
            "matrix affine column invalid: "
            f"_14={mget(a,1,4)} _24={mget(a,2,4)} _34={mget(a,3,4)}"
        )

    a11,a12,a13 = mget(a,1,1),mget(a,1,2),mget(a,1,3)
    a21,a22,a23 = mget(a,2,1),mget(a,2,2),mget(a,2,3)
    a31,a32,a33 = mget(a,3,1),mget(a,3,2),mget(a,3,3)

    det = (
        a11 * ((a22 * a33 - a23 * a32) >> FLOATNS)
        - a12 * ((a21 * a33 - a23 * a31) >> FLOATNS)
        + a13 * ((a21 * a32 - a22 * a31) >> FLOATNS)
    ) >> FLOATNS

    if det == 0:
        raise RuntimeError("matrix determinant is zero")

    det_inv = trunc_div(FONE << FLOATNS, det)

    q = [0] * 16
    q[0]  = (det_inv * ((a22*a33 - a23*a32) >> FLOATNS)) >> FLOATNS
    q[1]  = -(det_inv * ((a12*a33 - a13*a32) >> FLOATNS) >> FLOATNS)
    q[2]  = (det_inv * ((a12*a23 - a13*a22) >> FLOATNS)) >> FLOATNS
    q[3]  = 0

    q[4]  = -(det_inv * ((a21*a33 - a23*a31) >> FLOATNS) >> FLOATNS)
    q[5]  = (det_inv * ((a11*a33 - a13*a31) >> FLOATNS)) >> FLOATNS
    q[6]  = -(det_inv * ((a11*a23 - a13*a21) >> FLOATNS) >> FLOATNS)
    q[7]  = 0

    q[8]  = (det_inv * ((a21*a32 - a22*a31) >> FLOATNS)) >> FLOATNS
    q[9]  = -(det_inv * ((a11*a32 - a12*a31) >> FLOATNS) >> FLOATNS)
    q[10] = (det_inv * ((a11*a22 - a12*a21) >> FLOATNS)) >> FLOATNS
    q[11] = 0

    a41,a42,a43 = mget(a,4,1),mget(a,4,2),mget(a,4,3)
    q[12] = -((a41*q[0] + a42*q[4] + a43*q[8]) >> FLOATNS)
    q[13] = -((a41*q[1] + a42*q[5] + a43*q[9]) >> FLOATNS)
    q[14] = -((a41*q[2] + a42*q[6] + a43*q[10]) >> FLOATNS)
    q[15] = FONE

    return q


def bbox(points):
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    zs = [p[2] for p in points]
    return (
        min(xs), max(xs),
        min(ys), max(ys),
        min(zs), max(zs),
    )


def fmt_bbox_fixed(b):
    vals = [v / FONE for v in b]
    return (
        f"x=[{vals[0]:.3f},{vals[1]:.3f}] "
        f"y=[{vals[2]:.3f},{vals[3]:.3f}] "
        f"z=[{vals[4]:.3f},{vals[5]:.3f}] "
        f"size=[{vals[1]-vals[0]:.3f},"
        f"{vals[3]-vals[2]:.3f},"
        f"{vals[5]-vals[4]:.3f}]"
    )


def parse_skeleton(data, meta):
    bones = {}

    for info in meta["infos"]:
        raw = data[info["start"]:info["start"] + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise RuntimeError(f"skeleton object {info['index']}: truncated")

        name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        parent = cstr(raw[O_OFF_NODEPARENT:O_OFF_NODEPARENT + 32])
        tm = read_matrix_i32(raw, O_OFF_TM)

        if name in bones:
            raise RuntimeError(f"duplicate skeleton NodeName: {name!r}")

        bones[name] = {
            "parent": parent,
            "tm": tm,
            "inv_tm": invert_matrix_source(tm),
        }

    return bones


def parse_mesh_and_reconstruct(data, meta, bones):
    raw_points = []
    reconstructed = []
    per_object = {}
    bone_use = defaultdict(int)
    roundtrip_errors = []

    for info in meta["infos"]:
        start = info["start"]
        raw = data[start:start + SIZE_OBJ3D]

        name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        physique_ptr = u32(raw, O_OFF_PHYSIQUE_PTR)
        nv = i32(raw, O_OFF_NVERTEX)
        nf = i32(raw, O_OFF_NFACE)
        nt = i32(raw, O_OFF_NTEXLINK)
        nr = i32(raw, O_OFF_TMROTCNT)
        np = i32(raw, O_OFF_TMPOSCNT)
        ns = i32(raw, O_OFF_TMSCALECNT)

        layout = object_layout(
            start, nv, nf, nt, nr, np, ns, physique_ptr != 0
        )
        if layout["end"] != start + info["length"]:
            raise RuntimeError(f"{name}: object length mismatch")
        if layout["physique"] is None:
            raise RuntimeError(f"{name}: expected Physique but none serialized")

        obj_raw = []
        obj_rec = []

        for vidx in range(nv):
            voff = layout["vertices"] + vidx * SIZE_VERTEX
            vx, vy, vz = struct.unpack_from("<3i", data, voff)

            poff = layout["physique"] + vidx * 32
            bone_name = cstr(data[poff:poff + 32])
            if not bone_name:
                raise RuntimeError(f"{name}[{vidx}]: empty Physique name")
            if bone_name not in bones:
                raise RuntimeError(
                    f"{name}[{vidx}]: missing bone {bone_name!r}"
                )

            local = (vx, vy, vz)
            world = transform_point_fixed(local, bones[bone_name]["tm"])
            back = transform_point_fixed(world, bones[bone_name]["inv_tm"])

            err = (
                back[0] - local[0],
                back[1] - local[1],
                back[2] - local[2],
            )
            roundtrip_errors.append(err)

            obj_raw.append(local)
            obj_rec.append(world)
            raw_points.append(local)
            reconstructed.append(world)
            bone_use[bone_name] += 1

        per_object[name] = {
            "raw_bbox": bbox(obj_raw),
            "reconstructed_bbox": bbox(obj_rec),
            "count": nv,
        }

    return raw_points, reconstructed, per_object, bone_use, roundtrip_errors


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--mesh", required=True, type=Path)
    ap.add_argument("--skeleton", required=True, type=Path)
    args = ap.parse_args()

    mesh_data = args.mesh.read_bytes()
    skel_data = args.skeleton.read_bytes()

    mesh_meta = parse_meta(mesh_data, "mesh")
    skel_meta = parse_meta(skel_data, "skeleton")
    bones = parse_skeleton(skel_data, skel_meta)

    raw_points, rec_points, per_obj, bone_use, errors = (
        parse_mesh_and_reconstruct(mesh_data, mesh_meta, bones)
    )

    max_abs = max(abs(v) for e in errors for v in e)
    exact = sum(1 for e in errors if e == (0, 0, 0))
    within1 = sum(1 for e in errors if max(abs(v) for v in e) <= 1)
    within2 = sum(1 for e in errors if max(abs(v) for v in e) <= 2)

    print("PT_SMD_SMB_BIND_POSE_PROBE_V1")
    print(f"mesh={args.mesh}")
    print(f"skeleton={args.skeleton}")
    print()
    print("COUNTS")
    print(f"mesh_objects={mesh_meta['obj_count']}")
    print(f"skeleton_objects={skel_meta['obj_count']}")
    print(f"vertices={len(raw_points)}")
    print(f"bones_used={len(bone_use)}")
    print()
    print("BBOX_RAW_SMD_LOCAL")
    print(fmt_bbox_fixed(bbox(raw_points)))
    print()
    print("BBOX_RECONSTRUCTED_BY_BONE_TM")
    print(fmt_bbox_fixed(bbox(rec_points)))
    print()
    print("ROUNDTRIP")
    print(f"exact_vertices={exact}/{len(errors)}")
    print(f"within_1_fixed_unit={within1}/{len(errors)}")
    print(f"within_2_fixed_units={within2}/{len(errors)}")
    print(f"max_abs_error_fixed={max_abs}")
    print(f"max_abs_error_model_units={max_abs / FONE:.6f}")
    print()
    print("PER_OBJECT_RECONSTRUCTED_BBOX")
    for name, info in per_obj.items():
        print(
            f"{name}: V={info['count']} "
            f"{fmt_bbox_fixed(info['reconstructed_bbox'])}"
        )
    print()

    # This is a diagnostic gate, not a claim of visual proof.
    # PASS means structure/matrix direction is internally consistent.
    if len(raw_points) != 1444:
        raise RuntimeError(f"unexpected vertex count: {len(raw_points)}")
    if len(bone_use) != 44:
        raise RuntimeError(f"unexpected used bone count: {len(bone_use)}")
    if max_abs > 8:
        raise RuntimeError(
            f"roundtrip error too large for fixed-point inverse: {max_abs}"
        )

    print("bind_pose_math_check=PASS")


main()

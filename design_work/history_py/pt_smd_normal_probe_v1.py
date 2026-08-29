#!/usr/bin/env python3
"""
Summit Tale / Priston Tale - SMD normal convention probe V1.

Run inside Blender on the pristine C01 blend.

Purpose:
  Determine how serialized smVERTEX nx/ny/nz correspond to Blender normals
  after the already-proven SMD + Physique + bone.Tm bind reconstruction.

This script is READ-ONLY. It does not save or modify the blend/SMD.

Example:
  flatpak run --filesystem=home org.blender.Blender \
    --background MfbC01_direct_smd_v3_static_armature_clean.blend \
    --python pt_smd_normal_probe_v1.py -- \
    --smd MfbC01.smd \
    --skeleton m5.smb \
    --object MsmC01
"""

import argparse
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

O_OFF_PHYSIQUE_PTR = 16
O_OFF_NVERTEX = 84
O_OFF_NFACE = 88
O_OFF_NTEXLINK = 92
O_OFF_NODENAME = 172
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


def parse_header_and_infos(data):
    if len(data) < SIZE_DFILE_HEADER:
        raise ValueError("file smaller than smDFILE_HEADER")
    if not data.startswith(SMD_HEADER_TEXT):
        raise ValueError("unexpected SMD header")

    obj_counter = i32(data, 24)
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
    return infos


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
        "physique": physique,
        "end": end,
    }


def parse_object(data, object_name):
    infos = parse_header_and_infos(data)
    info = next((x for x in infos if x["name"] == object_name), None)
    if info is None:
        raise ValueError(f"SMD object {object_name!r} not found")

    start = info["start"]
    raw = data[start:start + SIZE_OBJ3D]
    if len(raw) != SIZE_OBJ3D:
        raise ValueError("truncated smOBJ3D")

    node_name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
    if node_name != object_name:
        raise ValueError("object table / NodeName mismatch")

    nv = i32(raw, O_OFF_NVERTEX)
    nf = i32(raw, O_OFF_NFACE)
    nt = i32(raw, O_OFF_NTEXLINK)
    nr = i32(raw, O_OFF_TMROTCNT)
    np = i32(raw, O_OFF_TMPOSCNT)
    ns = i32(raw, O_OFF_TMSCALECNT)
    has_physique = u32(raw, O_OFF_PHYSIQUE_PTR) != 0
    if not has_physique:
        raise ValueError("object has no Physique")

    layout = object_layout(start, nv, nf, nt, nr, np, ns, has_physique)
    if layout["end"] != start + info["length"]:
        raise ValueError("serialized object length mismatch")

    verts = []
    for idx in range(nv):
        off = layout["vertices"] + idx * SIZE_VERTEX
        vals = struct.unpack_from("<6i", data, off)
        verts.append({
            "xyz": vals[:3],
            "normal": vals[3:6],
        })

    bones = []
    p = layout["physique"]
    for idx in range(nv):
        bones.append(cstr(data[p + idx * 32:p + (idx + 1) * 32]))

    return {
        "vertex_count": nv,
        "face_count": nf,
        "vertices": verts,
        "bones": bones,
    }


def parse_skeleton_tms(data):
    infos = parse_header_and_infos(data)
    out = {}
    for info in infos:
        raw = data[info["start"]:info["start"] + SIZE_OBJ3D]
        name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        tm = struct.unpack_from("<16i", raw, O_OFF_TM)
        out[name] = tm
    return out


def mval(tm, row, col):
    return tm[(row - 1) * 4 + (col - 1)]


def normalize(v):
    x, y, z = (float(v[0]), float(v[1]), float(v[2]))
    n = math.sqrt(x*x + y*y + z*z)
    if n <= 1e-20:
        return None
    return (x/n, y/n, z/n)


def dot(a, b):
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]


def angle_deg(a, b):
    if a is None or b is None:
        return None
    d = max(-1.0, min(1.0, dot(a, b)))
    return math.degrees(math.acos(d))


def transform_row(n, tm):
    x, y, z = n
    return (
        x*mval(tm,1,1) + y*mval(tm,2,1) + z*mval(tm,3,1),
        x*mval(tm,1,2) + y*mval(tm,2,2) + z*mval(tm,3,2),
        x*mval(tm,1,3) + y*mval(tm,2,3) + z*mval(tm,3,3),
    )


def transform_col(n, tm):
    x, y, z = n
    return (
        x*mval(tm,1,1) + y*mval(tm,1,2) + z*mval(tm,1,3),
        x*mval(tm,2,1) + y*mval(tm,2,2) + z*mval(tm,2,3),
        x*mval(tm,3,1) + y*mval(tm,3,2) + z*mval(tm,3,3),
    )


def matrix_identity_error(matrix_world):
    mx = 0.0
    for r in range(4):
        for c in range(4):
            expected = 1.0 if r == c else 0.0
            mx = max(mx, abs(float(matrix_world[r][c]) - expected))
    return mx


def stats(values):
    vals = sorted(v for v in values if v is not None)
    if not vals:
        return None
    def pct(p):
        if len(vals) == 1:
            return vals[0]
        pos = (len(vals) - 1) * p
        lo = int(math.floor(pos))
        hi = int(math.ceil(pos))
        if lo == hi:
            return vals[lo]
        t = pos - lo
        return vals[lo]*(1-t) + vals[hi]*t
    return {
        "count": len(vals),
        "mean": sum(vals)/len(vals),
        "p50": pct(0.50),
        "p95": pct(0.95),
        "max": vals[-1],
    }


def fmt_stat(s):
    if not s:
        return "NO_DATA"
    return (
        f"count={s['count']} mean={s['mean']:.6f}deg "
        f"p50={s['p50']:.6f}deg p95={s['p95']:.6f}deg "
        f"max={s['max']:.6f}deg"
    )


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--skeleton", required=True)
    p.add_argument("--object", action="append",
                   help="Object to probe; repeatable. Default: MshC01/MsmC01/MslC01")
    p.add_argument("--top", type=int, default=8,
                   help="Print N worst vertices for the best candidate")
    return p.parse_args(argv)


def main(argv):
    args = parse_args(argv)
    objects = args.object or ["MshC01", "MsmC01", "MslC01"]

    smd = Path(args.smd).expanduser().resolve().read_bytes()
    smb = Path(args.skeleton).expanduser().resolve().read_bytes()
    skeleton = parse_skeleton_tms(smb)

    print("PT_SMD_NORMAL_PROBE_V1")
    print("blend=", bpy.data.filepath)
    print("objects=", objects)

    overall = {}

    for object_name in objects:
        parsed = parse_object(smd, object_name)
        obj = bpy.data.objects.get(object_name)
        if obj is None or obj.type != "MESH":
            raise ValueError(f"Blender mesh {object_name!r} not found")

        mesh = obj.data
        mesh.update()

        if len(mesh.vertices) != parsed["vertex_count"]:
            raise ValueError(
                f"{object_name}: vertex count differs "
                f"Blender={len(mesh.vertices)} SMD={parsed['vertex_count']}; "
                "run this probe on the pristine clean blend"
            )

        ident = matrix_identity_error(obj.matrix_world)
        if ident > 1e-7:
            raise ValueError(f"{object_name}: object matrix is not identity")

        candidate_errors = {
            "RAW": [],
            "NEG_RAW": [],
            "BONE_ROW": [],
            "NEG_BONE_ROW": [],
            "BONE_COL": [],
            "NEG_BONE_COL": [],
        }
        per_vertex = []

        for idx, bv in enumerate(mesh.vertices):
            blender_n = normalize(tuple(float(x) for x in bv.normal))
            smd_n = normalize(parsed["vertices"][idx]["normal"])
            if smd_n is None:
                raise ValueError(f"{object_name} v={idx}: zero serialized normal")

            bone = parsed["bones"][idx]
            tm = skeleton.get(bone)
            if tm is None:
                raise ValueError(f"{object_name} v={idx}: missing bone {bone!r}")

            row_n = normalize(transform_row(smd_n, tm))
            col_n = normalize(transform_col(smd_n, tm))

            candidates = {
                "RAW": smd_n,
                "NEG_RAW": tuple(-x for x in smd_n),
                "BONE_ROW": row_n,
                "NEG_BONE_ROW": tuple(-x for x in row_n),
                "BONE_COL": col_n,
                "NEG_BONE_COL": tuple(-x for x in col_n),
            }

            errs = {}
            for key, candidate in candidates.items():
                e = angle_deg(candidate, blender_n)
                candidate_errors[key].append(e)
                errs[key] = e

            per_vertex.append((idx, bone, errs))

        ranked = sorted(
            ((stats(v), k) for k, v in candidate_errors.items()),
            key=lambda x: (x[0]["mean"], x[0]["p95"], x[0]["max"])
        )

        print()
        print(f"OBJECT {object_name}")
        print(f"vertices={parsed['vertex_count']} faces={parsed['face_count']}")
        for s, key in ranked:
            print(f"  {key:16s} {fmt_stat(s)}")

        best_stat, best_key = ranked[0]
        overall[object_name] = (best_key, best_stat)
        print(f"BEST={best_key}")

        worst = sorted(
            ((pv[2][best_key], pv[0], pv[1]) for pv in per_vertex),
            reverse=True
        )[:max(0, args.top)]
        for err, idx, bone in worst:
            print(f"  WORST v={idx} bone={bone!r} angle={err:.6f}deg")

    print()
    print("SUMMARY")
    for name, (key, s) in overall.items():
        print(f"{name}: best={key} mean={s['mean']:.6f} p95={s['p95']:.6f} max={s['max']:.6f}")

    # Strong automatic gate only if all objects choose same convention and are very close.
    keys = {x[0] for x in overall.values()}
    max_p95 = max(x[1]["p95"] for x in overall.values())
    max_max = max(x[1]["max"] for x in overall.values())

    if len(keys) == 1 and max_p95 <= 2.0 and max_max <= 8.0:
        print(f"normal_convention={next(iter(keys))}")
        print("normal_probe_check=PASS")
    else:
        print("normal_probe_check=INCONCLUSIVE")
        print("NOTE: do not serialize new SMD vertex normals yet.")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"normal_probe_check=FAIL: {type(exc).__name__}: {exc}")
        raise

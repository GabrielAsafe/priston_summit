#!/usr/bin/env python3
# Priston Tale SMB rest-matrix axis alignment probe.
# Read-only. Determines which Tm basis axis most closely follows child direction.

import argparse
import math
import statistics
import struct
from collections import Counter, defaultdict
from pathlib import Path

HEADER = b"SMD Model data Ver 0.62"

SIZE_DFILE_HEADER = 556
SIZE_DFILE_OBJINFO = 40
SIZE_OBJ3D = 2236

O_OFF_NODENAME = 172
O_OFF_NODEPARENT = 204
O_OFF_TM = 240

FONE = 256.0


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def matrix_i32(data, off):
    return list(struct.unpack_from("<16i", data, off))


def mget(m, row, col):
    return m[(row - 1) * 4 + (col - 1)]


def vsub(a, b):
    return (a[0]-b[0], a[1]-b[1], a[2]-b[2])


def vdot(a, b):
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]


def vlen(a):
    return math.sqrt(vdot(a, a))


def vnorm(a):
    n = vlen(a)
    if n == 0:
        return None
    return (a[0]/n, a[1]/n, a[2]/n)


def det3(rows):
    a, b, c = rows
    return (
        a[0]*(b[1]*c[2] - b[2]*c[1])
        - a[1]*(b[0]*c[2] - b[2]*c[0])
        + a[2]*(b[0]*c[1] - b[1]*c[0])
    )


def parse(data):
    if len(data) < SIZE_DFILE_HEADER:
        raise RuntimeError("file too small")
    if not data.startswith(HEADER):
        raise RuntimeError("invalid header")

    obj_count = i32(data, 24)
    mat_point = i32(data, 32)

    expected = SIZE_DFILE_HEADER + obj_count * SIZE_DFILE_OBJINFO
    if mat_point != expected:
        raise RuntimeError(
            f"object table mismatch: MatFilePoint={mat_point} expected={expected}"
        )

    infos = []
    for idx in range(obj_count):
        off = SIZE_DFILE_HEADER + idx * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        infos.append({
            "index": idx,
            "name": cstr(raw[:32]),
            "start": i32(raw, 36),
        })

    bones = {}

    for info in infos:
        raw = data[info["start"]:info["start"] + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise RuntimeError(f"truncated object {info['index']}")

        name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME+32])
        parent = cstr(raw[O_OFF_NODEPARENT:O_OFF_NODEPARENT+32])
        tm = matrix_i32(raw, O_OFF_TM)

        if name != info["name"]:
            raise RuntimeError(
                f"name mismatch {info['name']!r} != {name!r}"
            )

        pos = (
            mget(tm, 4, 1),
            mget(tm, 4, 2),
            mget(tm, 4, 3),
        )

        rows = {
            "X": (mget(tm,1,1), mget(tm,1,2), mget(tm,1,3)),
            "Y": (mget(tm,2,1), mget(tm,2,2), mget(tm,2,3)),
            "Z": (mget(tm,3,1), mget(tm,3,2), mget(tm,3,3)),
        }

        cols = {
            "X": (mget(tm,1,1), mget(tm,2,1), mget(tm,3,1)),
            "Y": (mget(tm,1,2), mget(tm,2,2), mget(tm,3,2)),
            "Z": (mget(tm,1,3), mget(tm,2,3), mget(tm,3,3)),
        }

        bones[name] = {
            "parent": parent,
            "tm": tm,
            "pos": pos,
            "rows": rows,
            "cols": cols,
        }

    return bones


def analyze_basis(bones, children, basis_key):
    results = []

    for name, bone in bones.items():
        kids = children.get(name, [])
        if len(kids) != 1:
            continue

        child = bones[kids[0]]
        direction = vnorm(vsub(child["pos"], bone["pos"]))
        if direction is None:
            continue

        scores = {}
        signed = {}

        for axis_name, raw_axis in bone[basis_key].items():
            axis = vnorm(raw_axis)
            if axis is None:
                score = -1.0
                sign_score = 0.0
            else:
                sign_score = vdot(axis, direction)
                score = abs(sign_score)

            scores[axis_name] = score
            signed[axis_name] = sign_score

        best_axis = max(scores, key=scores.get)
        results.append({
            "name": name,
            "child": kids[0],
            "best": best_axis,
            "absdot": scores[best_axis],
            "signed": signed[best_axis],
            "scores": scores,
        })

    return results


def summarize(label, results):
    print(label)

    counts = Counter(r["best"] for r in results)
    print(f"single_child_samples={len(results)}")
    print(
        "best_axis_counts="
        + " ".join(f"{k}:{counts.get(k,0)}" for k in ("X","Y","Z"))
    )

    if results:
        vals = [r["absdot"] for r in results]
        print(f"best_absdot_min={min(vals):.6f}")
        print(f"best_absdot_median={statistics.median(vals):.6f}")
        print(f"best_absdot_mean={statistics.mean(vals):.6f}")
        print(f"best_absdot_max={max(vals):.6f}")

    for axis in ("X","Y","Z"):
        subset = [r for r in results if r["best"] == axis]
        if not subset:
            continue

        positive = sum(1 for r in subset if r["signed"] >= 0)
        negative = len(subset) - positive
        vals = [r["absdot"] for r in subset]

        print(
            f"{axis}: count={len(subset)} "
            f"positive={positive} negative={negative} "
            f"median_absdot={statistics.median(vals):.6f}"
        )

    weak = sorted(
        (r for r in results if r["absdot"] < 0.90),
        key=lambda r: r["absdot"]
    )

    print(f"weak_under_0_90={len(weak)}")
    for r in weak[:30]:
        print(
            f"  {r['name']!r} -> {r['child']!r} "
            f"best={r['best']} signed={r['signed']:.6f} "
            f"X={r['scores']['X']:.6f} "
            f"Y={r['scores']['Y']:.6f} "
            f"Z={r['scores']['Z']:.6f}"
        )

    print()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--skeleton", required=True, type=Path)
    args = ap.parse_args()

    bones = parse(args.skeleton.read_bytes())

    children = defaultdict(list)
    for name, bone in bones.items():
        if bone["parent"]:
            if bone["parent"] not in bones:
                raise RuntimeError(
                    f"missing parent {bone['parent']!r} for {name!r}"
                )
            children[bone["parent"]].append(name)

    row_lengths = []
    row_orth = []
    determinants = []

    for bone in bones.values():
        rows = [
            tuple(v / FONE for v in bone["rows"]["X"]),
            tuple(v / FONE for v in bone["rows"]["Y"]),
            tuple(v / FONE for v in bone["rows"]["Z"]),
        ]

        row_lengths.extend(vlen(v) for v in rows)
        row_orth.extend([
            abs(vdot(rows[0], rows[1])),
            abs(vdot(rows[0], rows[2])),
            abs(vdot(rows[1], rows[2])),
        ])
        determinants.append(det3(rows))

    row_results = analyze_basis(bones, children, "rows")
    col_results = analyze_basis(bones, children, "cols")

    print("PT_SMB_AXIS_ALIGNMENT_PROBE_V1")
    print(f"skeleton={args.skeleton}")
    print()

    print("MATRIX_BASIS_QUALITY")
    print(f"bones={len(bones)}")
    print(f"basis_vectors={len(row_lengths)}")
    print(f"basis_length_min={min(row_lengths):.6f}")
    print(f"basis_length_median={statistics.median(row_lengths):.6f}")
    print(f"basis_length_max={max(row_lengths):.6f}")
    print(f"orthogonality_absdot_max={max(row_orth):.6f}")
    print(f"determinant_min={min(determinants):.6f}")
    print(f"determinant_median={statistics.median(determinants):.6f}")
    print(f"determinant_max={max(determinants):.6f}")
    print()

    summarize("ROW_BASIS_ALIGNMENT", row_results)
    summarize("COLUMN_BASIS_ALIGNMENT", col_results)

    print("KEY_SINGLE_CHILD_BONES")
    wanted = [
        "Bip01 L Clavicle",
        "Bip01 L UpperArm",
        "Bip01 L Forearm",
        "Bip01 R Clavicle",
        "Bip01 R UpperArm",
        "Bip01 R Forearm",
        "Bip01 L Thigh",
        "Bip01 L Calf",
        "Bip01 L Foot",
        "Bip01 R Thigh",
        "Bip01 R Calf",
        "Bip01 R Foot",
        "Bip01 Spine",
    ]

    by_name = {r["name"]: r for r in row_results}

    for name in wanted:
        r = by_name.get(name)
        if r is None:
            print(f"{name!r}: not_single_child_or_missing")
            continue

        print(
            f"{name!r} -> {r['child']!r}: "
            f"best_row_axis={r['best']} "
            f"signed={r['signed']:.6f} "
            f"X={r['scores']['X']:.6f} "
            f"Y={r['scores']['Y']:.6f} "
            f"Z={r['scores']['Z']:.6f}"
        )

    print()
    print("axis_alignment_probe=PASS")


main()

#!/usr/bin/env python3
# Priston Tale m5.smb static skeleton/rest-pose probe.
# Read-only. Does not modify client files or Blender files.

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


def parse_meta(data):
    if len(data) < SIZE_DFILE_HEADER:
        raise RuntimeError("file too small")

    if not data.startswith(HEADER):
        raise RuntimeError("invalid SMD/SMB header")

    obj_count = i32(data, 24)
    mat_count = i32(data, 28)
    mat_point = i32(data, 32)

    expected_mat_point = SIZE_DFILE_HEADER + obj_count * SIZE_DFILE_OBJINFO
    if mat_point != expected_mat_point:
        raise RuntimeError(
            f"MatFilePoint mismatch: {mat_point} != {expected_mat_point}"
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
        "infos": infos,
    }


def matrix_i32(data, off):
    return list(struct.unpack_from("<16i", data, off))


def mget(m, row, col):
    return m[(row - 1) * 4 + (col - 1)]


def pos_from_tm(tm):
    return (
        mget(tm, 4, 1),
        mget(tm, 4, 2),
        mget(tm, 4, 3),
    )


def dist_fixed(a, b):
    dx = a[0] - b[0]
    dy = a[1] - b[1]
    dz = a[2] - b[2]
    return math.sqrt(dx * dx + dy * dy + dz * dz)


def fmt_pos(p):
    return (
        f"({p[0] / FONE:.3f}, "
        f"{p[1] / FONE:.3f}, "
        f"{p[2] / FONE:.3f})"
    )


def parse_bones(data, meta):
    bones = {}

    for info in meta["infos"]:
        start = info["start"]
        raw = data[start:start + SIZE_OBJ3D]

        if len(raw) != SIZE_OBJ3D:
            raise RuntimeError(
                f"object {info['index']} truncated at {start}"
            )

        name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        parent = cstr(raw[O_OFF_NODEPARENT:O_OFF_NODEPARENT + 32])
        tm = matrix_i32(raw, O_OFF_TM)

        if name != info["name"]:
            raise RuntimeError(
                f"object table/name mismatch: {info['name']!r} != {name!r}"
            )

        if name in bones:
            raise RuntimeError(f"duplicate bone name: {name!r}")

        bones[name] = {
            "index": info["index"],
            "name": name,
            "parent": parent,
            "tm": tm,
            "pos": pos_from_tm(tm),
        }

    return bones


def bbox(points):
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    zs = [p[2] for p in points]

    return (
        min(xs), max(xs),
        min(ys), max(ys),
        min(zs), max(zs),
    )


def fmt_bbox(b):
    return (
        f"x=[{b[0]/FONE:.3f},{b[1]/FONE:.3f}] "
        f"y=[{b[2]/FONE:.3f},{b[3]/FONE:.3f}] "
        f"z=[{b[4]/FONE:.3f},{b[5]/FONE:.3f}] "
        f"size=[{(b[1]-b[0])/FONE:.3f},"
        f"{(b[3]-b[2])/FONE:.3f},"
        f"{(b[5]-b[4])/FONE:.3f}]"
    )


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--skeleton", required=True, type=Path)
    args = ap.parse_args()

    data = args.skeleton.read_bytes()
    meta = parse_meta(data)
    bones = parse_bones(data, meta)

    children = defaultdict(list)
    roots = []
    missing_parents = []

    affine_bad = []
    edge_rows = []

    for bone in bones.values():
        tm = bone["tm"]

        affine_ok = (
            abs(mget(tm, 1, 4)) <= 1
            and abs(mget(tm, 2, 4)) <= 1
            and abs(mget(tm, 3, 4)) <= 1
            and abs(mget(tm, 4, 4) - 256) <= 1
        )

        if not affine_ok:
            affine_bad.append(bone["name"])

        parent = bone["parent"]

        if not parent:
            roots.append(bone["name"])
            continue

        if parent not in bones:
            missing_parents.append((bone["name"], parent))
            continue

        children[parent].append(bone["name"])

        d = dist_fixed(bone["pos"], bones[parent]["pos"])
        edge_rows.append((bone["name"], parent, d))

    all_points = [b["pos"] for b in bones.values()]
    all_bbox = bbox(all_points)

    edge_lengths = [row[2] / FONE for row in edge_rows]
    zero_edges = [row for row in edge_rows if row[2] == 0]
    near_edges = [
        row for row in edge_rows
        if 0 < row[2] <= 2
    ]

    leaf_bones = [
        name for name in bones
        if not children.get(name)
    ]

    print("PT_SMB_REST_SKELETON_PROBE_V1")
    print(f"skeleton={args.skeleton}")
    print()

    print("STRUCTURE")
    print(f"objects={meta['obj_count']}")
    print(f"materials={meta['mat_count']}")
    print(f"unique_names={len(bones)}")
    print(f"roots={len(roots)}")
    print(f"missing_parents={len(missing_parents)}")
    print(f"leaf_bones={len(leaf_bones)}")
    print(f"affine_tm_invalid={len(affine_bad)}")
    print()

    print("ROOTS")
    for name in roots:
        print(f"{name!r} pos={fmt_pos(bones[name]['pos'])}")
    print()

    print("HEAD_BBOX_FROM_TM_TRANSLATION")
    print(fmt_bbox(all_bbox))
    print()

    print("PARENT_CHILD_EDGES")
    print(f"edges={len(edge_rows)}")
    print(f"zero_length_edges={len(zero_edges)}")
    print(f"near_zero_edges_1_to_2_fixed_units={len(near_edges)}")

    if edge_lengths:
        print(f"min_nonzero_model_units={min((x for x in edge_lengths if x > 0), default=0):.6f}")
        print(f"median_model_units={statistics.median(edge_lengths):.6f}")
        print(f"max_model_units={max(edge_lengths):.6f}")
    print()

    if missing_parents:
        print("MISSING_PARENT_DETAILS")
        for child, parent in missing_parents:
            print(f"child={child!r} parent={parent!r}")
        print()

    if affine_bad:
        print("AFFINE_TM_INVALID_DETAILS")
        for name in affine_bad:
            print(name)
        print()

    if zero_edges:
        print("ZERO_LENGTH_PARENT_CHILD_EDGES")
        for child, parent, d in zero_edges:
            print(
                f"child={child!r} parent={parent!r} "
                f"pos={fmt_pos(bones[child]['pos'])}"
            )
        print()

    if near_edges:
        print("NEAR_ZERO_PARENT_CHILD_EDGES")
        for child, parent, d in near_edges:
            print(
                f"child={child!r} parent={parent!r} "
                f"distance_fixed={d:.3f} "
                f"distance_model={d/FONE:.6f}"
            )
        print()

    sample_names = [
        "Bip01",
        "Bip01 Pelvis",
        "Bip01 Spine",
        "Bip01 Spine1",
        "Bip01 Neck",
        "Bip01 Head",
        "Bip01 L Clavicle",
        "Bip01 L UpperArm",
        "Bip01 L Forearm",
        "Bip01 L Hand",
        "Bip01 R Clavicle",
        "Bip01 R UpperArm",
        "Bip01 R Forearm",
        "Bip01 R Hand",
        "Bip01 L Thigh",
        "Bip01 L Calf",
        "Bip01 L Foot",
        "Bip01 R Thigh",
        "Bip01 R Calf",
        "Bip01 R Foot",
        "Bip-0037",
    ]

    print("KEY_BONES")
    for name in sample_names:
        bone = bones.get(name)
        if bone is None:
            print(f"{name!r}: NOT_FOUND")
            continue

        parent = bone["parent"]
        if parent and parent in bones:
            d = dist_fixed(bone["pos"], bones[parent]["pos"]) / FONE
            dtext = f"{d:.6f}"
        else:
            dtext = "-"

        print(
            f"{name!r}: "
            f"parent={parent!r} "
            f"pos={fmt_pos(bone['pos'])} "
            f"parent_distance={dtext} "
            f"children={len(children.get(name, []))}"
        )
    print()

    # Useful for choosing a Blender tail strategy later.
    child_count_hist = Counter(len(children.get(name, [])) for name in bones)
    print("CHILD_COUNT_HISTOGRAM")
    for count in sorted(child_count_hist):
        print(f"children={count}: bones={child_count_hist[count]}")
    print()

    structural_ok = (
        len(bones) == meta["obj_count"]
        and not missing_parents
        and not affine_bad
    )

    print(f"rest_skeleton_structure_check={'PASS' if structural_ok else 'FAIL'}")


main()

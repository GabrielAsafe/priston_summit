#!/usr/bin/env python3
import argparse
import struct
import sys
from collections import Counter
from pathlib import Path

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
O_OFF_NODEPARENT = 204
O_OFF_TMROTCNT = 684
O_OFF_TMPOSCNT = 688
O_OFF_TMSCALECNT = 692

HEADER = b"SMD Model data Ver 0.62"


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def parse_meta(data, label):
    if len(data) < SIZE_DFILE_HEADER:
        raise ValueError(f"{label}: file too small")
    if not data.startswith(HEADER):
        raise ValueError(f"{label}: invalid header")

    obj_count = i32(data, 24)
    mat_count = i32(data, 28)
    mat_point = i32(data, 32)
    first_obj = i32(data, 36)

    table_end = SIZE_DFILE_HEADER + obj_count * SIZE_DFILE_OBJINFO
    if table_end != mat_point:
        raise ValueError(
            f"{label}: object table end {table_end} != MatFilePoint {mat_point}"
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
    return physique, end


def parse_skeleton_names(data, meta):
    names = []
    parents = {}

    for info in meta["infos"]:
        start = info["start"]
        raw = data[start:start + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise ValueError(f"SMB object {info['index']}: truncated smOBJ3D")

        node = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        parent = cstr(raw[O_OFF_NODEPARENT:O_OFF_NODEPARENT + 32])

        if node != info["name"]:
            raise ValueError(
                f"SMB object {info['index']}: table/header name mismatch "
                f"{info['name']!r} != {node!r}"
            )

        names.append(node)
        parents[node] = parent

    return names, parents


def parse_physique_names(data, meta):
    assignments = []
    per_object = {}

    for info in meta["infos"]:
        start = info["start"]
        raw = data[start:start + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise ValueError(f"SMD object {info['index']}: truncated smOBJ3D")

        node = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        physique_ptr = u32(raw, O_OFF_PHYSIQUE_PTR)
        nv = i32(raw, O_OFF_NVERTEX)
        nf = i32(raw, O_OFF_NFACE)
        nt = i32(raw, O_OFF_NTEXLINK)
        nr = i32(raw, O_OFF_TMROTCNT)
        np = i32(raw, O_OFF_TMPOSCNT)
        ns = i32(raw, O_OFF_TMSCALECNT)

        has_physique = physique_ptr != 0
        physique_off, end = object_layout(
            start, nv, nf, nt, nr, np, ns, has_physique
        )

        if end != start + info["length"]:
            raise ValueError(
                f"SMD object {node}: calculated length {end-start} "
                f"!= table length {info['length']}"
            )

        obj_names = []
        if has_physique:
            for vidx in range(nv):
                off = physique_off + vidx * 32
                bone = cstr(data[off:off + 32])
                obj_names.append(bone)
                assignments.append((node, vidx, bone))

        per_object[node] = obj_names

    return assignments, per_object


def main():
    ap = argparse.ArgumentParser(
        description="Match MfbC01.smd Physique names against m5.smb bone NodeNames"
    )
    ap.add_argument("--mesh", required=True, type=Path)
    ap.add_argument("--skeleton", required=True, type=Path)
    args = ap.parse_args()

    mesh_data = args.mesh.read_bytes()
    skel_data = args.skeleton.read_bytes()

    mesh_meta = parse_meta(mesh_data, "mesh")
    skel_meta = parse_meta(skel_data, "skeleton")

    skel_names, parents = parse_skeleton_names(skel_data, skel_meta)
    assignments, per_object = parse_physique_names(mesh_data, mesh_meta)

    skel_set = set(skel_names)
    phys_counter = Counter(bone for _, _, bone in assignments)
    unique_phys = sorted(phys_counter)

    missing = [name for name in unique_phys if name not in skel_set]
    matched = [name for name in unique_phys if name in skel_set]

    duplicate_skel = sorted(
        name for name, count in Counter(skel_names).items() if count > 1
    )

    empty_assignments = sum(1 for _, _, bone in assignments if not bone)

    print("PT_SMD_SMB_BONE_MATCH_V1")
    print(f"mesh={args.mesh}")
    print(f"skeleton={args.skeleton}")
    print()
    print("MESH")
    print(f"objects={mesh_meta['obj_count']}")
    print(f"physique_assignments={len(assignments)}")
    print(f"unique_physique_names={len(unique_phys)}")
    print(f"empty_physique_names={empty_assignments}")
    print()
    print("SKELETON")
    print(f"objects={skel_meta['obj_count']}")
    print(f"unique_node_names={len(skel_set)}")
    print(f"duplicate_node_names={len(duplicate_skel)}")
    if duplicate_skel:
        print("duplicates=" + repr(duplicate_skel))

    print()
    print("MATCH")
    print(f"matched_unique_names={len(matched)}")
    print(f"missing_unique_names={len(missing)}")

    if missing:
        for name in missing:
            print(f"MISSING {name!r} assignments={phys_counter[name]}")
    else:
        print("all_physique_names_found=YES")

    print()
    print("PHYSIQUE_BONES")
    for name in unique_phys:
        parent = parents.get(name)
        print(
            f"{name!r}: assignments={phys_counter[name]} "
            f"smb_parent={parent!r}"
        )

    ok = (
        len(assignments) > 0
        and empty_assignments == 0
        and not missing
        and not duplicate_skel
    )

    print()
    print(f"bone_match_check={'PASS' if ok else 'FAIL'}")
    return 0 if ok else 10


if __name__ == "__main__":
    sys.exit(main())

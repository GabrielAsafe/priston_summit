#!/usr/bin/env python3
# Priston Tale bone-usage classifier for one SMD + skeleton SMB + INX.
# Read-only. Classifies direct deform bones, ancestors, and extra animated nodes.

import argparse
import struct
from collections import Counter, defaultdict
from pathlib import Path

SMD_HEADER = b"SMD Model data Ver 0.62"

SIZE_HEADER = 556
SIZE_OBJINFO = 40
SIZE_OBJ3D = 2236
SIZE_VERTEX = 24
SIZE_FACE = 36
SIZE_TEXLINK = 32
SIZE_TM_ROT = 20
SIZE_TM_POS = 16
SIZE_TM_SCALE = 16
SIZE_PREVROT = 64

O_NVERTEX = 84
O_NFACE = 88
O_NTEXLINK = 92
O_NODENAME = 172
O_NODEPARENT = 204
O_TMROTCNT = 684
O_TMPOSCNT = 688
O_TMSCALECNT = 692

MODEL_GROUP_OFFSETS = {
    "high": 0x0C0,
    "default": 0x104,
    "low": 0x148,
}


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def object_infos(data):
    if len(data) < SIZE_HEADER:
        raise RuntimeError("file too small")

    if not data.startswith(SMD_HEADER):
        raise RuntimeError("invalid SMD/SMB header")

    count = i32(data, 24)
    infos = []

    for idx in range(count):
        off = SIZE_HEADER + idx * SIZE_OBJINFO
        raw = data[off:off + SIZE_OBJINFO]
        if len(raw) != SIZE_OBJINFO:
            raise RuntimeError("truncated object table")

        infos.append({
            "index": idx,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })

    return infos


def parse_inx_group(path, lod):
    data = path.read_bytes()
    off = MODEL_GROUP_OFFSETS[lod]

    if off + 68 > len(data):
        raise RuntimeError("INX too small")

    count = i32(data, off)
    if not 0 <= count <= 4:
        raise RuntimeError(f"invalid {lod} ModelNameCnt={count}")

    names = []
    base = off + 4

    for idx in range(4):
        raw = data[base + idx * 16:base + (idx + 1) * 16]
        names.append(cstr(raw))

    return names[:count]


def parse_mesh_physique(path):
    data = path.read_bytes()
    infos = object_infos(data)

    objects = {}
    all_bones = set()

    for info in infos:
        start = info["start"]
        end = start + info["length"]

        fixed = data[start:start + SIZE_OBJ3D]
        if len(fixed) != SIZE_OBJ3D:
            raise RuntimeError(f"truncated fixed object {info['name']!r}")

        name = cstr(fixed[O_NODENAME:O_NODENAME + 32])
        if name != info["name"]:
            raise RuntimeError(
                f"name mismatch {info['name']!r} != {name!r}"
            )

        nv = i32(fixed, O_NVERTEX)
        nf = i32(fixed, O_NFACE)
        nt = i32(fixed, O_NTEXLINK)
        nr = i32(fixed, O_TMROTCNT)
        np = i32(fixed, O_TMPOSCNT)
        ns = i32(fixed, O_TMSCALECNT)

        cursor = start + SIZE_OBJ3D
        cursor += nv * SIZE_VERTEX
        cursor += nf * SIZE_FACE
        cursor += nt * SIZE_TEXLINK
        cursor += nr * SIZE_TM_ROT
        cursor += np * SIZE_TM_POS
        cursor += ns * SIZE_TM_SCALE
        cursor += nr * SIZE_PREVROT

        remainder = end - cursor

        bones = []
        assignments = Counter()

        if remainder == 32 * nv and nv > 0:
            for idx in range(nv):
                raw = data[
                    cursor + idx * 32:
                    cursor + (idx + 1) * 32
                ]
                bone = cstr(raw)
                if bone:
                    bones.append(bone)
                    assignments[bone] += 1
                    all_bones.add(bone)

        elif remainder == 0:
            pass

        else:
            raise RuntimeError(
                f"{name}: unexpected serialized remainder={remainder}, "
                f"expected 0 or {32*nv}"
            )

        objects[name] = {
            "vertex_count": nv,
            "bones": set(bones),
            "assignments": assignments,
        }

    return objects, all_bones


def parse_skeleton(path):
    data = path.read_bytes()
    infos = object_infos(data)

    bones = {}

    for info in infos:
        fixed = data[
            info["start"]:
            info["start"] + SIZE_OBJ3D
        ]

        if len(fixed) != SIZE_OBJ3D:
            raise RuntimeError(f"truncated skeleton object {info['name']!r}")

        name = cstr(fixed[O_NODENAME:O_NODENAME + 32])
        parent = cstr(fixed[O_NODEPARENT:O_NODEPARENT + 32])

        if name != info["name"]:
            raise RuntimeError(
                f"name mismatch {info['name']!r} != {name!r}"
            )

        bones[name] = {
            "parent": parent,
            "rot_count": i32(fixed, O_TMROTCNT),
            "pos_count": i32(fixed, O_TMPOSCNT),
            "scale_count": i32(fixed, O_TMSCALECNT),
        }

    return bones


def ancestor_closure(names, skeleton):
    result = set()

    for name in names:
        seen = set()
        cur = name

        while cur in skeleton:
            parent = skeleton[cur]["parent"]
            if not parent:
                break

            if parent in seen:
                raise RuntimeError(f"parent cycle near {name!r}")

            seen.add(parent)
            result.add(parent)
            cur = parent

    return result


def descendant_closure(names, skeleton):
    children = defaultdict(list)

    for name, bone in skeleton.items():
        parent = bone["parent"]
        if parent:
            children[parent].append(name)

    result = set()
    queue = list(names)

    while queue:
        cur = queue.pop()
        for child in children.get(cur, []):
            if child not in result:
                result.add(child)
                queue.append(child)

    return result


def track_label(bone):
    return (
        f"R={bone['rot_count']} "
        f"P={bone['pos_count']} "
        f"S={bone['scale_count']}"
    )


def print_names(title, names, skeleton):
    print(title)
    print(f"count={len(names)}")

    for name in sorted(names):
        bone = skeleton[name]
        print(
            f"  {name!r} "
            f"parent={bone['parent']!r} "
            f"{track_label(bone)}"
        )

    print()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--mesh", required=True, type=Path)
    ap.add_argument("--skeleton", required=True, type=Path)
    ap.add_argument("--model-info", required=True, type=Path)
    ap.add_argument(
        "--lod",
        choices=("high", "default", "low"),
        default="default",
    )
    args = ap.parse_args()

    mesh_objects, all_mesh_bones = parse_mesh_physique(args.mesh)
    skeleton = parse_skeleton(args.skeleton)
    visible_nodes = parse_inx_group(args.model_info, args.lod)

    missing_visible_nodes = [
        name for name in visible_nodes
        if name not in mesh_objects
    ]
    if missing_visible_nodes:
        raise RuntimeError(
            f"INX references missing SMD nodes: {missing_visible_nodes!r}"
        )

    visible_bones = set()
    visible_assignments = Counter()

    for obj_name in visible_nodes:
        obj = mesh_objects[obj_name]
        visible_bones.update(obj["bones"])
        visible_assignments.update(obj["assignments"])

    missing_mesh_bones = sorted(
        name for name in all_mesh_bones
        if name not in skeleton
    )

    if missing_mesh_bones:
        raise RuntimeError(
            f"Physique bones missing from SMB: {missing_mesh_bones!r}"
        )

    all_names = set(skeleton)
    extra_vs_all_mesh = all_names - all_mesh_bones
    extra_vs_visible = all_names - visible_bones

    visible_ancestors = ancestor_closure(visible_bones, skeleton)
    all_mesh_ancestors = ancestor_closure(all_mesh_bones, skeleton)

    ancestor_only_visible = visible_ancestors - visible_bones
    ancestor_only_all = all_mesh_ancestors - all_mesh_bones

    safe_from_visible_mesh_graph = (
        extra_vs_visible - visible_ancestors
    )

    animated_safe = {
        name for name in safe_from_visible_mesh_graph
        if (
            skeleton[name]["rot_count"]
            or skeleton[name]["pos_count"]
            or skeleton[name]["scale_count"]
        )
    }

    zero_track_safe = safe_from_visible_mesh_graph - animated_safe

    descendants_visible = descendant_closure(visible_bones, skeleton)

    extra_descendants_of_visible = (
        safe_from_visible_mesh_graph & descendants_visible
    )

    roots = sorted(
        name for name, bone in skeleton.items()
        if not bone["parent"]
    )

    print("PT_SMB_BONE_USAGE_CLASSIFIER_V1")
    print(f"mesh={args.mesh}")
    print(f"skeleton={args.skeleton}")
    print(f"model_info={args.model_info}")
    print(f"lod={args.lod}")
    print()

    print("SUMMARY")
    print(f"skeleton_bones={len(skeleton)}")
    print(f"skeleton_roots={roots!r}")
    print(f"smd_objects={len(mesh_objects)}")
    print(f"inx_visible_nodes={visible_nodes!r}")
    print(f"physique_unique_all_smd={len(all_mesh_bones)}")
    print(f"physique_unique_visible_lod={len(visible_bones)}")
    print(f"extra_vs_all_smd={len(extra_vs_all_mesh)}")
    print(f"extra_vs_visible_lod={len(extra_vs_visible)}")
    print(f"ancestor_only_of_visible_deform={len(ancestor_only_visible)}")
    print(f"ancestor_only_of_any_smd_deform={len(ancestor_only_all)}")
    print(
        "extra_not_ancestor_of_visible_deform="
        f"{len(safe_from_visible_mesh_graph)}"
    )
    print(
        "animated_extra_not_ancestor_of_visible_deform="
        f"{len(animated_safe)}"
    )
    print(
        "zero_track_extra_not_ancestor_of_visible_deform="
        f"{len(zero_track_safe)}"
    )
    print(
        "extra_descendants_of_visible_deform="
        f"{len(extra_descendants_of_visible)}"
    )
    print()

    print("VISIBLE_LOD_VERTEX_ASSIGNMENT_TOP")
    for name, count in visible_assignments.most_common():
        print(f"  {name!r}: vertices={count}")
    print()

    print_names(
        "DIRECT_DEFORM_BONES_VISIBLE_LOD",
        visible_bones,
        skeleton,
    )

    print_names(
        "ANCESTOR_ONLY_BONES_NEEDED_TO_PROPAGATE_VISIBLE_LOD",
        ancestor_only_visible,
        skeleton,
    )

    print_names(
        "EXTRA_BONES_NOT_IN_VISIBLE_DEFORM_ANCESTRY",
        safe_from_visible_mesh_graph,
        skeleton,
    )

    print("INTERPRETATION")
    print(
        "direct deform bone = referenced by Physique of a visible SMD object"
    )
    print(
        "ancestor-only bone = no visible vertex assigned directly, "
        "but transform can propagate to a deform bone"
    )
    print(
        "extra not in visible deform ancestry = cannot affect the current "
        "visible mesh through parent->child deformation alone"
    )
    print(
        "IMPORTANT: such extra bones may still be externally used as "
        "weapon/effect/attachment/socket nodes; this probe does not classify "
        "external engine references"
    )
    print()

    ok = (
        not missing_mesh_bones
        and len(roots) == 1
    )

    print(
        "bone_usage_classifier_check="
        + ("PASS" if ok else "FAIL")
    )


main()

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
V5b normals compatibility probe

Purpose:
- compare original ASE MESH_FACENORMAL / MESH_VERTEXNORMAL
  against Blender 5.1 polygon/corner normals
- no file is modified
- no export is performed

Run this against the unmodified V4 .blend.
"""

import argparse
import math
import re
import sys
from pathlib import Path

import bpy


FLOAT = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


def find_braced_blocks(text, token):
    rx = re.compile(rf"(?im)^[ \t]*\*{re.escape(token)}[ \t]*\{{")
    out = []

    for m in rx.finditer(text):
        brace = text.find("{", m.start(), m.end())
        if brace < 0:
            continue

        depth = 0
        quote = False
        escape = False
        i = brace

        while i < len(text):
            ch = text[i]

            if quote:
                if escape:
                    escape = False
                elif ch == "\\":
                    escape = True
                elif ch == '"':
                    quote = False
            else:
                if ch == '"':
                    quote = True
                elif ch == "{":
                    depth += 1
                elif ch == "}":
                    depth -= 1
                    if depth == 0:
                        out.append((m.start(), i + 1, text[m.start():i + 1]))
                        break

            i += 1

    return out


def first_quoted(text, key):
    m = re.search(
        re.escape(key) + r'\s+"([^"]+)"',
        text,
        flags=re.IGNORECASE,
    )
    return m.group(1) if m else None


def first_mesh_block(geom_block):
    blocks = find_braced_blocks(geom_block, "MESH")
    return blocks[0][2] if blocks else None


FACE_NORMAL_RE = re.compile(
    rf"\*MESH_FACENORMAL\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
    flags=re.IGNORECASE,
)

VERTEX_NORMAL_RE = re.compile(
    rf"\*MESH_VERTEXNORMAL\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
    flags=re.IGNORECASE,
)


def parse_normals(mesh_block):
    blocks = find_braced_blocks(mesh_block, "MESH_NORMALS")

    if not blocks:
        return {}, {}

    block = blocks[0][2]
    face_normals = {}
    corner_normals = {}

    current_face = None
    corner_index = 0

    for line in block.splitlines():
        fm = FACE_NORMAL_RE.search(line)
        if fm:
            current_face = int(fm.group(1))
            face_normals[current_face] = (
                float(fm.group(2)),
                float(fm.group(3)),
                float(fm.group(4)),
            )
            corner_index = 0
            continue

        vm = VERTEX_NORMAL_RE.search(line)
        if vm and current_face is not None:
            corner_normals[(current_face, corner_index)] = {
                "vertex_index": int(vm.group(1)),
                "normal": (
                    float(vm.group(2)),
                    float(vm.group(3)),
                    float(vm.group(4)),
                ),
            }
            corner_index += 1

    return face_normals, corner_normals


def vec_tuple(v):
    return (float(v[0]), float(v[1]), float(v[2]))


def delta(a, b):
    return math.sqrt(
        (a[0] - b[0]) ** 2
        + (a[1] - b[1]) ** 2
        + (a[2] - b[2]) ** 2
    )


def dot(a, b):
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]


def parse_args():
    argv = sys.argv

    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    p = argparse.ArgumentParser()
    p.add_argument("--ase", required=True)
    p.add_argument("--object", default="MahC20")
    p.add_argument("--show", type=int, default=12)
    return p.parse_args(argv)


def main():
    args = parse_args()

    ase_path = Path(args.ase).expanduser().resolve()
    text = ase_path.read_bytes().decode("latin-1")

    geom = None

    for _, _, block in find_braced_blocks(text, "GEOMOBJECT"):
        if first_quoted(block, "*NODE_NAME") == args.object:
            geom = block
            break

    if geom is None:
        raise RuntimeError(
            f"ASE GEOMOBJECT {args.object!r} not found"
        )

    obj = bpy.data.objects.get(args.object)

    if obj is None or obj.type != "MESH":
        raise RuntimeError(
            f"Blender mesh {args.object!r} not found"
        )

    mesh_block = first_mesh_block(geom)

    if mesh_block is None:
        raise RuntimeError("MESH block not found")

    ase_face, ase_corner = parse_normals(mesh_block)

    mesh = obj.data

    # Ensure derived normal data is current.
    mesh.update()

    print("=" * 80)
    print("PT V5b NORMALS PROBE")
    print("=" * 80)
    print("ASE    :", ase_path)
    print("Object :", args.object)
    print()
    print("Blender:")
    print("  vertices:", len(mesh.vertices))
    print("  polygons:", len(mesh.polygons))
    print("  loops   :", len(mesh.loops))
    print("  has corner_normals:", hasattr(mesh, "corner_normals"))

    corner_api = getattr(mesh, "corner_normals", None)

    if corner_api is not None:
        try:
            print("  corner_normals len:", len(corner_api))
        except Exception as e:
            print("  corner_normals len ERROR:", repr(e))

    print()
    print("ASE normals:")
    print("  face normals  :", len(ase_face))
    print("  corner normals:", len(ase_corner))

    if len(ase_face) != len(mesh.polygons):
        raise RuntimeError(
            f"Face-normal count mismatch: "
            f"ASE={len(ase_face)} Blender polygons={len(mesh.polygons)}"
        )

    if len(ase_corner) != len(mesh.loops):
        raise RuntimeError(
            f"Corner-normal count mismatch: "
            f"ASE={len(ase_corner)} Blender loops={len(mesh.loops)}"
        )

    max_face_delta = -1.0
    max_face_info = None
    face_sum = 0.0

    max_corner_delta = -1.0
    max_corner_info = None
    corner_sum = 0.0

    vertex_index_mismatch = 0
    face_count = 0
    corner_count = 0

    shown = 0

    for poly in mesh.polygons:
        face_index = int(poly.index)

        if face_index not in ase_face:
            raise RuntimeError(
                f"ASE FACENORMAL missing face {face_index}"
            )

        a_fn = ase_face[face_index]
        b_fn = vec_tuple(poly.normal)

        d = delta(a_fn, b_fn)
        face_sum += d
        face_count += 1

        if d > max_face_delta:
            max_face_delta = d
            max_face_info = (face_index, a_fn, b_fn, dot(a_fn, b_fn))

        if shown < args.show:
            print()
            print(
                f"FACE {face_index}: "
                f"ASE={a_fn} Blender={b_fn} "
                f"delta={d:.9g} dot={dot(a_fn,b_fn):.9g}"
            )

        for corner, loop_index in enumerate(poly.loop_indices):
            key = (face_index, corner)

            if key not in ase_corner:
                raise RuntimeError(
                    f"ASE VERTEXNORMAL missing face={face_index} "
                    f"corner={corner}"
                )

            entry = ase_corner[key]
            blender_vertex_index = int(mesh.loops[loop_index].vertex_index)

            if entry["vertex_index"] != blender_vertex_index:
                vertex_index_mismatch += 1

            a_cn = entry["normal"]

            if corner_api is not None:
                try:
                    item = corner_api[loop_index]
                    b_cn = vec_tuple(item.vector)
                except Exception as e:
                    raise RuntimeError(
                        "Blender corner_normals exists but could not "
                        f"read loop {loop_index}: {e!r}"
                    )
            else:
                # Fallback: loop.normal existed in older Blender versions.
                loop = mesh.loops[loop_index]
                if not hasattr(loop, "normal"):
                    raise RuntimeError(
                        "No Blender corner-normal API found"
                    )
                b_cn = vec_tuple(loop.normal)

            cd = delta(a_cn, b_cn)
            corner_sum += cd
            corner_count += 1

            if cd > max_corner_delta:
                max_corner_delta = cd
                max_corner_info = (
                    face_index,
                    corner,
                    entry["vertex_index"],
                    blender_vertex_index,
                    a_cn,
                    b_cn,
                    dot(a_cn, b_cn),
                )

            if shown < args.show:
                print(
                    f"  CORNER {corner} "
                    f"vertex ASE={entry['vertex_index']} "
                    f"Blender={blender_vertex_index} "
                    f"delta={cd:.9g} "
                    f"dot={dot(a_cn,b_cn):.9g}"
                )

        if shown < args.show:
            shown += 1

    print()
    print("=" * 80)
    print("RESULT")
    print("=" * 80)

    print("Face index count              :", face_count)
    print("Corner index count            :", corner_count)
    print("Corner vertex-index mismatch  :", vertex_index_mismatch)

    print()
    print("Face normal average delta     :", face_sum / max(face_count, 1))
    print("Face normal max delta         :", max_face_delta)
    print("Face max detail               :", max_face_info)

    print()
    print("Corner normal average delta   :", corner_sum / max(corner_count, 1))
    print("Corner normal max delta       :", max_corner_delta)
    print("Corner max detail             :", max_corner_info)

    print()
    if vertex_index_mismatch == 0:
        print("ORDER: FACE/CORNER/VERTEX INDEX MAPPING MATCHES")
    else:
        print("ORDER: MISMATCH - DO NOT EXPORT NORMALS YET")

    # Loose diagnostic thresholds. We do not call this "proved" automatically;
    # user/runtime review still controls the project evidence label.
    if (
        vertex_index_mismatch == 0
        and max_face_delta < 1e-4
        and max_corner_delta < 1e-4
    ):
        print("NORMAL VALUES: DIRECT BLENDER MATCH")
    elif (
        vertex_index_mismatch == 0
        and max_face_info
        and max_face_info[3] < -0.99
    ):
        print("NORMAL VALUES: POSSIBLE GLOBAL SIGN/HANDEDNESS INVERSION")
    else:
        print("NORMAL VALUES: DIFFER - INVESTIGATE SMOOTHING/CUSTOM NORMALS")

    print("=" * 80)


if __name__ == "__main__":
    main()

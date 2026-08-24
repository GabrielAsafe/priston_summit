#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
Create a controlled V5b.1 one-TVERT Blender test.

This helper:
- opens the already-loaded V4 .blend;
- parses the original ASE;
- finds one referenced TVERT in the requested GEOMOBJECT;
- changes U by a small delta for every Blender loop that maps to that TVERT;
- saves a new .blend;
- does not change geometry, materials or Physique.
"""

import argparse
import re
import sys
from pathlib import Path

import bpy


def find_braced_blocks(text, token):
    rx = re.compile(
        rf"(?im)^[ \t]*\*{re.escape(token)}[ \t]*\{{"
    )
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
                        out.append(
                            (m.start(), i + 1, text[m.start():i + 1])
                        )
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


def parse_tverts(mesh_block):
    out = {}

    rx = re.compile(
        r"\*MESH_TVERT\s+(\d+)\s+"
        r"([-+0-9.eE]+)\s+([-+0-9.eE]+)\s+([-+0-9.eE]+)",
        flags=re.IGNORECASE,
    )

    for m in rx.finditer(mesh_block):
        out[int(m.group(1))] = (
            float(m.group(2)),
            float(m.group(3)),
            float(m.group(4)),
        )

    return out


def parse_tfaces(mesh_block):
    out = {}

    rx = re.compile(
        r"\*MESH_TFACE\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)",
        flags=re.IGNORECASE,
    )

    for m in rx.finditer(mesh_block):
        out[int(m.group(1))] = (
            int(m.group(2)),
            int(m.group(3)),
            int(m.group(4)),
        )

    return out


def parse_args():
    argv = sys.argv

    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    p = argparse.ArgumentParser()
    p.add_argument("--ase", required=True)
    p.add_argument("--object", default="MahC20")
    p.add_argument("--delta-u", type=float, default=0.01)
    p.add_argument("--out-blend", required=True)
    return p.parse_args(argv)


def main():
    args = parse_args()

    ase_path = Path(args.ase).expanduser().resolve()
    out_blend = Path(args.out_blend).expanduser().resolve()

    text = ase_path.read_bytes().decode("latin-1")

    geom = None

    for _, _, block in find_braced_blocks(text, "GEOMOBJECT"):
        if first_quoted(block, "*NODE_NAME") == args.object:
            geom = block
            break

    if geom is None:
        raise RuntimeError(
            f"GEOMOBJECT {args.object!r} not found in ASE"
        )

    mesh_block = first_mesh_block(geom)

    if mesh_block is None:
        raise RuntimeError(
            f"{args.object}: MESH block not found"
        )

    tverts = parse_tverts(mesh_block)
    tfaces = parse_tfaces(mesh_block)

    obj = bpy.data.objects.get(args.object)

    if obj is None or obj.type != "MESH":
        raise RuntimeError(
            f"Blender mesh {args.object!r} not found"
        )

    uv_layer = obj.data.uv_layers.get("ASE_UV") or obj.data.uv_layers.active

    if uv_layer is None:
        raise RuntimeError(
            f"{args.object}: no UV layer found"
        )

    # Find the first TVERT actually referenced by at least one TFACE.
    refs = {}

    for face_index, tf in sorted(tfaces.items()):
        for corner, tvert_index in enumerate(tf):
            refs.setdefault(tvert_index, []).append(
                (face_index, corner)
            )

    candidates = [
        tvert_index
        for tvert_index in sorted(refs)
        if tvert_index in tverts
    ]

    if not candidates:
        raise RuntimeError(
            f"{args.object}: no referenced TVERT found"
        )

    target = candidates[0]
    references = refs[target]

    # Map ASE face/corner to Blender loop.
    changed_loops = []

    for face_index, corner in references:
        if face_index < 0 or face_index >= len(obj.data.polygons):
            raise RuntimeError(
                f"Face {face_index} outside Blender polygon range"
            )

        poly = obj.data.polygons[face_index]

        if len(poly.loop_indices) != 3:
            raise RuntimeError(
                f"Face {face_index} is not a triangle"
            )

        loop_index = poly.loop_indices[corner]
        uv = uv_layer.data[loop_index].uv

        before = (float(uv.x), float(uv.y))

        uv.x += args.delta_u

        after = (float(uv.x), float(uv.y))

        changed_loops.append(
            (face_index, corner, loop_index, before, after)
        )

    out_blend.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    bpy.ops.wm.save_as_mainfile(
        filepath=str(out_blend)
    )

    print("=" * 80)
    print("PT V5b.1 ONE TVERT TEST")
    print("=" * 80)
    print("ASE       :", ase_path)
    print("Object    :", args.object)
    print("TVERT     :", target)
    print("ASE UV    :", tverts[target])
    print("References:", len(references))
    print("Delta U   :", args.delta_u)
    print()
    print("Changed Blender loops:")

    for item in changed_loops:
        face_index, corner, loop_index, before, after = item
        print(
            f"  face={face_index} corner={corner} loop={loop_index} "
            f"{before} -> {after}"
        )

    print()
    print("Saved:", out_blend)
    print("=" * 80)


if __name__ == "__main__":
    main()

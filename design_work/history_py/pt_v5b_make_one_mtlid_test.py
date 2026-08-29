#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
Create a controlled V5b.2 one-MESH_MTLID Blender test.

The helper:
- uses the currently loaded V4 .blend;
- targets one imported mesh (default MbhC19);
- finds one polygon whose current material slot can be switched to another
  ASE submaterial slot from the same MATERIAL_REF;
- changes exactly one polygon.material_index;
- saves a new .blend;
- does not modify geometry, UV or Physique.
"""

import argparse
import re
import sys
from pathlib import Path

import bpy


ASE_MAT_RE = re.compile(
    r"^ASE_M(-?\d+)_S(-?\d+)_",
    flags=re.IGNORECASE,
)


def parse_args():
    argv = sys.argv

    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    p = argparse.ArgumentParser()
    p.add_argument("--object", default="MbhC19")
    p.add_argument("--out-blend", required=True)
    return p.parse_args(argv)


def slot_info(obj):
    out = {}

    for slot_index, mat in enumerate(obj.data.materials):
        if mat is None:
            continue

        m = ASE_MAT_RE.match(mat.name)

        if not m:
            continue

        material_ref = int(m.group(1))
        mtl_id = int(m.group(2))

        out[slot_index] = {
            "material_ref": material_ref,
            "mtl_id": mtl_id,
            "name": mat.name,
        }

    return out


def main():
    args = parse_args()

    obj = bpy.data.objects.get(args.object)

    if obj is None or obj.type != "MESH":
        raise RuntimeError(
            f"Blender mesh {args.object!r} not found"
        )

    slots = slot_info(obj)

    if len(slots) < 2:
        raise RuntimeError(
            f"{args.object}: need at least two ASE material slots; "
            f"found {slots}"
        )

    chosen = None

    for poly in obj.data.polygons:
        old_slot = int(poly.material_index)

        if old_slot not in slots:
            continue

        old_info = slots[old_slot]

        alternatives = [
            (slot_idx, info)
            for slot_idx, info in sorted(slots.items())
            if slot_idx != old_slot
            and info["material_ref"] == old_info["material_ref"]
            and info["mtl_id"] != old_info["mtl_id"]
        ]

        if alternatives:
            new_slot, new_info = alternatives[0]
            chosen = (
                poly,
                old_slot,
                old_info,
                new_slot,
                new_info,
            )
            break

    if chosen is None:
        raise RuntimeError(
            f"{args.object}: no polygon with an alternate ASE submaterial "
            f"slot from the same MATERIAL_REF was found. Slots={slots}"
        )

    poly, old_slot, old_info, new_slot, new_info = chosen

    face_index = int(poly.index)

    print("=" * 80)
    print("PT V5b.2 ONE MESH_MTLID TEST")
    print("=" * 80)
    print("Object        :", obj.name)
    print("Face          :", face_index)
    print("Old slot      :", old_slot)
    print("Old material  :", old_info["name"])
    print("Old MATERIAL_REF:", old_info["material_ref"])
    print("Old MTLID     :", old_info["mtl_id"])
    print("New slot      :", new_slot)
    print("New material  :", new_info["name"])
    print("New MATERIAL_REF:", new_info["material_ref"])
    print("New MTLID     :", new_info["mtl_id"])
    print()

    poly.material_index = new_slot

    out_blend = Path(args.out_blend).expanduser().resolve()
    out_blend.parent.mkdir(parents=True, exist_ok=True)

    bpy.ops.wm.save_as_mainfile(
        filepath=str(out_blend)
    )

    print("Saved:", out_blend)
    print("=" * 80)


if __name__ == "__main__":
    main()

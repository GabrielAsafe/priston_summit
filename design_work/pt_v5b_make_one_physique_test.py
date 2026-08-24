#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
Create a controlled V5b.2 one-PHYSIQUE-assignment Blender test.

The helper:
- uses the currently loaded V4 .blend;
- targets one imported mesh (default MbhC19);
- finds one vertex with exactly one active Bip* group, weight 1.0;
- moves that single vertex to another existing Bip* group on the same object;
- keeps the new assignment rigid (weight 1.0);
- saves a new .blend;
- does not modify geometry, UV or material assignment.
"""

import argparse
import sys
from pathlib import Path

import bpy


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


def active_bip_groups_for_vertex(obj, vertex_index, eps=1e-6):
    vertex = obj.data.vertices[vertex_index]
    active = []

    for membership in vertex.groups:
        group = obj.vertex_groups[membership.group]

        if not group.name.lower().startswith("bip"):
            continue

        if membership.weight <= eps:
            continue

        active.append(
            (group.name, float(membership.weight))
        )

    return active


def main():
    args = parse_args()

    obj = bpy.data.objects.get(args.object)

    if obj is None or obj.type != "MESH":
        raise RuntimeError(
            f"Blender mesh {args.object!r} not found"
        )

    # Existing Bip groups that actually belong to the armature, when available.
    armature = None

    for modifier in obj.modifiers:
        if modifier.type == "ARMATURE" and modifier.object is not None:
            armature = modifier.object
            break

    armature_bones = set()

    if armature is not None and armature.type == "ARMATURE":
        armature_bones = set(armature.data.bones.keys())

    candidate_groups = []

    for group in obj.vertex_groups:
        if not group.name.lower().startswith("bip"):
            continue

        if armature_bones and group.name not in armature_bones:
            continue

        candidate_groups.append(group.name)

    if len(candidate_groups) < 2:
        raise RuntimeError(
            f"{args.object}: need at least two valid Bip* groups; "
            f"found {candidate_groups}"
        )

    chosen = None

    for vertex in obj.data.vertices:
        vertex_index = int(vertex.index)
        active = active_bip_groups_for_vertex(
            obj,
            vertex_index,
        )

        if len(active) != 1:
            continue

        old_bone, old_weight = active[0]

        if abs(old_weight - 1.0) > 1e-5:
            continue

        alternatives = [
            name
            for name in candidate_groups
            if name != old_bone
        ]

        if not alternatives:
            continue

        # Prefer a group already used by at least one other vertex.
        used_alternatives = []

        for name in alternatives:
            group = obj.vertex_groups.get(name)
            used = False

            if group is None:
                continue

            for other in obj.data.vertices:
                if other.index == vertex_index:
                    continue

                try:
                    weight = group.weight(other.index)
                except RuntimeError:
                    continue

                if weight > 1e-6:
                    used = True
                    break

            if used:
                used_alternatives.append(name)

        new_bone = (
            used_alternatives[0]
            if used_alternatives
            else alternatives[0]
        )

        chosen = (
            vertex_index,
            old_bone,
            new_bone,
        )
        break

    if chosen is None:
        raise RuntimeError(
            f"{args.object}: no suitable rigid vertex/bone reassignment found"
        )

    vertex_index, old_bone, new_bone = chosen

    old_group = obj.vertex_groups.get(old_bone)
    new_group = obj.vertex_groups.get(new_bone)

    if old_group is None or new_group is None:
        raise RuntimeError(
            "Internal error resolving old/new vertex groups"
        )

    print("=" * 80)
    print("PT V5b.2 ONE PHYSIQUE TEST")
    print("=" * 80)
    print("Object     :", obj.name)
    print("Vertex     :", vertex_index)
    print("Old bone   :", old_bone)
    print("Old weight :", old_group.weight(vertex_index))
    print("New bone   :", new_bone)
    print()

    # Remove the selected vertex from every active Bip group first.
    # This ensures the result is exactly one rigid Bip assignment.
    for group in obj.vertex_groups:
        if not group.name.lower().startswith("bip"):
            continue

        try:
            weight = group.weight(vertex_index)
        except RuntimeError:
            continue

        if weight > 1e-6:
            group.remove([vertex_index])

    new_group.add(
        [vertex_index],
        1.0,
        "REPLACE",
    )

    final = active_bip_groups_for_vertex(
        obj,
        vertex_index,
    )

    print("Final groups:", final)

    if len(final) != 1:
        raise RuntimeError(
            f"Expected exactly one active Bip group after edit; got {final}"
        )

    if final[0][0] != new_bone or abs(final[0][1] - 1.0) > 1e-5:
        raise RuntimeError(
            f"Rigid reassignment failed: {final}"
        )

    out_blend = Path(
        args.out_blend
    ).expanduser().resolve()

    out_blend.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    bpy.ops.wm.save_as_mainfile(
        filepath=str(out_blend)
    )

    print("Saved:", out_blend)
    print("=" * 80)


if __name__ == "__main__":
    main()

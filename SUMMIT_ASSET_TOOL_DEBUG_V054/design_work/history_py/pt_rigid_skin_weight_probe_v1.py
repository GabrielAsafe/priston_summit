#!/usr/bin/env python3
"""
PT RIGID SKIN WEIGHT PROBE V1
Read-only analysis of Blender vertex-group weights for MshC01.

Purpose:
- quantify how Subdivide interpolated rigid Physique groups;
- identify mixed-weight patterns and exact ties;
- show which vertices are already valid rigid seeds;
- estimate a deterministic nearest-rigid-neighbor fallback for ties.

Does not modify or save the .blend.
"""

import bpy
import math
from collections import Counter, defaultdict
from mathutils import Vector

TARGET = "MshC01"
EPS = 1e-6


def sync_object_mode(obj):
    original = obj.mode
    changed = False
    if obj.mode == "EDIT":
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        result = bpy.ops.object.mode_set(mode="OBJECT")
        if "FINISHED" not in result:
            raise RuntimeError(f"mode_set failed: {result}")
        changed = True
    obj.data.update()
    return original, changed


def weights_for_vertex(obj, vi):
    rows = []
    for e in obj.data.vertices[vi].groups:
        if e.weight <= 1e-8:
            continue
        name = obj.vertex_groups[e.group].name
        rows.append((name, float(e.weight)))
    rows.sort(key=lambda x: (-x[1], x[0]))
    return rows


def is_rigid(rows):
    return len(rows) == 1 and abs(rows[0][1] - 1.0) <= 1e-5


def main():
    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    original_mode, synced = sync_object_mode(obj)
    mesh = obj.data

    valid = []
    mixed = []
    empty = []
    weight_count_hist = Counter()
    pair_hist = Counter()
    tie_hist = Counter()
    top_bone_hist = Counter()

    rigid_seeds_by_bone = defaultdict(list)

    for vi, v in enumerate(mesh.vertices):
        rows = weights_for_vertex(obj, vi)
        weight_count_hist[len(rows)] += 1

        if not rows:
            empty.append(vi)
            continue

        if is_rigid(rows):
            valid.append(vi)
            rigid_seeds_by_bone[rows[0][0]].append(vi)
            continue

        mixed.append((vi, rows))
        top_bone_hist[rows[0][0]] += 1

        if len(rows) >= 2:
            pair = tuple(sorted((rows[0][0], rows[1][0])))
            pair_hist[pair] += 1
            if abs(rows[0][1] - rows[1][1]) <= EPS:
                tie_hist[pair] += 1

    # Build a deterministic local-neighbor suggestion for mixed vertices.
    # Only uses directly connected vertices that are already strict rigid seeds.
    adjacency = [[] for _ in mesh.vertices]
    for e in mesh.edges:
        a, b = map(int, e.vertices)
        adjacency[a].append(b)
        adjacency[b].append(a)

    suggestions = []
    suggestion_policy_hist = Counter()

    for vi, rows in mixed:
        maxw = rows[0][1]
        top = [name for name, w in rows if abs(w - maxw) <= EPS]

        if len(top) == 1:
            chosen = top[0]
            policy = "UNIQUE_MAX_WEIGHT"
        else:
            # Tie: inspect directly adjacent strict-rigid seeds restricted to tied bones.
            score = Counter()
            p = Vector(mesh.vertices[vi].co)
            distance_score = defaultdict(float)

            for nb in adjacency[vi]:
                nb_rows = weights_for_vertex(obj, nb)
                if not is_rigid(nb_rows):
                    continue
                bone = nb_rows[0][0]
                if bone not in top:
                    continue
                score[bone] += 1
                d = (Vector(mesh.vertices[nb].co) - p).length
                distance_score[bone] += 1.0 / max(d, 1e-9)

            if score:
                best_count = max(score.values())
                candidates = sorted([b for b, c in score.items() if c == best_count])
                if len(candidates) == 1:
                    chosen = candidates[0]
                    policy = "TIE_ADJACENT_RIGID_MAJORITY"
                else:
                    best_ds = max(distance_score[b] for b in candidates)
                    ds_candidates = sorted(
                        [b for b in candidates if abs(distance_score[b] - best_ds) <= 1e-12]
                    )
                    if len(ds_candidates) == 1:
                        chosen = ds_candidates[0]
                        policy = "TIE_ADJACENT_RIGID_DISTANCE"
                    else:
                        chosen = ds_candidates[0]
                        policy = "TIE_STILL_AMBIGUOUS_LEXICAL"
            else:
                chosen = sorted(top)[0]
                policy = "TIE_NO_RIGID_NEIGHBOR_LEXICAL"

        suggestion_policy_hist[policy] += 1
        if len(suggestions) < 80:
            suggestions.append({
                "vertex": vi,
                "weights": rows,
                "suggested_bone": chosen,
                "policy": policy,
                "neighbors": adjacency[vi],
            })

    print("PT_RIGID_SKIN_WEIGHT_PROBE_V1")
    print(f"original_object_mode={original_mode}")
    print(f"edit_mode_synchronized={synced}")
    print(f"vertices={len(mesh.vertices)}")
    print(f"edges={len(mesh.edges)}")
    print(f"polygons={len(mesh.polygons)}")
    print(f"strict_rigid_vertices={len(valid)}")
    print(f"mixed_vertices={len(mixed)}")
    print(f"empty_group_vertices={len(empty)}")
    print(f"weight_group_count_histogram={dict(sorted(weight_count_hist.items()))}")
    print(f"rigid_seed_bones={len(rigid_seeds_by_bone)}")
    print(f"rigid_seed_histogram={dict(sorted((k,len(v)) for k,v in rigid_seeds_by_bone.items()))}")
    print(f"top_bone_histogram_for_mixed={dict(top_bone_hist)}")
    print(f"top_two_pair_histogram={dict(pair_hist)}")
    print(f"exact_tie_pair_histogram={dict(tie_hist)}")
    print(f"resolution_policy_histogram={dict(suggestion_policy_hist)}")
    print(f"empty_vertex_sample={empty[:40]}")
    print(f"mixed_sample={mixed[:30]}")
    print(f"resolution_sample={suggestions[:30]}")
    print("rigid_skin_weight_probe_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"rigid_skin_weight_probe_check=FAIL: {type(exc).__name__}: {exc}")
        raise

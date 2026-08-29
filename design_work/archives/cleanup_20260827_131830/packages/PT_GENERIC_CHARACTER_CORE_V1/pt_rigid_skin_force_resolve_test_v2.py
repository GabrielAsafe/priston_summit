#!/usr/bin/env python3
"""
PT RIGID SKIN FORCE RESOLVE TEST V2

TEST-ONLY helper for Summit/Priston Tale rigid Physique experiments.

Input:
  A game-normalized .blend produced by PT_RIGID_SKIN_NORMALIZE_COPY_V1,
  containing ambiguous vertices marked in PT_REVIEW_REQUIRED.

Output:
  A NEW .blend in which every PT_REVIEW_REQUIRED vertex is forced to exactly
  one rigid bone.

Heuristic:
  - Candidate bones are ONLY the non-review vertex groups already present on
    that ambiguous vertex.
  - Freeze all currently strict-rigid vertices as seeds before resolving.
  - For each candidate bone, find the nearest strict-rigid seed vertex assigned
    to that bone.
  - Choose the candidate whose nearest rigid seed is geometrically closest.
  - Deterministic lexical tie-break only if distances are exactly equal.

This is intentionally NOT a production artistic policy. It exists only to
close a runtime/export gate without requiring manual rigging.

The input .blend is never overwritten.
No SMD/client file is touched.
"""

import argparse
import hashlib
import json
import math
import sys
from collections import Counter, defaultdict
from pathlib import Path

import bpy
from mathutils import Vector

DEFAULT_TARGET = "MshC01"
REVIEW_GROUP_DEFAULT = "PT_REVIEW_REQUIRED"
EPS = 1e-12


def file_sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def sync_object_mode(obj):
    original_mode = obj.mode
    synchronized = False

    if obj.mode == "EDIT":
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        result = bpy.ops.object.mode_set(mode="OBJECT")
        if "FINISHED" not in result or obj.mode != "OBJECT":
            raise RuntimeError(
                f"failed to synchronize Edit Mode mesh: result={result}, "
                f"mode={obj.mode}"
            )
        synchronized = True

    obj.data.update()
    return {
        "original_mode": original_mode,
        "current_mode": obj.mode,
        "edit_mode_synchronized": synchronized,
    }


def rows_for_vertex(obj, vi, review_group):
    rows = []
    has_review = False

    for e in obj.data.vertices[vi].groups:
        if not 0 <= e.group < len(obj.vertex_groups):
            raise RuntimeError(f"vertex {vi}: invalid vertex-group index {e.group}")

        name = obj.vertex_groups[e.group].name
        weight = float(e.weight)

        if name == review_group:
            if weight > 1e-8:
                has_review = True
            continue

        if weight > 1e-8:
            rows.append((name, weight))

    rows.sort(key=lambda x: (-x[1], x[0]))
    return rows, has_review


def is_strict_rigid(rows):
    return len(rows) == 1 and abs(rows[0][1] - 1.0) <= 1e-5


def set_exact_rigid(obj, vi, bone_name, review_group):
    current_names = []
    for e in list(obj.data.vertices[vi].groups):
        if 0 <= e.group < len(obj.vertex_groups):
            current_names.append(obj.vertex_groups[e.group].name)

    for name in current_names:
        group = obj.vertex_groups.get(name)
        if group is not None:
            try:
                group.remove([vi])
            except RuntimeError:
                pass

    bone_group = obj.vertex_groups.get(bone_name)
    if bone_group is None:
        raise RuntimeError(
            f"candidate bone group {bone_name!r} does not exist on the selected mesh"
        )

    bone_group.add([vi], 1.0, "REPLACE")

    review = obj.vertex_groups.get(review_group)
    if review is not None:
        try:
            review.remove([vi])
        except RuntimeError:
            pass


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default=DEFAULT_TARGET)
    p.add_argument("--review-group", default=REVIEW_GROUP_DEFAULT)
    p.add_argument("--out", required=True)
    p.add_argument("--report", required=True)
    p.add_argument("--overwrite", action="store_true")
    return p.parse_args(argv)


def main(argv):
    args = parse_args(argv)

    source_blend = Path(bpy.data.filepath).resolve()
    if not source_blend.exists():
        raise RuntimeError("loaded .blend does not have a valid source filepath")

    out = Path(args.out).expanduser().resolve()
    report_path = Path(args.report).expanduser().resolve()

    if out == source_blend:
        raise RuntimeError(
            "refusing to overwrite input normalized .blend; choose a different --out"
        )
    if out.exists() and not args.overwrite:
        raise RuntimeError(
            f"output already exists: {out}; use --overwrite intentionally"
        )

    target = args.object
    obj = bpy.data.objects.get(target)
    if obj is None or obj.type != "MESH":
        raise RuntimeError(f"{target}: Blender mesh not found")

    mode_sync = sync_object_mode(obj)
    mesh = obj.data
    mesh.update()

    review_group = obj.vertex_groups.get(args.review_group)
    if review_group is None:
        raise RuntimeError(
            f"review group {args.review_group!r} not found; nothing to force-resolve"
        )

    # Freeze state before resolving anything.
    state = []
    rigid_seeds_by_bone = defaultdict(list)
    ambiguous = []

    for vi, v in enumerate(mesh.vertices):
        rows, has_review = rows_for_vertex(obj, vi, args.review_group)
        state.append((rows, has_review))

        if is_strict_rigid(rows) and not has_review:
            rigid_seeds_by_bone[rows[0][0]].append(vi)

        if has_review:
            ambiguous.append(vi)

    if not ambiguous:
        raise RuntimeError("review group has no marked vertices")

    if not rigid_seeds_by_bone:
        raise RuntimeError("no strict-rigid seed vertices available")

    seed_positions = {
        bone: [(vi, Vector(mesh.vertices[vi].co)) for vi in vis]
        for bone, vis in rigid_seeds_by_bone.items()
    }

    resolutions = []
    policy_hist = Counter()
    chosen_hist = Counter()

    for vi in ambiguous:
        rows, has_review = state[vi]
        if not has_review:
            raise RuntimeError(f"internal error: vertex {vi} lost review marker")
        if not rows:
            raise RuntimeError(
                f"vertex {vi}: review-marked but has no candidate bone weights"
            )

        candidate_bones = sorted({name for name, _weight in rows})
        pos = Vector(mesh.vertices[vi].co)

        distances = {}
        nearest_seed_vertex = {}

        for bone in candidate_bones:
            seeds = seed_positions.get(bone, [])
            if not seeds:
                continue

            best_d2 = None
            best_vi = None
            for seed_vi, seed_pos in seeds:
                d2 = (seed_pos - pos).length_squared
                if (
                    best_d2 is None
                    or d2 < best_d2 - EPS
                    or (abs(d2 - best_d2) <= EPS and seed_vi < best_vi)
                ):
                    best_d2 = d2
                    best_vi = seed_vi

            if best_d2 is not None:
                distances[bone] = math.sqrt(best_d2)
                nearest_seed_vertex[bone] = best_vi

        if distances:
            best_distance = min(distances.values())
            winners = sorted(
                bone for bone, d in distances.items()
                if abs(d - best_distance) <= EPS
            )
            chosen = winners[0]
            policy = (
                "NEAREST_RIGID_SEED"
                if len(winners) == 1
                else "NEAREST_RIGID_SEED_DISTANCE_TIE_LEXICAL"
            )
        else:
            # Extremely defensive fallback: candidate bones exist on the
            # vertex, but none has a strict seed in this mesh.
            # For test-only purposes, take unique max weight if possible;
            # otherwise lexical among top-weight candidates.
            maxw = max(weight for _name, weight in rows)
            top = sorted(
                name for name, weight in rows
                if abs(weight - maxw) <= 1e-6
            )
            chosen = top[0]
            policy = (
                "NO_SEED_UNIQUE_MAX_WEIGHT"
                if len(top) == 1
                else "NO_SEED_TOP_WEIGHT_TIE_LEXICAL"
            )

        set_exact_rigid(obj, vi, chosen, args.review_group)

        policy_hist[policy] += 1
        chosen_hist[chosen] += 1

        resolutions.append({
            "vertex": vi,
            "weights_before": rows,
            "candidate_bones": candidate_bones,
            "candidate_nearest_seed_distance": distances,
            "candidate_nearest_seed_vertex": nearest_seed_vertex,
            "chosen_bone": chosen,
            "policy": policy,
        })

    mesh.update()

    # Remove review group entirely once no vertex uses it.
    review = obj.vertex_groups.get(args.review_group)
    if review is not None:
        any_review = False
        for vi in range(len(mesh.vertices)):
            _rows, has_review = rows_for_vertex(obj, vi, args.review_group)
            if has_review:
                any_review = True
                break
        if not any_review:
            obj.vertex_groups.remove(review)

    # Final strict-rigid verification.
    invalid = []
    final_bone_hist = Counter()

    for vi in range(len(mesh.vertices)):
        rows, has_review = rows_for_vertex(obj, vi, args.review_group)
        if has_review or not is_strict_rigid(rows):
            invalid.append({
                "vertex": vi,
                "weights": rows,
                "review_marker": has_review,
            })
        elif rows:
            final_bone_hist[rows[0][0]] += 1

    if invalid:
        raise RuntimeError(
            f"final strict-rigid verification failed: "
            f"count={len(invalid)} sample={invalid[:20]}"
        )

    source_sha = file_sha256(source_blend)

    out.parent.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)

    report = {
        "tool": "PT_RIGID_SKIN_FORCE_RESOLVE_TEST_V2",
        "test_only": True,
        "production_policy": False,
        "source_blend": str(source_blend),
        "source_blend_sha256": source_sha,
        "output_blend": str(out),
        "object": target,
        "mode_sync": mode_sync,
        "vertices": len(mesh.vertices),
        "review_vertices_before": len(ambiguous),
        "forced_resolved": len(resolutions),
        "invalid_after": len(invalid),
        "review_group_removed": obj.vertex_groups.get(args.review_group) is None,
        "policy_histogram": dict(policy_hist),
        "chosen_bone_histogram": dict(chosen_hist),
        "final_bone_histogram": dict(final_bone_hist),
        "resolutions": resolutions,
        "message": (
            "TEST ONLY: ambiguous Blender weights were forced to one rigid bone "
            "using the nearest existing strict-rigid seed among that vertex's "
            "candidate bones. This is not an approved production artistic rule."
        ),
    }

    report_path.write_text(
        json.dumps(report, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    bpy.ops.wm.save_as_mainfile(filepath=str(out), check_existing=False)

    print("PT_RIGID_SKIN_FORCE_RESOLVE_TEST_V2")
    print(f"object={target}")
    print("test_only=True")
    print("production_policy=False")
    print(f"source_blend={source_blend}")
    print(f"source_blend_sha256={source_sha}")
    print(f"output_blend={out}")
    print(f"vertices={len(mesh.vertices)}")
    print(f"review_vertices_before={len(ambiguous)}")
    print(f"forced_resolved={len(resolutions)}")
    print(f"invalid_after={len(invalid)}")
    print(f"review_group_removed={obj.vertex_groups.get(args.review_group) is None}")
    print(f"policy_histogram={dict(policy_hist)}")
    print(f"chosen_bone_histogram={dict(chosen_hist)}")
    print(f"final_rigid_bone_count={len(final_bone_hist)}")
    print(f"report={report_path}")
    print("force_resolve_test_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(
            f"force_resolve_test_check=FAIL: "
            f"{type(exc).__name__}: {exc}"
        )
        raise

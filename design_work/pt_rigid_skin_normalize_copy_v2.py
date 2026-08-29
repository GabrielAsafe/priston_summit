#!/usr/bin/env python3
"""
PT RIGID SKIN NORMALIZE COPY V2

Creates a GAME-NORMALIZED COPY of an artist Blender file.

Principles:
- The artist .blend loaded by Blender is NEVER overwritten.
- The output is a separate .blend.
- Existing strict rigid vertices are preserved.
- Mixed vertices are auto-rigidized only when the decision is defensible:
    1) unique maximum Blender weight;
    2) exact top-weight tie resolved by adjacent original rigid majority;
    3) remaining adjacent-rigid tie resolved by inverse-distance score.
- Ambiguous vertices are NOT guessed.
- Ambiguous vertices keep their artist weights and are additionally placed in
  a special vertex group PT_REVIEW_REQUIRED so the artist can select them.
- Because PT_REVIEW_REQUIRED is not a skeleton bone, the normal validator /
  exporter will continue to reject the copy until the artist reviews those
  vertices and removes the marker group. This is intentional.

The Priston/Summit runtime Physique model used by this pipeline is rigid:
exactly one bone per vertex. The exporter later recomputes the proper bone-local
coordinates for whatever rigid bone is finally assigned.

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
EPS = 1e-6
REVIEW_GROUP_DEFAULT = "PT_REVIEW_REQUIRED"


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


def weights_for_vertex(obj, vi, ignore_group=None):
    rows = []
    for e in obj.data.vertices[vi].groups:
        if e.weight <= 1e-8:
            continue
        if not 0 <= e.group < len(obj.vertex_groups):
            raise RuntimeError(f"vertex {vi}: invalid vertex-group index {e.group}")
        name = obj.vertex_groups[e.group].name
        if ignore_group is not None and name == ignore_group:
            continue
        rows.append((name, float(e.weight)))
    rows.sort(key=lambda x: (-x[1], x[0]))
    return rows


def is_strict_rigid(rows):
    return len(rows) == 1 and abs(rows[0][1] - 1.0) <= 1e-5


def adjacency_for_mesh(mesh):
    adjacency = [[] for _ in mesh.vertices]
    for edge in mesh.edges:
        a, b = map(int, edge.vertices)
        adjacency[a].append(b)
        adjacency[b].append(a)
    return adjacency


def resolve_vertex(vi, rows, mesh, adjacency, initial_rows):
    """
    Returns (chosen_bone|None, policy, details).
    Uses ONLY strict-rigid seeds from the original artist state so the result
    is deterministic and does not depend on iteration order.
    """
    if not rows:
        return None, "EMPTY_GROUPS", {}

    if is_strict_rigid(rows):
        return rows[0][0], "ALREADY_RIGID", {}

    maxw = rows[0][1]
    top = [name for name, weight in rows if abs(weight - maxw) <= EPS]

    if len(top) == 1:
        return top[0], "UNIQUE_MAX_WEIGHT", {
            "top_weight": maxw,
        }

    # Exact/near tie among maximum-weight candidate bones.
    neighbor_count = Counter()
    inverse_distance = defaultdict(float)
    pos = Vector(mesh.vertices[vi].co)

    for nb in adjacency[vi]:
        nb_rows = initial_rows[nb]
        if not is_strict_rigid(nb_rows):
            continue

        bone = nb_rows[0][0]
        if bone not in top:
            continue

        neighbor_count[bone] += 1
        d = (Vector(mesh.vertices[nb].co) - pos).length
        inverse_distance[bone] += 1.0 / max(float(d), 1e-9)

    if neighbor_count:
        best_count = max(neighbor_count.values())
        count_candidates = sorted(
            bone for bone, count in neighbor_count.items()
            if count == best_count
        )

        if len(count_candidates) == 1:
            return count_candidates[0], "TIE_ADJACENT_RIGID_MAJORITY", {
                "top_candidates": top,
                "neighbor_count": dict(neighbor_count),
                "inverse_distance": dict(inverse_distance),
            }

        best_distance = max(inverse_distance[b] for b in count_candidates)
        distance_candidates = sorted(
            b for b in count_candidates
            if abs(inverse_distance[b] - best_distance) <= 1e-12
        )

        if len(distance_candidates) == 1:
            return distance_candidates[0], "TIE_ADJACENT_RIGID_DISTANCE", {
                "top_candidates": top,
                "neighbor_count": dict(neighbor_count),
                "inverse_distance": dict(inverse_distance),
            }

        return None, "TIE_STILL_AMBIGUOUS", {
            "top_candidates": top,
            "neighbor_count": dict(neighbor_count),
            "inverse_distance": dict(inverse_distance),
        }

    return None, "TIE_NO_RIGID_NEIGHBOR", {
        "top_candidates": top,
        "neighbor_count": {},
        "inverse_distance": {},
    }


def set_exact_rigid_weight(obj, vi, bone_name, review_group_name):
    """
    Remove vi from every non-review group, then assign exactly 1.0 to bone.
    Also removes vi from the review group if it already exists.
    """
    # Snapshot names because group membership mutation changes the collection.
    current_names = []
    for e in list(obj.data.vertices[vi].groups):
        if 0 <= e.group < len(obj.vertex_groups):
            current_names.append(obj.vertex_groups[e.group].name)

    for name in current_names:
        group = obj.vertex_groups.get(name)
        if group is not None:
            group.remove([vi])

    group = obj.vertex_groups.get(bone_name)
    if group is None:
        group = obj.vertex_groups.new(name=bone_name)
    group.add([vi], 1.0, "REPLACE")

    review = obj.vertex_groups.get(review_group_name)
    if review is not None:
        try:
            review.remove([vi])
        except RuntimeError:
            pass


def add_review_marker(obj, vi, review_group_name):
    group = obj.vertex_groups.get(review_group_name)
    if group is None:
        group = obj.vertex_groups.new(name=review_group_name)
    group.add([vi], 1.0, "REPLACE")


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default=DEFAULT_TARGET)
    p.add_argument("--out", required=True)
    p.add_argument("--report", required=True)
    p.add_argument("--review-group", default=REVIEW_GROUP_DEFAULT)
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
            "refusing to overwrite the artist source .blend; choose a different --out"
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

    initial_rows = [
        weights_for_vertex(obj, vi, ignore_group=args.review_group)
        for vi in range(len(mesh.vertices))
    ]
    adjacency = adjacency_for_mesh(mesh)

    classifications = []
    policy_hist = Counter()
    chosen_bone_hist = Counter()

    strict_before = 0
    mixed_before = 0
    empty_before = 0

    for vi, rows in enumerate(initial_rows):
        if not rows:
            empty_before += 1
        elif is_strict_rigid(rows):
            strict_before += 1
        else:
            mixed_before += 1

        chosen, policy, details = resolve_vertex(
            vi, rows, mesh, adjacency, initial_rows
        )
        policy_hist[policy] += 1
        if chosen is not None:
            chosen_bone_hist[chosen] += 1

        classifications.append({
            "vertex": vi,
            "weights_before": rows,
            "chosen_bone": chosen,
            "policy": policy,
            "details": details,
        })

    auto_resolved = []
    unresolved = []
    preserved = []

    for row in classifications:
        vi = row["vertex"]
        policy = row["policy"]
        chosen = row["chosen_bone"]

        if policy == "ALREADY_RIGID":
            preserved.append(vi)
            continue

        if chosen is not None:
            set_exact_rigid_weight(obj, vi, chosen, args.review_group)
            auto_resolved.append(vi)
        else:
            # Leave artist weights intact and add an explicit blocker/selector.
            add_review_marker(obj, vi, args.review_group)
            unresolved.append(vi)

    mesh.update()

    # Verify all auto-resolved vertices are now strict rigid if we ignore the
    # review marker (which should not exist on them anyway).
    auto_verify_fail = []
    for vi in auto_resolved:
        rows = weights_for_vertex(obj, vi, ignore_group=args.review_group)
        if not is_strict_rigid(rows):
            auto_verify_fail.append((vi, rows))

    if auto_verify_fail:
        raise RuntimeError(
            f"automatic rigidization verification failed: {auto_verify_fail[:20]}"
        )

    # Verify unresolved artist weights were not replaced.
    unresolved_changed = []
    for vi in unresolved:
        now = weights_for_vertex(obj, vi, ignore_group=args.review_group)
        if now != initial_rows[vi]:
            unresolved_changed.append({
                "vertex": vi,
                "before": initial_rows[vi],
                "after": now,
            })

    if unresolved_changed:
        raise RuntimeError(
            "unresolved artist weights changed unexpectedly: "
            f"{unresolved_changed[:10]}"
        )

    source_sha = file_sha256(source_blend)

    out.parent.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)

    status = "PASS" if not unresolved else "REVIEW_REQUIRED"

    artist_message = {
        "summary": (
            "A cópia foi normalizada para o Physique rígido do jogo. "
            "O arquivo artístico original não foi alterado."
        ),
        "why": (
            "O runtime deste pipeline aceita exatamente um bone por vértice. "
            "O Blender interpola pesos ao subdividir, criando vértices com dois "
            "ou mais bones."
        ),
        "automatic": (
            f"{len(auto_resolved)} vértices mistos foram convertidos "
            "automaticamente apenas quando havia uma decisão segura."
        ),
        "manual": (
            f"{len(unresolved)} vértices continuam ambíguos e foram marcados "
            f"no vertex group {args.review_group!r}. Selecione esse grupo na "
            "cópia, escolha manualmente um único bone para cada vértice com "
            "weight 1.0 e depois remova o grupo marcador antes de validar/exportar."
            if unresolved else
            "Nenhum vértice exige revisão manual."
        ),
    }

    report = {
        "tool": "PT_RIGID_SKIN_NORMALIZE_COPY_V2",
        "status": status,
        "source_blend": str(source_blend),
        "source_blend_sha256": source_sha,
        "output_blend": str(out),
        "object": target,
        "mode_sync": mode_sync,
        "vertices": len(mesh.vertices),
        "strict_rigid_before": strict_before,
        "mixed_before": mixed_before,
        "empty_before": empty_before,
        "preserved_rigid": len(preserved),
        "auto_resolved": len(auto_resolved),
        "unresolved": len(unresolved),
        "review_group": args.review_group if unresolved else None,
        "policy_histogram": dict(policy_hist),
        "chosen_bone_histogram": dict(chosen_bone_hist),
        "unresolved_vertices": [
            row for row in classifications if row["chosen_bone"] is None
        ],
        "auto_resolved_vertices": [
            row for row in classifications
            if row["chosen_bone"] is not None
            and row["policy"] != "ALREADY_RIGID"
        ],
        "artist_message": artist_message,
    }

    report_path.write_text(
        json.dumps(report, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    # Save only to the NEW path. Original artist source remains untouched.
    bpy.ops.wm.save_as_mainfile(filepath=str(out), check_existing=False)

    print("PT_RIGID_SKIN_NORMALIZE_COPY_V2")
    print(f"object={target}")
    print(f"source_blend={source_blend}")
    print(f"source_blend_sha256={source_sha}")
    print(f"output_blend={out}")
    print(f"vertices={len(mesh.vertices)}")
    print(f"strict_rigid_before={strict_before}")
    print(f"mixed_before={mixed_before}")
    print(f"empty_before={empty_before}")
    print(f"preserved_rigid={len(preserved)}")
    print(f"auto_resolved={len(auto_resolved)}")
    print(f"unresolved={len(unresolved)}")
    print(f"policy_histogram={dict(policy_hist)}")
    print(f"review_group={repr(args.review_group) if unresolved else None}")
    print(f"report={report_path}")
    print()
    print("ARTIST_MESSAGE")
    print(f"WHY: {artist_message['why']}")
    print(f"AUTOMATIC: {artist_message['automatic']}")
    print(f"MANUAL: {artist_message['manual']}")
    print()
    if unresolved:
        print("normalize_copy_check=REVIEW_REQUIRED")
    else:
        print("normalize_copy_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(
            f"normalize_copy_check=FAIL: "
            f"{type(exc).__name__}: {exc}"
        )
        raise

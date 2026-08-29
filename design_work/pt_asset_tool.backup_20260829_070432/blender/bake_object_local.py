from __future__ import annotations

import argparse
import json
import pathlib
import sys

try:
    import bpy
except Exception:
    bpy = None

PROJECT_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from pt_asset_tool.object_local import (  # noqa: E402
    ENGINE_UNITS_PER_PREVIEW_FRAME,
    compact_report,
    normalized_sample,
    parse_object_local_smd,
)


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--report", required=True)
    p.add_argument("--fps", type=int, default=30)
    p.add_argument("--engine-step", type=int, default=ENGINE_UNITS_PER_PREVIEW_FRAME)
    return p.parse_args(argv)


def _numeric_suffix_rank(name: str, base: str):
    if name.casefold() == base.casefold():
        return (0, 0, name.casefold())
    prefix = base + "."
    if name.casefold().startswith(prefix.casefold()):
        tail = name[len(prefix):]
        if tail.isdigit():
            return (1, int(tail), name.casefold())
    return (2, 10**9, name.casefold())


def _map_source_meshes(parsed):
    """Map serialized objects to Blender meshes by source order, including duplicates.

    Blender renames duplicate object names to ``Name.001`` etc.  The client SMD
    format allows duplicate NodeName values, so name-only lookup is insufficient.
    """
    all_meshes = [o for o in bpy.data.objects if getattr(o, "type", None) == "MESH"]
    used = set()
    mapped = {}
    for src in parsed["objects"]:
        base = str(src["name"])
        hits = [o for o in all_meshes
                if id(o) not in used and
                (o.name.casefold() == base.casefold() or o.name.casefold().startswith((base + ".").casefold()))]
        hits.sort(key=lambda o: _numeric_suffix_rank(o.name, base))
        if not hits:
            raise RuntimeError(
                f"source object not present in Blender by serialized order: index={src['index']} name={base!r}"
            )
        obj = hits[0]
        used.add(id(obj))
        mapped[int(src["index"])] = obj
    return mapped


def _action_curves(action):
    try:
        return list(action.fcurves)
    except Exception:
        curves = []
        try:
            for layer in action.layers:
                for strip in layer.strips:
                    for bag in getattr(strip, "channelbags", []):
                        curves.extend(list(getattr(bag, "fcurves", [])))
        except Exception:
            pass
        return curves


def _set_linear_interpolation(obj):
    ad = obj.animation_data
    if ad is None or ad.action is None:
        return
    for fc in _action_curves(ad.action):
        for kp in fc.keyframe_points:
            kp.interpolation = "LINEAR"


def _global_engine_frames(animated_sources, step: int):
    global_ref = min(int(o["reference_engine_frame"]) for o in animated_sources)
    global_end = max(int(o["max_engine_frame"]) for o in animated_sources)
    frames = {global_ref, global_end}
    for src in animated_sources:
        frames.update(int(x) for x in src.get("source_key_frames") or [])
    f = global_ref
    while f <= global_end:
        frames.add(f)
        f += step
    return global_ref, global_end, sorted(x for x in frames if global_ref <= x <= global_end)


def bake(a):
    if bpy is None:
        raise RuntimeError("bake_object_local.py must run inside Blender")
    if a.fps <= 0 or a.fps > 240:
        raise ValueError("fps must be 1..240")
    if a.engine_step <= 0:
        raise ValueError("engine-step must be positive")

    smd = pathlib.Path(a.smd).expanduser().resolve()
    out = pathlib.Path(a.out).expanduser().resolve()
    report_path = pathlib.Path(a.report).expanduser().resolve()
    parsed = parse_object_local_smd(smd, strict_v1=True)

    animated_sources = [o for o in parsed["objects"] if o["motion_class"] == "ANIMATED"]
    if not animated_sources:
        raise RuntimeError("object-local source contains no moving transforms (HOLD/NONE only)")
    blender_by_index = _map_source_meshes(parsed)
    for src in parsed["objects"]:
        obj = blender_by_index[int(src["index"])]
        obj.animation_data_clear()
        obj.rotation_mode = "QUATERNION"
        # V3: preserve the static/imported bind transform in the primary channels.
        # Object-local animation is baked into Blender Delta Transforms so animated
        # children retain their parent-relative bind while receiving local curves.
        obj.delta_location = (0.0, 0.0, 0.0)
        obj.delta_scale = (1.0, 1.0, 1.0)
        obj.delta_rotation_quaternion = (1.0, 0.0, 0.0, 0.0)

    # Recreate only *resolved runtime* edges. Missing NodeParent helpers remain roots,
    # matching smPAT3D::LinkObject (pParent stays null when no match is found).
    for src in parsed["objects"]:
        pi = src.get("parent_index")
        if pi is None:
            continue
        child = blender_by_index[int(src["index"])]
        parent = blender_by_index[int(pi)]
        world = child.matrix_world.copy()
        child.parent = parent
        child.matrix_world = world

    global_ref, global_end, frames = _global_engine_frames(animated_sources, int(a.engine_step))
    previous_q = {}
    sample_rows = []
    animated_names = []

    for src in animated_sources:
        obj = blender_by_index[int(src["index"])]
        animated_names.append(src["name"])
        src_key = int(src["index"])
        ref = int(src["reference_engine_frame"])
        for ef in frames:
            sample = normalized_sample(src, ef, ref)
            qx, qy, qz, qw = (float(v) for v in sample["rotation_xyzw"])
            q_xyzw = (qx, qy, qz, qw)
            prev = previous_q.get(src_key)
            if prev is not None and sum(x*y for x, y in zip(prev, q_xyzw)) < 0.0:
                q_xyzw = tuple(-x for x in q_xyzw)
                qx, qy, qz, qw = q_xyzw
            previous_q[src_key] = q_xyzw

            bf = 1.0 + (float(ef)-float(global_ref))/float(a.engine_step)
            obj.delta_location = tuple(float(v) for v in sample["location"])
            obj.delta_scale = tuple(float(v) for v in sample["scale"])
            obj.delta_rotation_quaternion = (qw, qx, qy, qz)
            obj.keyframe_insert(data_path="delta_location", frame=bf, group="PT_OBJECT_LOCAL")
            obj.keyframe_insert(data_path="delta_scale", frame=bf, group="PT_OBJECT_LOCAL")
            obj.keyframe_insert(data_path="delta_rotation_quaternion", frame=bf, group="PT_OBJECT_LOCAL")
            if ef in {frames[0], frames[len(frames)//2], frames[-1]}:
                sample_rows.append({"object": src["name"], "object_index": src_key, "engine_frame": ef, "blender_frame": bf, **sample})
        _set_linear_interpolation(obj)

    scene = bpy.context.scene
    scene.render.fps = int(a.fps)
    scene.frame_start = 1
    scene.frame_end = int(round(1.0 + (global_end-global_ref)/float(a.engine_step)))
    scene.frame_set(1)
    scene["pt_object_local_preview"] = True
    scene["pt_object_local_preview_version"] = 3
    scene["pt_object_local_source"] = str(smd)
    scene["pt_object_local_animated_objects"] = json.dumps(animated_names, ensure_ascii=False)
    scene["pt_object_local_parent_edge_count"] = int(parsed.get("parent_edge_count", 0))
    scene["pt_object_local_hierarchy_semantics"] = "LOCAL_QMAT_TIMES_PARENT_TMRESULT"
    scene["pt_object_local_preview_reference"] = "FIRST_KEY_NORMALIZED_PER_OBJECT_DELTA_OVER_STATIC_BIND"
    scene["pt_object_local_rotation_semantics"] = "ACCUMULATED_TM_PREV_ROT"
    scene["pt_object_local_source_rot_key_semantics"] = "INCREMENTAL_LOCAL_QUATERNION"
    scene["pt_object_local_engine_units_per_frame"] = int(a.engine_step)
    scene["pt_object_local_max_engine_frame"] = int(global_end)
    scene["pt_object_local_timing_status"] = "PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED"

    out.parent.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    source_summary = compact_report(parsed)
    channels = sorted({c for src in animated_sources for c in src["channels_animated"]})
    report = {
        "format": "PT_OBJECT_LOCAL_BLENDER_PREVIEW_V3",
        "status": "PASS",
        "source": str(smd),
        "blend": str(out),
        "object_count": parsed["object_count"],
        "animated_object_count": len(animated_sources),
        "animated_objects": animated_names,
        "parent_edge_count": parsed.get("parent_edge_count", 0),
        "parent_edges": parsed.get("parent_edges", []),
        "root_objects": parsed.get("root_objects", []),
        "unresolved_parent_count": parsed.get("unresolved_parent_count", 0),
        "unresolved_parents": parsed.get("unresolved_parents", []),
        "duplicate_node_name_group_count": parsed.get("duplicate_node_name_group_count", 0),
        "duplicate_node_name_groups": parsed.get("duplicate_node_name_groups", {}),
        "parent_resolution_semantics": parsed.get("parent_resolution_semantics"),
        "channels_animated": channels,
        "reference_engine_frame": global_ref,
        "max_engine_frame": global_end,
        "engine_units_per_preview_frame": int(a.engine_step),
        "preview_frame_start": scene.frame_start,
        "preview_frame_end": scene.frame_end,
        "preview_fps": scene.render.fps,
        "preview_reference": "FIRST_KEY_NORMALIZED_PER_OBJECT_DELTA_OVER_STATIC_BIND",
        "blender_transform_semantics": "DELTA_TRANSFORMS_OVER_STATIC_BIND",
        "hierarchy_semantics": "LOCAL_QMAT_TIMES_PARENT_TMRESULT",
        "hierarchy_sourcecode_proof": "smOBJ3D::TmAnimation qmat * pParent->TmResult; smPAT3D::LinkObject parent-first TmSort",
        "rotation_semantics": "ACCUMULATED_TM_PREV_ROT",
        "source_rot_key_semantics": "INCREMENTAL_LOCAL_QUATERNION",
        "quaternion_sign_continuity": True,
        "interpolation": "LINEAR_SAMPLED_EVERY_SOURCE_TICK",
        "timing_status": "PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED",
        "samples": sample_rows,
        "source_summary": source_summary,
        "client_modified": False,
    }
    report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
    print("PT_OBJECT_LOCAL_BLENDER_PREVIEW_V3")
    print(f"objects={parsed['object_count']} animated={len(animated_sources)} parent_edges={parsed.get('parent_edge_count',0)}")
    print(f"animated_objects={animated_names}")
    print(f"source_frames={global_ref}..{global_end}")
    print(f"preview_frames={scene.frame_start}..{scene.frame_end}")
    print(f"preview_fps={scene.render.fps}")
    print("hierarchy_semantics=LOCAL_QMAT_TIMES_PARENT_TMRESULT")
    print("blender_transform_semantics=DELTA_TRANSFORMS_OVER_STATIC_BIND")
    print("timing_status=PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED")
    print("client_modified=False")
    print("object_local_preview_check=PASS")


def main(argv):
    a = parse_args(argv)
    bake(a)


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    main(argv)

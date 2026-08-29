from __future__ import annotations

import argparse
import json
import pathlib
import sys

try:
    import bpy
except Exception:
    bpy = None

# Package root is .../pt_asset_tool/blender/bake_object_local.py
PROJECT_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from pt_asset_tool.object_local import (  # noqa: E402
    ENGINE_UNITS_PER_PREVIEW_FRAME,
    compact_report,
    normalized_sample,
    parse_object_local_smd,
    preview_engine_frames,
)


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--report", required=True)
    p.add_argument("--fps", type=int, default=30)
    p.add_argument("--engine-step", type=int, default=ENGINE_UNITS_PER_PREVIEW_FRAME)
    return p.parse_args(argv)


def _find_object(name: str):
    exact = bpy.data.objects.get(name)
    if exact is not None:
        return exact
    hits = [o for o in bpy.data.objects if o.name.casefold() == name.casefold()]
    if len(hits) == 1:
        return hits[0]
    raise RuntimeError(f"source object not uniquely present in Blender: {name!r}; hits={[o.name for o in hits]}")


def _set_linear_interpolation(obj):
    ad = obj.animation_data
    if ad is None or ad.action is None:
        return
    action = ad.action
    # Blender <=4.x API.
    try:
        curves = list(action.fcurves)
    except Exception:
        curves = []
        # Blender 5.x layered actions. Traverse whatever channelbags expose.
        try:
            for layer in action.layers:
                for strip in layer.strips:
                    for bag in getattr(strip, "channelbags", []):
                        curves.extend(list(getattr(bag, "fcurves", [])))
        except Exception:
            pass
    for fc in curves:
        for kp in fc.keyframe_points:
            kp.interpolation = "LINEAR"


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

    animated = [o for o in parsed["objects"] if o["motion_class"] == "ANIMATED"]
    if not animated:
        raise RuntimeError("object-local source contains no moving transforms (HOLD/NONE only)")
    if len(parsed["objects"]) != 1:
        raise RuntimeError(f"OBJECT_LOCAL_PREVIEW_V1 requires exactly one source object; objects={len(parsed['objects'])}")
    obj_src = animated[0]
    if obj_src.get("parent"):
        raise RuntimeError(f"OBJECT_LOCAL_PREVIEW_V1 does not certify NodeParent hierarchy: {obj_src['parent']!r}")

    obj = _find_object(obj_src["name"])
    if obj.type != "MESH":
        raise RuntimeError(f"object-local target is not a mesh: {obj.name}/{obj.type}")
    obj.animation_data_clear()
    obj.rotation_mode = "QUATERNION"

    ref = int(obj_src["reference_engine_frame"])
    frames = preview_engine_frames(obj_src, a.engine_step)
    previous_q = None
    sample_rows = []
    for ef in frames:
        sample = normalized_sample(obj_src, ef, ref)
        qx, qy, qz, qw = (float(v) for v in sample["rotation_xyzw"])
        # Keep quaternion sign continuous before component keyframing.
        q_xyzw = (qx, qy, qz, qw)
        if previous_q is not None and sum(x*y for x, y in zip(previous_q, q_xyzw)) < 0.0:
            q_xyzw = tuple(-x for x in q_xyzw)
            qx, qy, qz, qw = q_xyzw
        previous_q = q_xyzw

        bf = 1.0 + (float(ef)-float(ref))/float(a.engine_step)
        obj.location = tuple(float(v) for v in sample["location"])
        obj.scale = tuple(float(v) for v in sample["scale"])
        obj.rotation_quaternion = (qw, qx, qy, qz)
        obj.keyframe_insert(data_path="location", frame=bf, group="PT_OBJECT_LOCAL")
        obj.keyframe_insert(data_path="scale", frame=bf, group="PT_OBJECT_LOCAL")
        obj.keyframe_insert(data_path="rotation_quaternion", frame=bf, group="PT_OBJECT_LOCAL")
        if ef in {frames[0], frames[len(frames)//2], frames[-1]}:
            sample_rows.append({"engine_frame": ef, "blender_frame": bf, **sample})

    _set_linear_interpolation(obj)
    scene = bpy.context.scene
    scene.render.fps = int(a.fps)
    scene.frame_start = 1
    scene.frame_end = int(round(1.0 + (frames[-1]-ref)/float(a.engine_step)))
    scene.frame_set(1)
    scene["pt_object_local_preview"] = True
    scene["pt_object_local_source"] = str(smd)
    scene["pt_object_local_object"] = obj_src["name"]
    scene["pt_object_local_preview_reference"] = "FIRST_KEY_NORMALIZED_OBJECT_LOCAL"
    scene["pt_object_local_rotation_semantics"] = "ACCUMULATED_TM_PREV_ROT"
    scene["pt_object_local_source_rot_key_semantics"] = "INCREMENTAL_LOCAL_QUATERNION"
    scene["pt_object_local_engine_units_per_frame"] = int(a.engine_step)
    scene["pt_object_local_max_engine_frame"] = int(obj_src["max_engine_frame"])
    scene["pt_object_local_timing_status"] = "PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED"

    out.parent.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    source_summary = compact_report(parsed)
    report = {
        "format": "PT_OBJECT_LOCAL_BLENDER_PREVIEW_V1",
        "status": "PASS",
        "source": str(smd),
        "blend": str(out),
        "object": obj_src["name"],
        "channels_serialized": obj_src["channels_serialized"],
        "channels_animated": obj_src["channels_animated"],
        "nRot": obj_src["nRot"], "nPos": obj_src["nPos"], "nScale": obj_src["nScale"],
        "reference_engine_frame": ref,
        "max_engine_frame": obj_src["max_engine_frame"],
        "engine_units_per_preview_frame": int(a.engine_step),
        "preview_frame_start": scene.frame_start,
        "preview_frame_end": scene.frame_end,
        "preview_fps": scene.render.fps,
        "preview_reference": "FIRST_KEY_NORMALIZED_OBJECT_LOCAL",
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
    print("PT_OBJECT_LOCAL_BLENDER_PREVIEW_V1")
    print(f"object={obj_src['name']}")
    print(f"channels={obj_src['channels_animated']}")
    print(f"source_frames={ref}..{obj_src['max_engine_frame']}")
    print(f"preview_frames={scene.frame_start}..{scene.frame_end}")
    print(f"preview_fps={scene.render.fps}")
    print("timing_status=PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED")
    print("client_modified=False")
    print("object_local_preview_check=PASS")


def main(argv):
    a = parse_args(argv)
    bake(a)


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    main(argv)

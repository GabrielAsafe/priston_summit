#!/usr/bin/env python3
# Priston Tale Blender motion bake V2
#
# Uses the visually validated Armature Mapping V2 method:
#   source TmResult
#   -> desired Blender pose matrix
#   -> Bone.convert_local_to_pose(..., invert=True)
#   -> PoseBone.matrix_basis
#   -> keyframe location/quaternion/scale
#
# Then re-evaluates every keyed frame through Blender and compares the
# Armature-modifier result vertex-by-vertex against direct source skinning.
#
# No source asset modification.

import argparse
import importlib.util
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix


def load_module(path, name):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load module: {path}")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def row_transform_point(p, m):
    x, y, z = p
    return (
        x * m[0] + y * m[4] + z * m[8]  + m[12],
        x * m[1] + y * m[5] + z * m[9]  + m[13],
        x * m[2] + y * m[6] + z * m[10] + m[14],
    )


def point_distance(a, b):
    return math.sqrt(
        (a[0] - b[0]) ** 2
        + (a[1] - b[1]) ** 2
        + (a[2] - b[2]) ** 2
    )


def bbox(points):
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    zs = [p[2] for p in points]
    return (
        min(xs), max(xs),
        min(ys), max(ys),
        min(zs), max(zs),
    )


def fmt_bbox(bb):
    return (
        f"x[{bb[0]:.4f},{bb[1]:.4f}] "
        f"y[{bb[2]:.4f},{bb[3]:.4f}] "
        f"z[{bb[4]:.4f},{bb[5]:.4f}]"
    )


def evaluated_vertex_positions(obj):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    eobj = obj.evaluated_get(depsgraph)
    mesh = eobj.to_mesh()
    try:
        mw = eobj.matrix_world
        out = []
        for v in mesh.vertices:
            co = mw @ v.co
            out.append((float(co.x), float(co.y), float(co.z)))
        return out
    finally:
        eobj.to_mesh_clear()


def desired_pose_matrix(core, source_rest_row, source_pose_row, blender_rest):
    source_rest_col = core.row_engine_to_blender_matrix(source_rest_row)
    source_pose_col = core.row_engine_to_blender_matrix(source_pose_row)
    deform_col = source_pose_col @ source_rest_col.inverted()
    return deform_col @ blender_rest


def assign_frame_basis(arm, smb, core, results):
    # Parent-first assignment, using Blender's own local/pose conversion.
    for idx in smb["tm_sort"]:
        src_bone = smb["bones"][idx]
        name = src_bone["name"]

        pb = arm.pose.bones[name]
        bone = arm.data.bones[name]

        target_pose = desired_pose_matrix(
            core,
            src_bone["tm"],
            results[idx],
            bone.matrix_local.copy(),
        )

        if pb.parent is None:
            basis = bone.convert_local_to_pose(
                target_pose,
                bone.matrix_local,
                invert=True,
            )
        else:
            basis = bone.convert_local_to_pose(
                target_pose,
                bone.matrix_local,
                parent_matrix=pb.parent.matrix,
                parent_matrix_local=bone.parent.matrix_local,
                invert=True,
            )

        pb.matrix_basis = basis
        bpy.context.view_layer.update()


def set_all_interpolation(action, mode='LINEAR'):
    # Blender 5.x Action API compatibility:
    # legacy fcurves remain available on normal Actions created by keyframe_insert.
    fcurves = getattr(action, "fcurves", None)
    if fcurves is None:
        return 0

    count = 0
    for fc in fcurves:
        for kp in fc.keyframe_points:
            kp.interpolation = mode
            count += 1
    return count


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--smd", required=True, type=Path)
    ap.add_argument("--skeleton", required=True, type=Path)
    ap.add_argument("--inx", required=True, type=Path)
    ap.add_argument("--logical-path", required=True)
    ap.add_argument("--motion-index", type=int, default=10)
    ap.add_argument("--armature", default="PT_m5_Armature")
    ap.add_argument("--out", required=True, type=Path)

    raw = sys.argv
    args = ap.parse_args(raw[raw.index("--") + 1:] if "--" in raw else [])

    cwd = Path.cwd()

    core = load_module(
        cwd / "pt_blender_apply_motion_v1.py",
        "pt_anim_core_bake_v2",
    )
    imp = load_module(
        cwd / "pt_smd_blender_import_v3_static_armature.py",
        "pt_smd_import_core_bake_v2",
    )

    arm = bpy.data.objects.get(args.armature)
    if arm is None or arm.type != "ARMATURE":
        raise RuntimeError(f"armature {args.armature!r} not found")

    smd_data = args.smd.read_bytes()
    smd_meta = imp.parse_header_and_infos(smd_data)
    smd_objects = imp.parse_objects(smd_data, smd_meta)
    smd_by_name = {o["name"]: o for o in smd_objects}

    smb = core.parse_smb(args.skeleton)

    motion, motion_count = core.read_inx_motion(
        args.inx,
        args.logical_path,
        args.motion_index,
        smb["header_frames"],
    )

    visible = [
        obj for obj in bpy.data.objects
        if obj.type == "MESH" and int(obj.get("pt_inx_selected", 0)) == 1
    ]
    if not visible:
        visible = [
            obj for obj in bpy.data.objects
            if obj.type == "MESH" and not obj.hide_render
        ]

    for obj in visible:
        arm_mods = [m for m in obj.modifiers if m.type == "ARMATURE"]
        if len(arm_mods) != 1:
            raise RuntimeError(
                f"{obj.name}: expected one Armature modifier, "
                f"found {len(arm_mods)}"
            )
        arm_mods[0].object = arm
        arm_mods[0].show_viewport = True
        arm_mods[0].show_render = True

    logical_frames = list(
        range(motion["global_start"], motion["global_end"])
    )
    engine_frames = [f * 160 for f in logical_frames]
    blender_frames = list(range(1, len(engine_frames) + 1))

    if not blender_frames:
        raise RuntimeError("selected motion has no runtime frames")

    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = len(blender_frames)
    scene.frame_preview_start = 1
    scene.frame_preview_end = len(blender_frames)

    # Clear previous animation and pose state.
    arm.animation_data_clear()
    for pb in arm.pose.bones:
        pb.rotation_mode = 'QUATERNION'
        pb.matrix_basis = Matrix.Identity(4)

    bpy.context.view_layer.update()

    print("PT_BLENDER_MOTION_BAKE_V2")
    print(f"input_blend={bpy.data.filepath}")
    print(f"motion_count={motion_count}")
    print(f"motion_index={motion['index']}")
    print(f"state={motion['state_name']}")
    print(f"repeat={motion['repeat']}")
    print(f"motion_frame={motion['motion_frame']}")
    print(
        f"engine_half_open=[{motion['engine_start']},"
        f"{motion['engine_end']})"
    )
    print(f"blender_frames=1..{len(blender_frames)}")
    print(f"bones={len(smb['bones'])}")
    print(f"visible_meshes={[o.name for o in visible]!r}")
    print()

    print("BAKE")

    for bf, ef in zip(blender_frames, engine_frames):
        scene.frame_set(bf)

        results, _, _, _ = core.evaluate_pose(
            smb,
            ef,
            motion["engine_end"],
        )

        assign_frame_basis(arm, smb, core, results)

        for idx in smb["tm_sort"]:
            name = smb["bones"][idx]["name"]
            pb = arm.pose.bones[name]

            pb.keyframe_insert(data_path="location", frame=bf)
            pb.keyframe_insert(
                data_path="rotation_quaternion",
                frame=bf,
            )
            pb.keyframe_insert(data_path="scale", frame=bf)

    action = (
        arm.animation_data.action
        if arm.animation_data is not None
        else None
    )
    if action is None:
        raise RuntimeError("keyframe bake did not create an Action")

    action.name = (
        f"PT_m5_{motion['state_name']}_motion{motion['index']}_V2"
    )

    interpolation_keys = set_all_interpolation(action, 'LINEAR')

    # ------------------------------------------------------------------
    # Ground-truth validation AFTER the keyframes exist.
    # This tests Blender evaluating the Action, not merely assignment time.
    # ------------------------------------------------------------------

    bone_index = {
        bone["name"]: bone["index"]
        for bone in smb["bones"]
    }

    total_vertex_samples = 0
    sum_sq = 0.0
    max_vertex_error = 0.0
    max_vertex_where = None

    frame_max_errors = []
    frame_rms_errors = []
    bbox_samples = {}

    sample_frames = sorted(set([
        1,
        max(1, len(blender_frames) // 2),
        len(blender_frames),
    ]))

    print()
    print("POST_BAKE_SOURCE_COMPARISON")

    for bf, ef in zip(blender_frames, engine_frames):
        scene.frame_set(bf)
        bpy.context.view_layer.update()

        source_results, _, _, source_stats = core.evaluate_pose(
            smb,
            ef,
            motion["engine_end"],
        )

        frame_count = 0
        frame_sum_sq = 0.0
        frame_max = 0.0

        source_points = []
        blender_points = []

        for obj in visible:
            src = smd_by_name.get(obj.name)
            if src is None:
                raise RuntimeError(f"{obj.name}: absent from SMD")

            actual_positions = evaluated_vertex_positions(obj)

            if len(actual_positions) != len(src["raw_vertices"]):
                raise RuntimeError(
                    f"{obj.name}: evaluated vertices="
                    f"{len(actual_positions)} "
                    f"SMD vertices={len(src['raw_vertices'])}"
                )

            for vidx, (raw_local, bone_name) in enumerate(
                zip(src["raw_vertices"], src["bones"])
            ):
                bidx = bone_index.get(bone_name)
                if bidx is None:
                    raise RuntimeError(
                        f"{obj.name} vertex {vidx}: "
                        f"missing bone {bone_name!r}"
                    )

                local = (
                    raw_local[0] / 256.0,
                    raw_local[1] / 256.0,
                    raw_local[2] / 256.0,
                )

                expected = row_transform_point(
                    local,
                    source_results[bidx],
                )
                actual = actual_positions[vidx]

                err = point_distance(expected, actual)

                frame_max = max(frame_max, err)
                frame_sum_sq += err * err
                frame_count += 1

                sum_sq += err * err
                total_vertex_samples += 1

                if err > max_vertex_error:
                    max_vertex_error = err
                    max_vertex_where = (
                        bf,
                        ef,
                        obj.name,
                        vidx,
                        bone_name,
                        expected,
                        actual,
                    )

                source_points.append(expected)
                blender_points.append(actual)

        frame_rms = math.sqrt(frame_sum_sq / frame_count)
        frame_max_errors.append((bf, ef, frame_max))
        frame_rms_errors.append((bf, ef, frame_rms))

        if bf in sample_frames:
            source_bb = bbox(source_points)
            blender_bb = bbox(blender_points)
            bbox_samples[bf] = (source_bb, blender_bb)

            print(
                f"frame={bf} engine={ef} "
                f"max_vertex_error={frame_max:.9g} "
                f"rms_vertex_error={frame_rms:.9g}"
            )
            print(f"  source_bbox={fmt_bbox(source_bb)}")
            print(f"  blender_bbox={fmt_bbox(blender_bb)}")
            print(
                f"  source_rot_active={source_stats['active_rot']} "
                f"source_pos_active={source_stats['active_pos']} "
                f"source_rot_end_clamps={source_stats['rot_end_clamps']}"
            )

    overall_rms = math.sqrt(sum_sq / total_vertex_samples)

    worst_frame = max(frame_max_errors, key=lambda x: x[2])
    worst_rms_frame = max(frame_rms_errors, key=lambda x: x[2])

    print()
    print("VALIDATION_METRICS")
    print(f"action={action.name}")
    print(f"keyed_bones={len(smb['bones'])}")
    print(f"keyed_frames={len(blender_frames)}")
    print(f"interpolation_keypoints_set={interpolation_keys}")
    print(f"total_vertex_samples={total_vertex_samples}")
    print(f"max_vertex_error={max_vertex_error:.9g}")
    print(f"overall_rms_vertex_error={overall_rms:.9g}")
    print(f"worst_frame={worst_frame!r}")
    print(f"worst_rms_frame={worst_rms_frame!r}")
    print(f"max_vertex_error_where={max_vertex_where!r}")

    # The validated single-pose V2 produced max ~= 0.0217.
    # Use 0.05 as a conservative gate for the whole baked STAND.
    geometry_pass = (
        math.isfinite(max_vertex_error)
        and math.isfinite(overall_rms)
        and max_vertex_error <= 0.05
        and overall_rms <= 0.02
    )

    scene.frame_set(1)
    bpy.context.view_layer.update()

    scene["pt_animation_bake_version"] = "V2"
    scene["pt_animation_motion_index"] = motion["index"]
    scene["pt_animation_state"] = motion["state_name"]
    scene["pt_animation_repeat"] = int(bool(motion["repeat"]))
    scene["pt_animation_engine_start"] = motion["engine_start"]
    scene["pt_animation_engine_end_exclusive"] = motion["engine_end"]
    scene["pt_animation_blender_start"] = 1
    scene["pt_animation_blender_end"] = len(blender_frames)
    scene["pt_animation_mapping"] = (
        "convert_local_to_pose(invert=True)->matrix_basis"
    )
    scene["pt_animation_validation_max_vertex_error"] = max_vertex_error
    scene["pt_animation_validation_rms_vertex_error"] = overall_rms

    args.out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.out))

    print()
    print("FINAL")
    print(f"timeline=1..{len(blender_frames)}")
    print(f"output={args.out}")
    print(
        "post_bake_source_skin_check="
        + ("PASS" if geometry_pass else "FAIL")
    )
    print(
        "blender_motion_bake_v2_check="
        + ("PASS" if geometry_pass else "FAIL")
    )


main()

#!/usr/bin/env python3
# Priston Tale -> Blender armature mapping probe V2
#
# Goal:
# Compare the Armature-modifier result against the already validated
# direct source skinning result for ONE frame.
#
# No keyframes.
# No source asset modification.
# Opens V3 static blend and saves a NEW diagnostic blend.

import argparse
import importlib.util
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


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


def matrix_max_abs_diff(a, b):
    return max(
        abs(a[r][c] - b[r][c])
        for r in range(4)
        for c in range(4)
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


def apply_pose_via_matrix_basis(arm, smb, core, results):
    # Clear inherited pose state.
    arm.animation_data_clear()
    for pb in arm.pose.bones:
        pb.matrix_basis = Matrix.Identity(4)

    bpy.context.view_layer.update()

    max_pose_error = 0.0
    max_pose_error_bone = None

    # Parent-first, same as source TmSort.
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

        parent_pb = pb.parent
        parent_bone = bone.parent

        if parent_pb is None:
            # Convert target pose-space matrix into this bone's local/basis
            # matrix using Blender's own rest/pose conversion.
            basis = bone.convert_local_to_pose(
                target_pose,
                bone.matrix_local,
                invert=True,
            )
        else:
            basis = bone.convert_local_to_pose(
                target_pose,
                bone.matrix_local,
                parent_matrix=parent_pb.matrix,
                parent_matrix_local=parent_bone.matrix_local,
                invert=True,
            )

        pb.matrix_basis = basis
        bpy.context.view_layer.update()

        err = matrix_max_abs_diff(pb.matrix, target_pose)
        if err > max_pose_error:
            max_pose_error = err
            max_pose_error_bone = name

    bpy.context.view_layer.update()

    # Final pass because changing parents can propagate through children.
    final_max_error = 0.0
    final_max_bone = None

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

        err = matrix_max_abs_diff(pb.matrix, target_pose)
        if err > final_max_error:
            final_max_error = err
            final_max_bone = name

    return (
        max_pose_error,
        max_pose_error_bone,
        final_max_error,
        final_max_bone,
    )


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--smd", required=True, type=Path)
    ap.add_argument("--skeleton", required=True, type=Path)
    ap.add_argument("--inx", required=True, type=Path)
    ap.add_argument("--logical-path", required=True)
    ap.add_argument("--motion-index", type=int, default=10)
    ap.add_argument("--engine-frame", type=int, default=1600)
    ap.add_argument("--armature", default="PT_m5_Armature")
    ap.add_argument("--out", required=True, type=Path)

    raw = sys.argv
    args = ap.parse_args(raw[raw.index("--") + 1:] if "--" in raw else [])

    cwd = Path.cwd()

    core = load_module(
        cwd / "pt_blender_apply_motion_v1.py",
        "pt_anim_core_mapping_probe_v2",
    )
    imp = load_module(
        cwd / "pt_smd_blender_import_v3_static_armature.py",
        "pt_smd_import_core_mapping_probe_v2",
    )

    arm = bpy.data.objects.get(args.armature)
    if arm is None or arm.type != "ARMATURE":
        raise RuntimeError(f"armature {args.armature!r} not found")

    smd_data = args.smd.read_bytes()
    smd_meta = imp.parse_header_and_infos(smd_data)
    smd_objects = imp.parse_objects(smd_data, smd_meta)
    smd_by_name = {o["name"]: o for o in smd_objects}

    smb = core.parse_smb(args.skeleton)

    motion, _ = core.read_inx_motion(
        args.inx,
        args.logical_path,
        args.motion_index,
        smb["header_frames"],
    )

    if not (
        motion["engine_start"]
        <= args.engine_frame
        < motion["engine_end"]
    ):
        raise RuntimeError(
            f"engine frame {args.engine_frame} outside "
            f"[{motion['engine_start']},{motion['engine_end']})"
        )

    results, _, _, stats = core.evaluate_pose(
        smb,
        args.engine_frame,
        motion["engine_end"],
    )

    bone_index = {
        bone["name"]: bone["index"]
        for bone in smb["bones"]
    }

    visible = [
        obj for obj in bpy.data.objects
        if obj.type == "MESH" and int(obj.get("pt_inx_selected", 0)) == 1
    ]
    if not visible:
        visible = [
            obj for obj in bpy.data.objects
            if obj.type == "MESH" and not obj.hide_render
        ]

    print("PT_BLENDER_ARMATURE_MAPPING_PROBE_V2")
    print(f"input_blend={bpy.data.filepath}")
    print(f"motion_index={motion['index']}")
    print(f"state={motion['state_name']}")
    print(f"engine_frame={args.engine_frame}")
    print(f"logical_frame={args.engine_frame / 160.0:g}")
    print(f"bones={len(smb['bones'])}")
    print(f"visible_meshes={[o.name for o in visible]!r}")
    print()

    # Ensure Armature modifiers are active and point to expected armature.
    for obj in visible:
        arm_mods = [m for m in obj.modifiers if m.type == "ARMATURE"]
        if len(arm_mods) != 1:
            raise RuntimeError(
                f"{obj.name}: expected exactly one Armature modifier, "
                f"found {len(arm_mods)}"
            )
        arm_mods[0].object = arm
        arm_mods[0].show_viewport = True
        arm_mods[0].show_render = True

    (
        assignment_max_error,
        assignment_max_bone,
        final_pose_max_error,
        final_pose_max_bone,
    ) = apply_pose_via_matrix_basis(
        arm,
        smb,
        core,
        results,
    )

    print("POSE_ASSIGNMENT")
    print(
        f"max_error_during_assignment="
        f"{assignment_max_error:.9g}"
    )
    print(
        f"max_error_during_assignment_bone="
        f"{assignment_max_bone!r}"
    )
    print(f"final_pose_max_error={final_pose_max_error:.9g}")
    print(f"final_pose_max_error_bone={final_pose_max_bone!r}")
    print()

    # Build source-ground-truth positions and compare against evaluated
    # Armature modifier output vertex-by-vertex.
    total_vertices = 0
    max_vertex_error = 0.0
    max_vertex_where = None
    sum_sq = 0.0
    source_points_all = []
    blender_points_all = []

    per_object = []

    for obj in visible:
        src = smd_by_name.get(obj.name)
        if src is None:
            raise RuntimeError(f"{obj.name}: absent from SMD")
        if src["bones"] is None:
            raise RuntimeError(f"{obj.name}: SMD has no Physique")

        evaluated = evaluated_vertex_positions(obj)

        if len(evaluated) != len(src["raw_vertices"]):
            raise RuntimeError(
                f"{obj.name}: evaluated vertices={len(evaluated)} "
                f"SMD vertices={len(src['raw_vertices'])}"
            )

        source_positions = []

        obj_max = 0.0
        obj_sum_sq = 0.0

        for vidx, (raw_local, bone_name) in enumerate(
            zip(src["raw_vertices"], src["bones"])
        ):
            bidx = bone_index.get(bone_name)
            if bidx is None:
                raise RuntimeError(
                    f"{obj.name} vertex {vidx}: missing bone {bone_name!r}"
                )

            local = (
                raw_local[0] / 256.0,
                raw_local[1] / 256.0,
                raw_local[2] / 256.0,
            )

            expected = row_transform_point(local, results[bidx])
            actual = evaluated[vidx]

            err = point_distance(expected, actual)

            if err > max_vertex_error:
                max_vertex_error = err
                max_vertex_where = (
                    obj.name,
                    vidx,
                    bone_name,
                    expected,
                    actual,
                )

            obj_max = max(obj_max, err)
            obj_sum_sq += err * err
            sum_sq += err * err
            total_vertices += 1

            source_positions.append(expected)
            source_points_all.append(expected)
            blender_points_all.append(actual)

        obj_rms = math.sqrt(obj_sum_sq / len(source_positions))
        per_object.append((obj.name, obj_max, obj_rms))

    rms_vertex_error = math.sqrt(sum_sq / total_vertices)

    source_bbox = bbox(source_points_all)
    blender_bbox = bbox(blender_points_all)

    print("VERTEX_COMPARISON")
    print(f"total_vertices={total_vertices}")
    print(f"max_vertex_error={max_vertex_error:.9g}")
    print(f"rms_vertex_error={rms_vertex_error:.9g}")
    print(f"max_vertex_error_where={max_vertex_where!r}")

    for name, obj_max, obj_rms in per_object:
        print(
            f"  {name}: "
            f"max_error={obj_max:.9g} "
            f"rms_error={obj_rms:.9g}"
        )

    print()
    print("BBOX")
    print(f"source_bbox={fmt_bbox(source_bbox)}")
    print(f"armature_bbox={fmt_bbox(blender_bbox)}")
    print()

    print("SOURCE_BRANCH")
    print(f"active_rot={stats['active_rot']}")
    print(f"active_pos={stats['active_pos']}")
    print(f"rot_end_clamps={stats['rot_end_clamps']}")
    print()

    bpy.context.scene["pt_armature_mapping_probe_v2"] = 1
    bpy.context.scene["pt_armature_mapping_motion_index"] = motion["index"]
    bpy.context.scene["pt_armature_mapping_engine_frame"] = args.engine_frame
    bpy.context.scene["pt_armature_mapping_no_keyframes"] = 1

    args.out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.out))

    pose_ok = final_pose_max_error < 1e-4
    skin_ok = max_vertex_error < 1e-3

    print("VALIDATION")
    print("no_keyframes=YES")
    print(
        f"pose_matrix_mapping="
        f"{'PASS' if pose_ok else 'FAIL'}"
    )
    print(
        f"armature_vs_source_skin="
        f"{'PASS' if skin_ok else 'FAIL'}"
    )
    print(f"output={args.out}")
    print(
        "armature_mapping_probe_check="
        + ("PASS" if pose_ok and skin_ok else "FAIL")
    )


main()

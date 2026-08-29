#!/usr/bin/env python3
# Direct single-pose Blender diagnostic for Priston Tale motion.
# Read-only with respect to source SMD/SMB/INX.
# Opens an existing static .blend and saves a NEW diagnostic .blend.
# No keyframes are created.

import argparse
import importlib.util
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix


def load_core():
    path = Path.cwd() / "pt_blender_apply_motion_v1.py"
    if not path.is_file():
        raise RuntimeError(f"missing dependency: {path}")

    spec = importlib.util.spec_from_file_location("pt_anim_core_v1", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def matrix_max_abs_diff(a, b):
    return max(
        abs(a[r][c] - b[r][c])
        for r in range(4)
        for c in range(4)
    )


def matrix_identity_error(m):
    ident = Matrix.Identity(4)
    return matrix_max_abs_diff(m, ident)


def object_bbox_world(obj, evaluated=False):
    if evaluated:
        depsgraph = bpy.context.evaluated_depsgraph_get()
        eobj = obj.evaluated_get(depsgraph)
        mesh = eobj.to_mesh()
        try:
            mw = eobj.matrix_world
            pts = [mw @ v.co for v in mesh.vertices]
        finally:
            eobj.to_mesh_clear()
    else:
        mw = obj.matrix_world
        pts = [mw @ v.co for v in obj.data.vertices]

    if not pts:
        return None

    xs = [float(p.x) for p in pts]
    ys = [float(p.y) for p in pts]
    zs = [float(p.z) for p in pts]
    return (
        min(xs), max(xs),
        min(ys), max(ys),
        min(zs), max(zs),
    )


def merge_bboxes(boxes):
    boxes = [b for b in boxes if b is not None]
    if not boxes:
        return None
    return (
        min(b[0] for b in boxes), max(b[1] for b in boxes),
        min(b[2] for b in boxes), max(b[3] for b in boxes),
        min(b[4] for b in boxes), max(b[5] for b in boxes),
    )


def fmt_bbox(bb):
    if bb is None:
        return "NONE"
    return (
        f"x[{bb[0]:.4f},{bb[1]:.4f}] "
        f"y[{bb[2]:.4f},{bb[3]:.4f}] "
        f"z[{bb[4]:.4f},{bb[5]:.4f}]"
    )


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--skeleton", required=True, type=Path)
    ap.add_argument("--inx", required=True, type=Path)
    ap.add_argument("--logical-path", required=True)
    ap.add_argument("--motion-index", type=int, default=10)
    ap.add_argument("--engine-frame", type=int)
    ap.add_argument("--armature", default="PT_m5_Armature")
    ap.add_argument("--out", required=True, type=Path)

    raw = sys.argv
    args = ap.parse_args(raw[raw.index("--") + 1:] if "--" in raw else [])

    core = load_core()

    arm = bpy.data.objects.get(args.armature)
    if arm is None or arm.type != "ARMATURE":
        raise RuntimeError(f"armature {args.armature!r} not found")

    smb = core.parse_smb(args.skeleton)
    motion, _ = core.read_inx_motion(
        args.inx,
        args.logical_path,
        args.motion_index,
        smb["header_frames"],
    )

    engine_frame = (
        args.engine_frame
        if args.engine_frame is not None
        else motion["engine_start"]
    )

    if not (
        motion["engine_start"]
        <= engine_frame
        < motion["engine_end"]
    ):
        raise RuntimeError(
            f"engine frame {engine_frame} outside "
            f"[{motion['engine_start']},{motion['engine_end']})"
        )

    visible = [
        o for o in bpy.data.objects
        if o.type == "MESH" and int(o.get("pt_inx_selected", 0)) == 1
    ]
    if not visible:
        visible = [
            o for o in bpy.data.objects
            if o.type == "MESH" and not o.hide_render
        ]

    print("PT_BLENDER_DIRECT_POSE_PROBE_V1")
    print(f"input_blend={bpy.data.filepath}")
    print(f"motion_index={motion['index']}")
    print(f"state={motion['state_name']}")
    print(
        f"motion_engine_range="
        f"[{motion['engine_start']},{motion['engine_end']})"
    )
    print(f"engine_frame={engine_frame}")
    print(f"logical_frame={engine_frame / 160.0:g}")
    print(f"bones={len(smb['bones'])}")
    print(f"visible_meshes={[o.name for o in visible]!r}")
    print()

    mesh_world_matrix_error = max(
        matrix_identity_error(o.matrix_world)
        for o in visible
    )
    arm_world_matrix_error = matrix_identity_error(arm.matrix_world)

    print("OBJECT_TRANSFORMS")
    print(f"armature_world_identity_error={arm_world_matrix_error:.9g}")
    print(f"mesh_world_identity_error_max={mesh_world_matrix_error:.9g}")
    print()

    base_bbox = merge_bboxes([
        object_bbox_world(o, evaluated=False)
        for o in visible
    ])

    arm.animation_data_clear()

    for pb in arm.pose.bones:
        pb.matrix_basis = Matrix.Identity(4)

    bpy.context.view_layer.update()

    results, _, _, stats = core.evaluate_pose(
        smb,
        engine_frame,
        motion["engine_end"],
    )

    desired_deforms = {}

    for idx in smb["tm_sort"]:
        bone = smb["bones"][idx]
        name = bone["name"]

        rest_engine = core.row_engine_to_blender_matrix(bone["tm"])
        pose_engine = core.row_engine_to_blender_matrix(results[idx])

        deform = pose_engine @ rest_engine.inverted()
        desired_deforms[name] = deform

        blender_rest = arm.data.bones[name].matrix_local.copy()

        # Same V1 hypothesis, but applied directly with no keyframe/decompose.
        arm.pose.bones[name].matrix = deform @ blender_rest

    bpy.context.view_layer.update()

    max_deform_error = 0.0
    max_deform_where = None
    det_min = float("inf")
    det_max = float("-inf")

    for idx in smb["tm_sort"]:
        name = smb["bones"][idx]["name"]
        pb = arm.pose.bones[name]
        rest = arm.data.bones[name].matrix_local

        actual = pb.matrix @ rest.inverted()
        desired = desired_deforms[name]

        err = matrix_max_abs_diff(actual, desired)
        if err > max_deform_error:
            max_deform_error = err
            max_deform_where = name

        d = actual.to_3x3().determinant()
        det_min = min(det_min, d)
        det_max = max(det_max, d)

    evaluated_bbox = merge_bboxes([
        object_bbox_world(o, evaluated=True)
        for o in visible
    ])

    print("DIRECT_ASSIGNMENT")
    print(f"max_effective_deform_error={max_deform_error:.9g}")
    print(f"max_effective_deform_error_bone={max_deform_where!r}")
    print(f"effective_deform_det_range={det_min:.9g}..{det_max:.9g}")
    print(f"source_active_rot={stats['active_rot']}")
    print(f"source_active_pos={stats['active_pos']}")
    print(f"source_rot_end_clamps={stats['rot_end_clamps']}")
    print()

    print("VISIBLE_MESH")
    print(f"bind_bbox={fmt_bbox(base_bbox)}")
    print(f"direct_pose_bbox={fmt_bbox(evaluated_bbox)}")
    print()

    bpy.context.scene["pt_direct_pose_probe"] = 1
    bpy.context.scene["pt_direct_pose_motion_index"] = motion["index"]
    bpy.context.scene["pt_direct_pose_engine_frame"] = engine_frame
    bpy.context.scene["pt_direct_pose_no_keyframes"] = 1

    args.out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.out))

    finite_bbox = (
        evaluated_bbox is not None
        and all(math.isfinite(v) for v in evaluated_bbox)
    )

    assignment_ok = max_deform_error < 1e-4

    print("VALIDATION")
    print(f"no_keyframes=YES")
    print(f"effective_deform_assignment={'PASS' if assignment_ok else 'FAIL'}")
    print(f"visible_mesh_bbox_finite={'YES' if finite_bbox else 'NO'}")
    print(f"output={args.out}")
    print(
        "direct_pose_probe_check="
        + ("PASS" if assignment_ok and finite_bbox else "FAIL")
    )


main()

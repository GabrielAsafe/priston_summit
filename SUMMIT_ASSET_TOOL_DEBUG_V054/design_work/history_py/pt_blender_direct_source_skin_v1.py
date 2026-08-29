#!/usr/bin/env python3
# Priston Tale direct source skinning diagnostic V1.
# Opens the already validated V3 static .blend, disables Armature modifiers,
# and writes ONE source-evaluated pose directly into mesh vertex coordinates.
# No keyframes. No source asset modification.

import argparse
import importlib.util
import math
import sys
from pathlib import Path

import bpy


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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--smd", required=True, type=Path)
    ap.add_argument("--skeleton", required=True, type=Path)
    ap.add_argument("--inx", required=True, type=Path)
    ap.add_argument("--logical-path", required=True)
    ap.add_argument("--motion-index", type=int, default=10)
    ap.add_argument("--engine-frame", type=int)
    ap.add_argument("--out", required=True, type=Path)

    raw = sys.argv
    args = ap.parse_args(raw[raw.index("--") + 1:] if "--" in raw else [])

    cwd = Path.cwd()
    core = load_module(
        cwd / "pt_blender_apply_motion_v1.py",
        "pt_anim_core_direct_skin_v1",
    )
    imp = load_module(
        cwd / "pt_smd_blender_import_v3_static_armature.py",
        "pt_smd_import_core_direct_skin_v1",
    )

    smd_data = args.smd.read_bytes()
    smd_meta = imp.parse_header_and_infos(smd_data)
    smd_objects = imp.parse_objects(smd_data, smd_meta)

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

    results, _, _, stats = core.evaluate_pose(
        smb,
        engine_frame,
        motion["engine_end"],
    )

    bone_index = {
        bone["name"]: bone["index"]
        for bone in smb["bones"]
    }

    source_objects = {obj["name"]: obj for obj in smd_objects}

    visible = [
        obj for obj in bpy.data.objects
        if obj.type == "MESH" and int(obj.get("pt_inx_selected", 0)) == 1
    ]
    if not visible:
        visible = [
            obj for obj in bpy.data.objects
            if obj.type == "MESH" and not obj.hide_render
        ]

    print("PT_BLENDER_DIRECT_SOURCE_SKIN_V1")
    print(f"input_blend={bpy.data.filepath}")
    print(f"smd={args.smd}")
    print(f"skeleton={args.skeleton}")
    print(f"motion_index={motion['index']}")
    print(f"state={motion['state_name']}")
    print(f"engine_frame={engine_frame}")
    print(f"logical_frame={engine_frame / 160.0:g}")
    print(f"visible_meshes={[o.name for o in visible]!r}")
    print()

    bind_points = []
    for bobj in visible:
        for v in bobj.data.vertices:
            co = bobj.matrix_world @ v.co
            bind_points.append((float(co.x), float(co.y), float(co.z)))

    bind_bbox = bbox(bind_points)

    changed_vertices = 0
    used_bones = set()
    max_displacement = 0.0
    max_displacement_where = None

    for bobj in visible:
        src = source_objects.get(bobj.name)
        if src is None:
            raise RuntimeError(
                f"Blender object {bobj.name!r} absent from SMD"
            )

        if src["bones"] is None:
            raise RuntimeError(
                f"SMD object {bobj.name!r} has no Physique"
            )

        if len(bobj.data.vertices) != len(src["raw_vertices"]):
            raise RuntimeError(
                f"{bobj.name}: Blender vertices={len(bobj.data.vertices)} "
                f"SMD vertices={len(src['raw_vertices'])}"
            )

        # Disable armature deformation. This diagnostic writes the source
        # world-space pose directly into the mesh coordinates.
        for mod in bobj.modifiers:
            if mod.type == "ARMATURE":
                mod.show_viewport = False
                mod.show_render = False

        for vidx, (raw_local, bone_name) in enumerate(
            zip(src["raw_vertices"], src["bones"])
        ):
            if not bone_name:
                raise RuntimeError(
                    f"{bobj.name} vertex {vidx}: empty Physique bone"
                )

            idx = bone_index.get(bone_name)
            if idx is None:
                raise RuntimeError(
                    f"{bobj.name} vertex {vidx}: "
                    f"bone {bone_name!r} absent from SMB"
                )

            # SMD Physique vertex is stored bone-local in fixed 1/256 units.
            local = (
                raw_local[0] / 256.0,
                raw_local[1] / 256.0,
                raw_local[2] / 256.0,
            )

            world = row_transform_point(local, results[idx])

            old = bobj.data.vertices[vidx].co.copy()
            bobj.data.vertices[vidx].co = world

            d = math.sqrt(
                (world[0] - old.x) ** 2
                + (world[1] - old.y) ** 2
                + (world[2] - old.z) ** 2
            )

            if d > max_displacement:
                max_displacement = d
                max_displacement_where = (
                    bobj.name, vidx, bone_name
                )

            used_bones.add(bone_name)
            changed_vertices += 1

        bobj.data.update()

    bpy.context.view_layer.update()

    pose_points = []
    nonfinite_vertices = 0

    for bobj in visible:
        for v in bobj.data.vertices:
            co = bobj.matrix_world @ v.co
            xyz = (float(co.x), float(co.y), float(co.z))
            pose_points.append(xyz)

            if not all(math.isfinite(x) for x in xyz):
                nonfinite_vertices += 1

    pose_bbox = bbox(pose_points)

    print("SOURCE_SKIN")
    print(f"changed_vertices={changed_vertices}")
    print(f"used_bones={len(used_bones)}")
    print(f"source_active_rot={stats['active_rot']}")
    print(f"source_active_pos={stats['active_pos']}")
    print(f"source_rot_end_clamps={stats['rot_end_clamps']}")
    print(f"nonfinite_vertices={nonfinite_vertices}")
    print(f"max_vertex_displacement={max_displacement:.6f}")
    print(f"max_vertex_displacement_where={max_displacement_where!r}")
    print()

    print("BBOX")
    print(f"bind_bbox={fmt_bbox(bind_bbox)}")
    print(f"source_pose_bbox={fmt_bbox(pose_bbox)}")
    print()

    bpy.context.scene["pt_direct_source_skin_probe"] = 1
    bpy.context.scene["pt_direct_source_skin_motion_index"] = motion["index"]
    bpy.context.scene["pt_direct_source_skin_engine_frame"] = engine_frame
    bpy.context.scene["pt_direct_source_skin_no_keyframes"] = 1
    bpy.context.scene["pt_direct_source_skin_armature_disabled"] = 1

    args.out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.out))

    bbox_finite = all(math.isfinite(v) for v in pose_bbox)
    ok = (
        changed_vertices > 0
        and nonfinite_vertices == 0
        and bbox_finite
    )

    print("VALIDATION")
    print("no_keyframes=YES")
    print("armature_modifiers_disabled=YES")
    print(f"source_pose_bbox_finite={'YES' if bbox_finite else 'NO'}")
    print(f"output={args.out}")
    print(
        "direct_source_skin_check="
        + ("PASS" if ok else "FAIL")
    )


main()

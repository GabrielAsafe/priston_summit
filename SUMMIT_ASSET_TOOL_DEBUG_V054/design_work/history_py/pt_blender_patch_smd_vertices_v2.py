#!/usr/bin/env python3
"""
Priston Tale / Summit Tale - restricted Blender -> SMD vertex patcher V2.

V2 exists because some saved V3 .blend files can contain extra Blender vertices
while the importer metadata and original SMD still describe fewer source vertices.

This tool is intentionally conservative. It only patches SMD XYZ when ALL of
these gates pass:

1. Original SMD object parses structurally.
2. Baseline .blend metadata still matches SMD source counts.
3. Baseline faces are byte-logically identical to SMD face vertex indices/order.
4. Baseline source prefix vertices [0..SMD_NV-1] match the reconstructed SMD
   bind-pose positions and rigid Physique bone assignments.
5. Any Blender-only extra vertices are not referenced by any polygon.
6. Current/test .blend has exactly the same topology as the baseline .blend.
7. Blender-only extra vertices did not move between baseline and current.
8. Only XYZ bytes of changed source vertices are modified in the output SMD.

This is NOT a topology exporter and NOT a universal SMD writer.
"""

import argparse
import hashlib
import json
import math
import struct
import sys
from pathlib import Path

import bpy

SMD_HEADER_TEXT = b"SMD Model data Ver 0.62"

SIZE_DFILE_HEADER = 556
SIZE_DFILE_OBJINFO = 40
SIZE_OBJ3D = 2236
SIZE_VERTEX = 24
SIZE_FACE = 36
SIZE_TEXLINK = 32
SIZE_TM_ROT = 20
SIZE_TM_POS = 16
SIZE_TM_SCALE = 16
SIZE_MATRIX = 64

O_OFF_TEXLINK_PTR = 12
O_OFF_PHYSIQUE_PTR = 16
O_OFF_NVERTEX = 84
O_OFF_NFACE = 88
O_OFF_NTEXLINK = 92
O_OFF_NODENAME = 172
O_OFF_NODEPARENT = 204
O_OFF_TM = 240
O_OFF_TMROTCNT = 684
O_OFF_TMPOSCNT = 688
O_OFF_TMSCALECNT = 692


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def dist3(a, b):
    return math.sqrt(sum((float(a[k]) - float(b[k])) ** 2 for k in range(3)))


def parse_header_and_infos(data):
    if len(data) < SIZE_DFILE_HEADER:
        raise ValueError("file smaller than smDFILE_HEADER")
    header = data[:SIZE_DFILE_HEADER]
    if not header.startswith(SMD_HEADER_TEXT):
        raise ValueError(f"unexpected SMD header: {cstr(header[:24])!r}")

    obj_counter = i32(header, 24)
    mat_counter = i32(header, 28)
    mat_file_point = i32(header, 32)
    first_obj_point = i32(header, 36)

    if not (0 <= obj_counter <= 128):
        raise ValueError(f"implausible ObjCounter={obj_counter}")
    if not (0 <= mat_counter <= 4096):
        raise ValueError(f"implausible MatCounter={mat_counter}")
    if not (SIZE_DFILE_HEADER <= mat_file_point <= first_obj_point <= len(data)):
        raise ValueError("invalid material/object region offsets")

    table_end = SIZE_DFILE_HEADER + obj_counter * SIZE_DFILE_OBJINFO
    if table_end != mat_file_point:
        raise ValueError(
            f"object table end {table_end} != MatFilePoint {mat_file_point}"
        )

    infos = []
    for index in range(obj_counter):
        off = SIZE_DFILE_HEADER + index * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        infos.append({
            "index": index,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })

    return {
        "obj_counter": obj_counter,
        "mat_counter": mat_counter,
        "mat_file_point": mat_file_point,
        "first_obj_point": first_obj_point,
        "infos": infos,
    }


def object_layout(obj_start, nv, nf, nt, nr, np, ns, has_physique):
    vertices = obj_start + SIZE_OBJ3D
    faces = vertices + SIZE_VERTEX * nv
    texlinks = faces + SIZE_FACE * nf
    tmrot = texlinks + SIZE_TEXLINK * nt
    tmpos = tmrot + SIZE_TM_ROT * nr
    tmscale = tmpos + SIZE_TM_POS * np
    prevrot = tmscale + SIZE_TM_SCALE * ns
    physique = prevrot + SIZE_MATRIX * nr if has_physique else None
    end = physique + 32 * nv if has_physique else prevrot + SIZE_MATRIX * nr
    return {
        "vertices": vertices,
        "faces": faces,
        "texlinks": texlinks,
        "physique": physique,
        "end": end,
    }


def parse_object(data, object_name):
    meta = parse_header_and_infos(data)
    info = next((x for x in meta["infos"] if x["name"] == object_name), None)
    if info is None:
        raise ValueError(f"SMD object {object_name!r} not found")

    start = info["start"]
    raw = data[start:start + SIZE_OBJ3D]
    if len(raw) != SIZE_OBJ3D:
        raise ValueError(f"{object_name}: truncated smOBJ3D")

    node_name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
    if node_name != object_name:
        raise ValueError(
            f"object table name {object_name!r} != serialized NodeName {node_name!r}"
        )

    nv = i32(raw, O_OFF_NVERTEX)
    nf = i32(raw, O_OFF_NFACE)
    nt = i32(raw, O_OFF_NTEXLINK)
    nr = i32(raw, O_OFF_TMROTCNT)
    np = i32(raw, O_OFF_TMPOSCNT)
    ns = i32(raw, O_OFF_TMSCALECNT)
    has_physique = u32(raw, O_OFF_PHYSIQUE_PTR) != 0

    if min(nv, nf, nt, nr, np, ns) < 0:
        raise ValueError(f"{object_name}: negative serialized count")
    if not has_physique:
        raise ValueError(f"{object_name}: no serialized Physique; V2 refuses to patch")

    layout = object_layout(start, nv, nf, nt, nr, np, ns, has_physique)
    if layout["end"] != start + info["length"]:
        raise ValueError(
            f"{object_name}: serialized length mismatch: "
            f"computed={layout['end'] - start} table={info['length']}"
        )

    vertices = []
    for idx in range(nv):
        off = layout["vertices"] + idx * SIZE_VERTEX
        vertices.append(struct.unpack_from("<6i", data, off))

    faces = []
    for idx in range(nf):
        off = layout["faces"] + idx * SIZE_FACE
        a, b, c, mat = struct.unpack_from("<4H", data, off)
        faces.append((int(a), int(b), int(c)))

    bones = []
    p = layout["physique"]
    for idx in range(nv):
        bones.append(cstr(data[p + idx * 32:p + (idx + 1) * 32]))

    return {
        "meta": meta,
        "info": info,
        "layout": layout,
        "name": object_name,
        "vertex_count": nv,
        "face_count": nf,
        "texlink_count": nt,
        "vertices": vertices,
        "faces": faces,
        "bones": bones,
    }


def parse_skeleton_tms(data):
    meta = parse_header_and_infos(data)
    if meta["mat_counter"] != 0:
        raise ValueError(
            f"unexpected skeleton MatCounter={meta['mat_counter']}; expected 0"
        )

    bones = {}
    for info in meta["infos"]:
        start = info["start"]
        raw = data[start:start + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise ValueError(f"skeleton object {info['index']}: truncated smOBJ3D")

        name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        if name != info["name"]:
            raise ValueError(
                f"skeleton table name {info['name']!r} != NodeName {name!r}"
            )
        if name in bones:
            raise ValueError(f"duplicate skeleton NodeName {name!r}")

        tm = struct.unpack_from("<16i", raw, O_OFF_TM)
        bones[name] = tm

    return bones


def mval(tm, row, col):
    return tm[(row - 1) * 4 + (col - 1)]


def transform_point_fixed(local_xyz, tm):
    x, y, z = (int(local_xyz[0]), int(local_xyz[1]), int(local_xyz[2]))
    rx = x * mval(tm, 1, 1) + y * mval(tm, 2, 1) + z * mval(tm, 3, 1)
    ry = x * mval(tm, 1, 2) + y * mval(tm, 2, 2) + z * mval(tm, 3, 2)
    rz = x * mval(tm, 1, 3) + y * mval(tm, 2, 3) + z * mval(tm, 3, 3)
    return (
        (rx >> 8) + mval(tm, 4, 1),
        (ry >> 8) + mval(tm, 4, 2),
        (rz >> 8) + mval(tm, 4, 3),
    )


def inverse_3x3_row_vector(world_xyz, tm):
    a, b, c = mval(tm, 1, 1) / 256.0, mval(tm, 1, 2) / 256.0, mval(tm, 1, 3) / 256.0
    d, e, f = mval(tm, 2, 1) / 256.0, mval(tm, 2, 2) / 256.0, mval(tm, 2, 3) / 256.0
    g, h, i = mval(tm, 3, 1) / 256.0, mval(tm, 3, 2) / 256.0, mval(tm, 3, 3) / 256.0

    det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g)
    if abs(det) < 1e-12:
        raise ValueError("singular bone 3x3 matrix")

    inv = (
        ((e * i - f * h) / det, (c * h - b * i) / det, (b * f - c * e) / det),
        ((f * g - d * i) / det, (a * i - c * g) / det, (c * d - a * f) / det),
        ((d * h - e * g) / det, (b * g - a * h) / det, (a * e - b * d) / det),
    )

    qx = world_xyz[0] - mval(tm, 4, 1) / 256.0
    qy = world_xyz[1] - mval(tm, 4, 2) / 256.0
    qz = world_xyz[2] - mval(tm, 4, 3) / 256.0

    return (
        qx * inv[0][0] + qy * inv[1][0] + qz * inv[2][0],
        qx * inv[0][1] + qy * inv[1][1] + qz * inv[2][1],
        qx * inv[0][2] + qy * inv[1][2] + qz * inv[2][2],
    )


def choose_local_fixed(target_world, tm, original_local, radius):
    lf = inverse_3x3_row_vector(target_world, tm)
    center = tuple(int(round(v * 256.0)) for v in lf)

    best = None
    for dx in range(-radius, radius + 1):
        for dy in range(-radius, radius + 1):
            for dz in range(-radius, radius + 1):
                cand = (center[0] + dx, center[1] + dy, center[2] + dz)
                wf = transform_point_fixed(cand, tm)
                world = (wf[0] / 256.0, wf[1] / 256.0, wf[2] / 256.0)
                err2 = sum((world[k] - target_world[k]) ** 2 for k in range(3))
                delta2 = sum((cand[k] - original_local[k]) ** 2 for k in range(3))
                score = (err2, delta2, cand)
                if best is None or score < best[0]:
                    best = (score, cand, world)

    err = math.sqrt(best[0][0])
    return best[1], best[2], err


def matrix_identity_error(matrix_world):
    mx = 0.0
    for r in range(4):
        for c in range(4):
            expected = 1.0 if r == c else 0.0
            mx = max(mx, abs(float(matrix_world[r][c]) - expected))
    return mx


def vertex_has_group(obj, vertex, group_name, min_weight=0.999):
    for elem in vertex.groups:
        if elem.group < len(obj.vertex_groups):
            vg = obj.vertex_groups[elem.group]
            if vg.name == group_name and float(elem.weight) >= min_weight:
                return True
    return False


def snapshot_object(obj):
    return {
        "vertex_count": len(obj.data.vertices),
        "face_count": len(obj.data.polygons),
        "coords": [tuple(float(x) for x in v.co) for v in obj.data.vertices],
        "faces": [tuple(int(x) for x in p.vertices) for p in obj.data.polygons],
        "matrix_identity_error": matrix_identity_error(obj.matrix_world),
        "saved_vertex_count": obj.get("pt_smd_vertex_count"),
        "saved_face_count": obj.get("pt_smd_face_count"),
    }


def load_baseline_object(path, object_name):
    current = Path(bpy.data.filepath).resolve() if bpy.data.filepath else None
    if current is not None and current == path:
        obj = bpy.data.objects.get(object_name)
        if obj is None:
            raise ValueError(f"current baseline object {object_name!r} not found")
        return obj, False

    with bpy.data.libraries.load(str(path), link=False) as (data_from, data_to):
        if object_name not in data_from.objects:
            raise ValueError(
                f"baseline blend does not contain object {object_name!r}"
            )
        data_to.objects = [object_name]

    obj = data_to.objects[0]
    if obj is None:
        raise ValueError(f"failed to append baseline object {object_name!r}")
    return obj, True


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True, help="Original/template MfbC01.smd")
    p.add_argument("--skeleton", required=True, help="Companion m5.smb")
    p.add_argument("--baseline-blend", required=True,
                   help="Unedited V3 baseline .blend used to validate source indices")
    p.add_argument("--object", default="MsmC01")
    p.add_argument("--out", required=True)
    p.add_argument("--epsilon", type=float, default=1e-6,
                   help="Change threshold between baseline and current Blender coords")
    p.add_argument("--source-match-tolerance", type=float, default=2e-5,
                   help="Required baseline source-prefix match to reconstructed SMD")
    p.add_argument("--search-radius", type=int, default=3,
                   help="Integer local-coordinate search radius around inverse solution")
    p.add_argument("--max-fit-error", type=float, default=0.02,
                   help="Reject changed source vertex if fixed-point fit exceeds this")
    p.add_argument("--expect-changed", type=int,
                   help="Require exactly N patched source vertices")
    p.add_argument("--report", help="Optional JSON report path")
    return p.parse_args(argv)


def main(argv):
    args = parse_args(argv)

    smd_path = Path(args.smd).expanduser().resolve()
    skeleton_path = Path(args.skeleton).expanduser().resolve()
    baseline_path = Path(args.baseline_blend).expanduser().resolve()
    out_path = Path(args.out).expanduser().resolve()

    if out_path == smd_path:
        raise ValueError("refusing in-place SMD overwrite; choose a different --out")
    if args.search_radius < 0 or args.search_radius > 10:
        raise ValueError("--search-radius must be between 0 and 10")
    if not baseline_path.is_file():
        raise ValueError(f"baseline blend not found: {baseline_path}")

    src = smd_path.read_bytes()
    skeleton_data = skeleton_path.read_bytes()
    parsed = parse_object(src, args.object)
    skeleton = parse_skeleton_tms(skeleton_data)

    current_obj = bpy.data.objects.get(args.object)
    if current_obj is None:
        raise ValueError(f"current Blender object {args.object!r} not found")
    if current_obj.type != "MESH":
        raise ValueError(f"current object {args.object!r} is {current_obj.type}, expected MESH")

    current = snapshot_object(current_obj)

    baseline_obj, appended = load_baseline_object(baseline_path, args.object)
    if baseline_obj.type != "MESH":
        raise ValueError(f"baseline object {args.object!r} is {baseline_obj.type}, expected MESH")
    baseline = snapshot_object(baseline_obj)

    nv = parsed["vertex_count"]
    nf = parsed["face_count"]

    # Metadata must still say what the importer said, even if Blender mesh has extras.
    if baseline["saved_vertex_count"] is not None and int(baseline["saved_vertex_count"]) != nv:
        raise ValueError(
            f"baseline metadata pt_smd_vertex_count={baseline['saved_vertex_count']} "
            f"does not match SMD={nv}"
        )
    if baseline["saved_face_count"] is not None and int(baseline["saved_face_count"]) != nf:
        raise ValueError(
            f"baseline metadata pt_smd_face_count={baseline['saved_face_count']} "
            f"does not match SMD={nf}"
        )

    if baseline["matrix_identity_error"] > 1e-7:
        raise ValueError(
            f"baseline object matrix is not identity "
            f"(error={baseline['matrix_identity_error']:.9g})"
        )
    if current["matrix_identity_error"] > 1e-7:
        raise ValueError(
            f"current object matrix is not identity "
            f"(error={current['matrix_identity_error']:.9g}); use Edit Mode only"
        )

    if baseline["vertex_count"] < nv:
        raise ValueError(
            f"baseline has fewer vertices ({baseline['vertex_count']}) than SMD ({nv})"
        )
    if baseline["face_count"] != nf:
        raise ValueError(
            f"baseline face count differs from SMD: Blender={baseline['face_count']} SMD={nf}"
        )

    # Strong topology gate: polygons must be exactly the imported SMD face tuples.
    face_mismatches = []
    for i, (bf, sf) in enumerate(zip(baseline["faces"], parsed["faces"])):
        if tuple(bf) != tuple(sf):
            face_mismatches.append((i, bf, sf))
            if len(face_mismatches) >= 10:
                break
    if face_mismatches:
        raise ValueError(
            f"baseline topology is not source-identical; first mismatch={face_mismatches[0]}"
        )

    # Because every face tuple is source-identical, any index >= nv must be loose.
    referenced = {vid for face in baseline["faces"] for vid in face}
    extra_indices = list(range(nv, baseline["vertex_count"]))
    referenced_extras = [i for i in extra_indices if i in referenced]
    if referenced_extras:
        raise ValueError(
            f"baseline extra vertices are referenced by faces: {referenced_extras[:20]}"
        )

    # Validate that source prefix 0..nv-1 is still the exact imported bind pose.
    max_source_prefix_error = 0.0
    bad_source_prefix = []
    bad_bone_groups = []

    for idx in range(nv):
        bone_name = parsed["bones"][idx]
        tm = skeleton.get(bone_name)
        if tm is None:
            raise ValueError(f"SMD vertex {idx}: bone {bone_name!r} missing from SMB")

        local = parsed["vertices"][idx][:3]
        wf = transform_point_fixed(local, tm)
        expected = (wf[0] / 256.0, wf[1] / 256.0, wf[2] / 256.0)
        actual = baseline["coords"][idx]
        err = dist3(expected, actual)
        max_source_prefix_error = max(max_source_prefix_error, err)
        if err > args.source_match_tolerance:
            bad_source_prefix.append((idx, err, expected, actual))
            if len(bad_source_prefix) >= 10:
                break

        if not vertex_has_group(baseline_obj, baseline_obj.data.vertices[idx], bone_name):
            bad_bone_groups.append((idx, bone_name))
            if len(bad_bone_groups) >= 10:
                break

    if bad_source_prefix:
        raise ValueError(
            "baseline source-prefix vertices no longer match SMD bind pose; "
            f"first={bad_source_prefix[0]}"
        )
    if bad_bone_groups:
        raise ValueError(
            "baseline source-prefix Physique group mismatch; "
            f"first={bad_bone_groups[0]}"
        )

    # Current test must preserve the entire baseline topology.
    if current["vertex_count"] != baseline["vertex_count"]:
        raise ValueError(
            f"current vertex count differs from baseline: "
            f"current={current['vertex_count']} baseline={baseline['vertex_count']}"
        )
    if current["face_count"] != baseline["face_count"]:
        raise ValueError(
            f"current face count differs from baseline: "
            f"current={current['face_count']} baseline={baseline['face_count']}"
        )

    current_topology_mismatch = []
    for i, (cf, bf) in enumerate(zip(current["faces"], baseline["faces"])):
        if tuple(cf) != tuple(bf):
            current_topology_mismatch.append((i, cf, bf))
            if len(current_topology_mismatch) >= 10:
                break
    if current_topology_mismatch:
        raise ValueError(
            f"current topology differs from baseline; first={current_topology_mismatch[0]}"
        )

    # Blender-only extras cannot be represented in this restricted patch.
    changed_extras = []
    for idx in extra_indices:
        move = dist3(current["coords"][idx], baseline["coords"][idx])
        if move > args.epsilon:
            changed_extras.append((idx, move))
    if changed_extras:
        raise ValueError(
            "Blender-only extra vertices moved, which V2 cannot serialize; "
            f"first={changed_extras[0]}"
        )

    patched = bytearray(src)
    changes = []
    max_fit_error = 0.0

    for idx in range(nv):
        baseline_world = baseline["coords"][idx]
        current_world = current["coords"][idx]
        move = dist3(current_world, baseline_world)
        if move <= args.epsilon:
            continue

        serialized = parsed["vertices"][idx]
        original_local = serialized[:3]
        bone_name = parsed["bones"][idx]
        tm = skeleton[bone_name]

        new_local, represented_world, fit_error = choose_local_fixed(
            current_world, tm, original_local, args.search_radius
        )
        max_fit_error = max(max_fit_error, fit_error)
        if fit_error > args.max_fit_error:
            raise ValueError(
                f"vertex {idx}: fixed-point fit error {fit_error:.9g} exceeds "
                f"--max-fit-error {args.max_fit_error}"
            )

        off = parsed["layout"]["vertices"] + idx * SIZE_VERTEX
        struct.pack_into("<3i", patched, off, *new_local)

        changes.append({
            "index": idx,
            "bone": bone_name,
            "move_from_baseline": move,
            "baseline_world": baseline_world,
            "edited_world": current_world,
            "represented_world": represented_world,
            "fit_error": fit_error,
            "original_local_fixed": original_local,
            "new_local_fixed": new_local,
        })

    if args.expect_changed is not None and len(changes) != args.expect_changed:
        raise ValueError(
            f"changed source vertex count {len(changes)} != --expect-changed {args.expect_changed}"
        )

    allowed = set()
    for ch in changes:
        off = parsed["layout"]["vertices"] + ch["index"] * SIZE_VERTEX
        allowed.update(range(off, off + 12))

    diff_offsets = [i for i, (a, b) in enumerate(zip(src, patched)) if a != b]
    illegal = [i for i in diff_offsets if i not in allowed]

    if len(src) != len(patched):
        raise ValueError("internal error: output size changed")
    if illegal:
        raise ValueError(
            f"internal error: {len(illegal)} changed bytes outside allowed XYZ fields"
        )

    reparsed = parse_object(bytes(patched), args.object)
    if reparsed["vertex_count"] != nv or reparsed["face_count"] != nf:
        raise ValueError("post-patch structural count mismatch")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(patched)

    report = {
        "tool": "pt_blender_patch_smd_vertices_v2",
        "blend": bpy.data.filepath,
        "baseline_blend": str(baseline_path),
        "object": args.object,
        "input_smd": str(smd_path),
        "output_smd": str(out_path),
        "input_size": len(src),
        "output_size": len(patched),
        "input_sha256": sha256_bytes(src),
        "output_sha256": sha256_bytes(patched),
        "byte_identical": src == bytes(patched),
        "smd_vertex_count": nv,
        "smd_face_count": nf,
        "baseline_vertex_count": baseline["vertex_count"],
        "baseline_face_count": baseline["face_count"],
        "baseline_metadata_vertex_count": baseline["saved_vertex_count"],
        "baseline_metadata_face_count": baseline["saved_face_count"],
        "extra_vertex_count": len(extra_indices),
        "extra_vertex_indices": extra_indices,
        "referenced_extra_vertices": referenced_extras,
        "changed_extra_vertices": changed_extras,
        "max_source_prefix_error": max_source_prefix_error,
        "changed_vertices": len(changes),
        "changed_bytes": len(diff_offsets),
        "max_fit_error": max_fit_error,
        "changes": changes,
    }

    if args.report:
        report_path = Path(args.report).expanduser().resolve()
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print("PT_BLENDER_PATCH_SMD_VERTICES_V2")
    print("blend=", bpy.data.filepath)
    print("baseline_blend=", baseline_path)
    print("object=", args.object)
    print("smd_vertices=", nv)
    print("baseline_vertices=", baseline["vertex_count"])
    print("smd_faces=", nf)
    print("baseline_faces=", baseline["face_count"])
    print("metadata_vertices=", baseline["saved_vertex_count"])
    print("metadata_faces=", baseline["saved_face_count"])
    print("extra_vertices=", len(extra_indices))
    print("extra_indices=", extra_indices)
    print("referenced_extra_vertices=", referenced_extras)
    print("changed_extra_vertices=", changed_extras)
    print("max_source_prefix_error=", f"{max_source_prefix_error:.9g}")
    print("changed_vertices=", len(changes))
    print("changed_bytes=", len(diff_offsets))
    print("max_fit_error=", f"{max_fit_error:.9g}")
    print("input_size=", len(src))
    print("output_size=", len(patched))
    print("input_sha256=", report["input_sha256"])
    print("output_sha256=", report["output_sha256"])
    print("byte_identical=", report["byte_identical"])

    for ch in changes:
        print(
            "PATCH",
            f"v={ch['index']}",
            f"bone={ch['bone']!r}",
            f"move={ch['move_from_baseline']:.9g}",
            f"fit={ch['fit_error']:.9g}",
            f"local={tuple(ch['original_local_fixed'])}->{tuple(ch['new_local_fixed'])}",
        )

    print("patch_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"patch_check=FAIL: {type(exc).__name__}: {exc}")
        raise

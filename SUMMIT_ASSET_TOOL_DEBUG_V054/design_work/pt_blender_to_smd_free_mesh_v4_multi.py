#!/usr/bin/env python3
"""
PT SMD Model 0.62 Free Mesh Writer V4 Multi-Object

Rebuilds multiple selected mesh object blocks from ONE Blender scene and ONE
pristine/source SMD, then writes ONE final SMD.

Important:
- target blocks are not serialized one after another through intermediate SMDs;
- every target is rebuilt against the same source SMD and skeleton;
- the object table is recalculated once for the final combined file;
- every non-target object block remains byte-identical;
- output is written only after all targets pass.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import math
import pathlib
import statistics
import struct
import sys
from collections import defaultdict


def load_core():
    path = pathlib.Path(__file__).resolve().with_name("pt_blender_to_smd_free_mesh_v3.py")
    if not path.is_file():
        raise FileNotFoundError(path)
    spec = importlib.util.spec_from_file_location("pt_free_mesh_writer_v3_core", path)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)
    return mod


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--skeleton", required=True)
    p.add_argument("--object", action="append", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--report")
    p.add_argument("--max-fit-error", type=float, default=0.02)
    p.add_argument("--expected-source-sha256")
    return p.parse_args(argv)


def build_target_block(core, src, infos, skeleton, target, max_fit_error):
    so = core.parse_object(src, infos, target)
    if not so["phys"]:
        raise ValueError(f"{target}: source object has no Physique")
    if so["nr"] or so["np"] or so["ns"]:
        raise ValueError(
            f"{target}: object-local animation arrays unsupported "
            f"(nr={so['nr']}, np={so['np']}, ns={so['ns']})"
        )

    templates, htex_by_mat = core.material_templates(so)
    allowed_materials = set(templates)

    obj = core.bpy.data.objects.get(target)
    if obj is None or obj.type != "MESH":
        raise ValueError(f"{target}: Blender mesh missing")

    mode_sync = core.sync_mesh_from_edit_mode(obj)
    mesh = obj.data
    mesh.update()

    nv = len(mesh.vertices)
    source_polygons = len(mesh.polygons)
    if not 3 <= nv <= core.MAX_VERTICES:
        raise ValueError(f"{target}: vertex count {nv} outside safe range")
    if source_polygons < 1:
        raise ValueError(f"{target}: mesh has no polygons")
    if nv > 65535:
        raise ValueError(f"{target}: uint16 vertex-index overflow")

    core.require_uv_storage(mesh)
    export_triangles, triangulation_info = core.build_export_triangles(mesh)
    nf = len(export_triangles)
    if not 1 <= nf <= core.MAX_FACES:
        raise ValueError(f"{target}: triangulated face count {nf} outside safe range")

    if not core.object_matrix_is_identity(obj):
        raise ValueError(f"{target}: object transform is not identity")

    for fi, tri in enumerate(export_triangles):
        mat = tri["material_index"]
        if mat not in allowed_materials:
            raise ValueError(
                f"{target}: triangle {fi} material {mat} absent from source; "
                f"allowed={sorted(allowed_materials)}"
            )

    bones = []
    for vi in range(nv):
        bone = core.rigid_bone(obj, vi)
        if bone not in skeleton:
            raise ValueError(f"{target}: vertex {vi} bone {bone!r} missing from skeleton")
        bones.append(bone)

    eqn = core.equal_normals(mesh, export_triangles)

    mags_by_bone = defaultdict(list)
    all_mags = []
    for vi, sv in enumerate(so["vertices"]):
        nx, ny, nz = sv[3:6]
        mag = math.sqrt(nx * nx + ny * ny + nz * nz)
        if mag > 0:
            mags_by_bone[so["bones"][vi]].append(mag)
            all_mags.append(mag)
    if not all_mags:
        raise ValueError(f"{target}: source normal calibration is empty")
    fallback_mag = statistics.median(all_mags)

    vertices = bytearray()
    physique = bytearray()
    fit_errors = []

    for vi in range(nv):
        bone = bones[vi]
        tm = skeleton[bone]
        world = tuple(float(x) for x in mesh.vertices[vi].co)

        local, represented, fit = core.fit_local(world, tm)
        if fit > max_fit_error:
            raise ValueError(
                f"{target}: vertex {vi} bind-position fit {fit} exceeds "
                f"{max_fit_error}"
            )

        local_dir = core.normalize(core.inv_row3(eqn[vi], tm))
        scale = (
            statistics.median(mags_by_bone[bone])
            if mags_by_bone.get(bone)
            else fallback_mag
        )
        normal = tuple(int(round(x * scale)) for x in local_dir)

        vertices.extend(struct.pack("<6i", *local, *normal))
        physique.extend(core.bone_slot(bone))
        fit_errors.append(fit)

    faces = bytearray()
    tex = bytearray()
    for fi, tri in enumerate(export_triangles):
        ids = tri["vertices"]
        if any(v < 0 or v >= nv for v in ids):
            raise ValueError(f"{target}: triangle {fi} invalid vertex index")
        if any(v > 65535 for v in ids):
            raise ValueError(f"{target}: uint16 face-index overflow")

        mat = tri["material_index"]
        face_template, tex_template = templates[mat]
        ptr = so["texbase"] + fi * core.SZ_TEX
        if ptr > 0xFFFFFFFF:
            raise ValueError(f"{target}: historical TexLink pointer overflow")

        faces.extend(core.face_blob(face_template, ids, mat, ptr))
        tex.extend(
            core.tex_blob(
                tex_template,
                core.blender_uvs_smd(tri["uvs"]),
                htex_by_mat[mat],
            )
        )

    nt = nf
    raw = bytearray(so["raw"])
    for off, val in (
        (core.O_VERTEX_CAP, nv),
        (core.O_FACE_CAP, nf),
        (core.O_NV, nv),
        (core.O_NF, nf),
        (core.O_NT, nt),
    ):
        struct.pack_into("<i", raw, off, val)

    block = bytes(raw) + bytes(vertices) + bytes(faces) + bytes(tex) + bytes(physique)
    expected_size = (
        core.SZ_OBJ
        + nv * core.SZ_VERT
        + nf * core.SZ_FACE
        + nt * core.SZ_TEX
        + nv * 32
    )
    if len(block) != expected_size:
        raise ValueError(f"{target}: block size mismatch")

    return block, {
        "object": target,
        "mode_sync": mode_sync,
        "source_counts": {
            "vertices": so["nv"],
            "faces": so["nf"],
            "texlinks": so["nt"],
        },
        "artist_mesh": {
            "vertices": nv,
            "source_polygons": source_polygons,
            "export_triangles": nf,
            "triangulation_delta": nf - source_polygons,
            "triangulation": triangulation_info,
        },
        "output_counts": {
            "vertices": nv,
            "faces": nf,
            "texlinks": nt,
        },
        "vertex_delta": nv - so["nv"],
        "face_delta": nf - so["nf"],
        "allowed_materials": sorted(allowed_materials),
        "hTexture_by_material": {str(k): v for k, v in htex_by_mat.items()},
        "unique_rigid_bones": sorted(set(bones)),
        "rigid_bone_count": len(set(bones)),
        "max_vertex_fit_error": max(fit_errors) if fit_errors else 0.0,
        "mean_vertex_fit_error": (
            sum(fit_errors) / len(fit_errors) if fit_errors else 0.0
        ),
        "all_vertices_rebuilt": True,
        "all_faces_rebuilt": True,
        "all_texlinks_rebuilt": True,
        "all_physique_rebuilt": True,
    }


def main(argv):
    a = parse_args(argv)
    targets = list(dict.fromkeys(a.object))
    if len(targets) != len(a.object):
        raise ValueError("duplicate --object values are not allowed")

    core = load_core()
    smd = pathlib.Path(a.smd).expanduser().resolve()
    skel = pathlib.Path(a.skeleton).expanduser().resolve()
    outp = pathlib.Path(a.out).expanduser().resolve()

    src = smd.read_bytes()
    source_sha = core.sha(src)
    if a.expected_source_sha256 and source_sha != a.expected_source_sha256.lower():
        raise ValueError(
            f"source SMD SHA mismatch: {source_sha} != "
            f"{a.expected_source_sha256.lower()}"
        )

    infos = core.parse_file(src)
    available = {x["name"] for x in infos}
    missing = [x for x in targets if x not in available]
    if missing:
        raise ValueError(f"target objects missing from SMD: {missing}")

    skeleton = core.parse_skeleton(skel.read_bytes())

    replacement_blocks = {}
    object_reports = {}
    for target in targets:
        print(f"\n--- BUILD {target} ---")
        block, report = build_target_block(
            core, src, infos, skeleton, target, a.max_fit_error
        )
        replacement_blocks[target] = block
        object_reports[target] = report
        c = report["output_counts"]
        print(
            f"{target}: source="
            f"{report['source_counts']['vertices']}/"
            f"{report['source_counts']['faces']}/"
            f"{report['source_counts']['texlinks']} "
            f"output={c['vertices']}/{c['faces']}/{c['texlinks']} "
            f"materials={report['allowed_materials']} "
            f"bones={report['rigid_bone_count']}"
        )

    prefix = bytearray(src[:infos[0]["start"]])
    blocks = []
    cursor = infos[0]["start"]
    table = []

    for inf in infos:
        block = replacement_blocks.get(
            inf["name"],
            src[inf["start"]:inf["start"] + inf["length"]],
        )
        blocks.append(block)
        struct.pack_into("<i", prefix, inf["table_off"] + 32, len(block))
        struct.pack_into("<i", prefix, inf["table_off"] + 36, cursor)
        table.append({
            "name": inf["name"],
            "target": inf["name"] in replacement_blocks,
            "old_start": inf["start"],
            "new_start": cursor,
            "old_length": inf["length"],
            "new_length": len(block),
        })
        cursor += len(block)

    out = bytes(prefix) + b"".join(blocks)
    out_infos = core.parse_file(out)

    # Verify all target blocks semantically.
    for target in targets:
        c = object_reports[target]["output_counts"]
        core.verify_output(out, target, c["vertices"], c["faces"])

    # Verify all non-target blocks byte-identical.
    non_target_identical = 0
    for old, new in zip(infos, out_infos):
        if old["name"] in replacement_blocks:
            continue
        old_block = src[old["start"]:old["start"] + old["length"]]
        new_block = out[new["start"]:new["start"] + new["length"]]
        if old_block != new_block:
            raise ValueError(f"non-target object changed: {old['name']}")
        non_target_identical += 1

    outp.parent.mkdir(parents=True, exist_ok=True)
    outp.write_bytes(out)

    report = {
        "tool": "PT_SMD_MODEL_062_FREE_MESH_WRITER_V4_MULTI",
        "blend": core.bpy.data.filepath,
        "source_smd": str(smd),
        "source_sha256": source_sha,
        "targets": targets,
        "target_count": len(targets),
        "objects": [object_reports[x] for x in targets],
        "non_target_object_blocks_byte_identical": non_target_identical,
        "input_size": len(src),
        "output_size": len(out),
        "growth_bytes": len(out) - len(src),
        "input_sha256": source_sha,
        "output_sha256": core.sha(out),
        "object_table": table,
        "semantic_check": "PASS",
        "multi_object_writer_check": "PASS",
    }

    rp = (
        pathlib.Path(a.report).expanduser().resolve()
        if a.report else outp.with_suffix(".json")
    )
    rp.parent.mkdir(parents=True, exist_ok=True)
    rp.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")

    print("\nPT_SMD_MODEL_062_FREE_MESH_WRITER_V4_MULTI")
    print(f"targets={targets}")
    print(f"target_count={len(targets)}")
    for target in targets:
        row = object_reports[target]
        c = row["output_counts"]
        print(
            f"OBJECT {target}: "
            f"V={c['vertices']} F={c['faces']} T={c['texlinks']} "
            f"vertex_delta={row['vertex_delta']:+d} "
            f"face_delta={row['face_delta']:+d} "
            f"materials={row['allowed_materials']} "
            f"bones={row['rigid_bone_count']} "
            f"check=PASS"
        )
    print(f"non_target_object_blocks_byte_identical={non_target_identical}")
    print(f"input_size={len(src)}")
    print(f"output_size={len(out)}")
    print(f"growth_bytes={len(out)-len(src):+d}")
    print(f"input_sha256={source_sha}")
    print(f"output_sha256={core.sha(out)}")
    print(f"report={rp}")
    print("semantic_check=PASS")
    print("multi_object_writer_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"multi_object_writer_check=FAIL: {type(exc).__name__}: {exc}")
        raise

#!/usr/bin/env python3
"""
PT SMD MODEL 0.62 — FREE TOPOLOGY REINDEX WRITER V1

Gate scope
----------
This writer intentionally handles only the first free-topology proof:

  MshC01 pristine semantic mesh
  -> all vertex indices changed
  -> all face-array positions changed
  -> visually identical topology/content
  -> rebuilt smVERTEX order
  -> rebuilt smFACE indices/order
  -> rebuilt Physique order
  -> source TexLinks preserved byte-exact and still referenced by each face

It does NOT write the client and it does NOT yet accept arbitrary counts.
That comes after this reindex gate proves that vertex/face identity is not
externally required by runtime.

Other 14 object blocks remain byte-identical.
"""

import argparse
import hashlib
import json
import math
import struct
import sys
from pathlib import Path

import bpy

SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
TARGET = "MshC01"

SZ_HEADER = 556
SZ_OBJINFO = 40
SZ_OBJ = 2236
SZ_VERTEX = 24
SZ_FACE = 36
SZ_TEXLINK = 32
SZ_TM_ROT = 20
SZ_TM_POS = 16
SZ_TM_SCALE = 16
SZ_MATRIX = 64

OFF_PHYSIQUE_PTR = 16
OFF_NV_MIRROR = 76
OFF_NF_MIRROR = 80
OFF_NV = 84
OFF_NF = 88
OFF_NT = 92
OFF_NAME = 172
OFF_TMROT = 684
OFF_TMPOS = 688
OFF_TMSCALE = 692

SOURCE_V = 173
SOURCE_F = 331
SOURCE_T = 331


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", default="MfbC01.smd")
    p.add_argument("--object", default=TARGET)
    p.add_argument("--out", default="pt_build/C01_topology/MfbC01_high_free_topology_reindex_v1.smd")
    p.add_argument("--report", default="pt_build/C01_topology/MfbC01_high_free_topology_reindex_v1_report.json")
    return p.parse_args(argv)


def i32(buf, off):
    return struct.unpack_from("<i", buf, off)[0]


def u32(buf, off):
    return struct.unpack_from("<I", buf, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="strict")


def parse_csv_ints(value, expected_count, label):
    if not isinstance(value, str):
        raise RuntimeError(f"{label}: expected string metadata")
    rows = [int(x) for x in value.split(",") if x != ""]
    if len(rows) != expected_count:
        raise RuntimeError(
            f"{label}: expected {expected_count} values; got {len(rows)}"
        )
    return rows


def polygon_uvs(mesh, poly, uv_layer):
    return [
        (float(uv_layer.data[li].uv.x), float(uv_layer.data[li].uv.y))
        for li in poly.loop_indices
    ]


def canonical_mesh_digest(obj):
    mesh = obj.data
    if mesh.uv_layers.active is None:
        raise RuntimeError("active UV layer missing")
    uv = mesh.uv_layers.active

    h = hashlib.sha256()
    h.update(b"PT_FREE_TOPOLOGY_REINDEX_STATE_V1\0")
    h.update(struct.pack("<II", len(mesh.vertices), len(mesh.polygons)))

    for v in mesh.vertices:
        h.update(struct.pack("<3f", float(v.co.x), float(v.co.y), float(v.co.z)))

        rows = []
        for e in v.groups:
            if e.weight <= 1e-8:
                continue
            name = obj.vertex_groups[e.group].name
            rows.append((name, float(e.weight)))
        rows.sort(key=lambda x: x[0])

        h.update(struct.pack("<I", len(rows)))
        for name, weight in rows:
            raw = name.encode("utf-8")
            h.update(struct.pack("<I", len(raw)))
            h.update(raw)
            h.update(struct.pack("<f", weight))

    for p in mesh.polygons:
        ids = tuple(int(x) for x in p.vertices)
        if len(ids) != 3:
            raise RuntimeError(f"polygon {p.index}: non-triangle")
        h.update(struct.pack("<3I", *ids))
        h.update(struct.pack("<I?", int(p.material_index), bool(p.use_smooth)))

        for u, v in polygon_uvs(mesh, p, uv):
            h.update(struct.pack("<2f", u, v))

    return h.hexdigest()


def rigid_bone(obj, vi):
    rows = []
    for e in obj.data.vertices[vi].groups:
        if e.weight <= 1e-8:
            continue
        rows.append((obj.vertex_groups[e.group].name, float(e.weight)))

    if len(rows) != 1 or abs(rows[0][1] - 1.0) > 1e-5:
        raise RuntimeError(
            f"vertex {vi}: expected exactly one rigid group weight 1.0; got {rows}"
        )
    return rows[0][0]


def parse_object_table(data):
    if len(data) < SZ_HEADER:
        raise RuntimeError("SMD too small")
    if not data.startswith(b"SMD Model data Ver 0.62"):
        raise RuntimeError("unexpected SMD header")

    obj_count = i32(data, 24)
    if obj_count <= 0 or obj_count > 128:
        raise RuntimeError(f"implausible ObjCounter={obj_count}")

    rows = []
    for i in range(obj_count):
        off = SZ_HEADER + i * SZ_OBJINFO
        raw = data[off:off + SZ_OBJINFO]
        if len(raw) != SZ_OBJINFO:
            raise RuntimeError("truncated object table")
        rows.append({
            "index": i,
            "table_off": off,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })
    return rows


def object_layout(data, row):
    start = row["start"]
    length = row["length"]
    end = start + length
    if start < 0 or length <= 0 or end > len(data):
        raise RuntimeError(f"{row['name']}: invalid object bounds")

    head = data[start:start + SZ_OBJ]
    if len(head) != SZ_OBJ:
        raise RuntimeError(f"{row['name']}: truncated smOBJ3D")

    if cstr(head[OFF_NAME:OFF_NAME + 32]) != row["name"]:
        raise RuntimeError(f"{row['name']}: NodeName mismatch")

    nv = i32(head, OFF_NV)
    nf = i32(head, OFF_NF)
    nt = i32(head, OFF_NT)
    nr = i32(head, OFF_TMROT)
    np = i32(head, OFF_TMPOS)
    ns = i32(head, OFF_TMSCALE)
    physique_ptr = u32(head, OFF_PHYSIQUE_PTR)

    if min(nv, nf, nt, nr, np, ns) < 0:
        raise RuntimeError(f"{row['name']}: negative count")

    vertex_off = start + SZ_OBJ
    face_off = vertex_off + nv * SZ_VERTEX
    tex_off = face_off + nf * SZ_FACE
    rot_off = tex_off + nt * SZ_TEXLINK
    pos_off = rot_off + nr * SZ_TM_ROT
    scale_off = pos_off + np * SZ_TM_POS
    prevrot_off = scale_off + ns * SZ_TM_SCALE
    physique_off = prevrot_off + nr * SZ_MATRIX
    calc_end = physique_off + (nv * 32 if physique_ptr else 0)

    if calc_end != end:
        raise RuntimeError(
            f"{row['name']}: computed end {calc_end} != object end {end}"
        )

    return {
        "head": head,
        "nv": nv,
        "nf": nf,
        "nt": nt,
        "nr": nr,
        "np": np,
        "ns": ns,
        "physique_ptr": physique_ptr,
        "vertex_off": vertex_off,
        "face_off": face_off,
        "tex_off": tex_off,
        "rot_off": rot_off,
        "pos_off": pos_off,
        "scale_off": scale_off,
        "prevrot_off": prevrot_off,
        "physique_off": physique_off,
        "end": end,
    }


def source_physique_names(data, layout):
    if not layout["physique_ptr"]:
        raise RuntimeError("MshC01 has no Physique")
    out = []
    for vi in range(layout["nv"]):
        off = layout["physique_off"] + vi * 32
        out.append(cstr(data[off:off + 32]))
    return out


def texlink_uv(data, tex_off, ti):
    off = tex_off + ti * SZ_TEXLINK
    u = struct.unpack_from("<3f", data, off + 0)
    v = struct.unpack_from("<3f", data, off + 12)
    return u, v


def infer_texlink_index(face_raw, texlinks, old_face_index):
    # Real C01 has one TexLink per face and source face i points to TexLink i.
    # We still validate the historical pointer stride instead of assuming silently.
    ptrs = [u32(face_raw, i * SZ_FACE + 32) for i in range(SOURCE_F)]
    base = ptrs[0]
    expected = base + old_face_index * SZ_TEXLINK
    if ptrs[old_face_index] != expected:
        raise RuntimeError(
            f"source face {old_face_index}: TexLink pointer is not base+index*32"
        )
    return old_face_index, base


def main(argv):
    args = parse_args(argv)
    if args.object != TARGET:
        raise RuntimeError("V1 supports only MshC01")

    source = Path(args.smd).expanduser().resolve()
    data = source.read_bytes()
    input_sha = hashlib.sha256(data).hexdigest()
    if input_sha != SOURCE_SHA:
        raise RuntimeError(f"unexpected pristine SMD SHA256: {input_sha}")

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    mesh = obj.data
    mesh.update()

    if (len(mesh.vertices), len(mesh.polygons)) != (SOURCE_V, SOURCE_F):
        raise RuntimeError(
            f"reindex gate requires {SOURCE_V}/{SOURCE_F}; "
            f"got {len(mesh.vertices)}/{len(mesh.polygons)}"
        )
    if len(mesh.uv_layers) != 1 or mesh.uv_layers.active is None:
        raise RuntimeError("requires exactly one active UV layer")
    for p in mesh.polygons:
        if len(p.vertices) != 3:
            raise RuntimeError(f"polygon {p.index}: non-triangle")

    if obj.get("pt_free_topology_operation") != "FULL_REINDEX_VISUAL_EQUIVALENCE":
        raise RuntimeError("blend was not prepared by free-topology reindex helper V1")
    if int(obj.get("pt_free_topology_version", -1)) != 1:
        raise RuntimeError("unexpected free-topology metadata version")
    if obj.get("pt_free_topology_source_smd_sha256") != SOURCE_SHA:
        raise RuntimeError("free-topology source SHA metadata mismatch")

    new_to_old_v = parse_csv_ints(
        obj.get("pt_free_topology_new_to_old_vertices"), SOURCE_V,
        "new_to_old_vertices"
    )
    old_to_new_v = parse_csv_ints(
        obj.get("pt_free_topology_old_to_new_vertices"), SOURCE_V,
        "old_to_new_vertices"
    )
    new_to_old_f = parse_csv_ints(
        obj.get("pt_free_topology_new_to_old_faces"), SOURCE_F,
        "new_to_old_faces"
    )
    old_to_new_f = parse_csv_ints(
        obj.get("pt_free_topology_old_to_new_faces"), SOURCE_F,
        "old_to_new_faces"
    )

    if sorted(new_to_old_v) != list(range(SOURCE_V)):
        raise RuntimeError("vertex permutation is not bijective")
    if sorted(new_to_old_f) != list(range(SOURCE_F)):
        raise RuntimeError("face permutation is not bijective")

    for new_i, old_i in enumerate(new_to_old_v):
        if old_to_new_v[old_i] != new_i:
            raise RuntimeError("vertex permutation inverse mismatch")
    for new_i, old_i in enumerate(new_to_old_f):
        if old_to_new_f[old_i] != new_i:
            raise RuntimeError("face permutation inverse mismatch")

    fixed_vertices = [i for i in range(SOURCE_V) if old_to_new_v[i] == i]
    fixed_faces = [i for i in range(SOURCE_F) if old_to_new_f[i] == i]
    if fixed_vertices:
        raise RuntimeError(f"fixed vertex indices remain: {fixed_vertices}")
    if fixed_faces:
        raise RuntimeError(f"fixed face-array indices remain: {fixed_faces}")

    expected_state_sha = obj.get("pt_free_topology_state_sha256")
    state_sha = canonical_mesh_digest(obj)
    if state_sha != expected_state_sha:
        raise RuntimeError(
            "Blender mesh changed after the controlled reindex helper; "
            "refusing this isolated gate"
        )

    rows = parse_object_table(data)
    target_row = next((r for r in rows if r["name"] == TARGET), None)
    if target_row is None:
        raise RuntimeError("MshC01 not found in SMD object table")

    lay = object_layout(data, target_row)
    if (lay["nv"], lay["nf"], lay["nt"]) != (SOURCE_V, SOURCE_F, SOURCE_T):
        raise RuntimeError(
            f"unexpected source counts {lay['nv']}/{lay['nf']}/{lay['nt']}"
        )
    if (lay["nr"], lay["np"], lay["ns"]) != (0, 0, 0):
        raise RuntimeError(
            f"MshC01 unexpectedly has animation arrays "
            f"{lay['nr']}/{lay['np']}/{lay['ns']}"
        )
    if i32(lay["head"], OFF_NV_MIRROR) != SOURCE_V:
        raise RuntimeError("raw nVertex mirror mismatch")
    if i32(lay["head"], OFF_NF_MIRROR) != SOURCE_F:
        raise RuntimeError("raw nFace mirror mismatch")

    src_vertices = [
        data[lay["vertex_off"] + i * SZ_VERTEX:
             lay["vertex_off"] + (i + 1) * SZ_VERTEX]
        for i in range(SOURCE_V)
    ]
    src_faces = [
        data[lay["face_off"] + i * SZ_FACE:
             lay["face_off"] + (i + 1) * SZ_FACE]
        for i in range(SOURCE_F)
    ]
    src_texlinks = data[
        lay["tex_off"]:lay["tex_off"] + SOURCE_T * SZ_TEXLINK
    ]
    src_physique_raw = [
        data[lay["physique_off"] + i * 32:
             lay["physique_off"] + (i + 1) * 32]
        for i in range(SOURCE_V)
    ]
    src_physique_names = source_physique_names(data, lay)

    # Validate Blender rigid groups and face semantics against the source mapping.
    uv_layer = mesh.uv_layers.active
    uv_max_error = 0.0
    material_mismatches = 0

    source_face_blob = data[
        lay["face_off"]:lay["face_off"] + SOURCE_F * SZ_FACE
    ]

    for new_vi, old_vi in enumerate(new_to_old_v):
        bone = rigid_bone(obj, new_vi)
        if bone != src_physique_names[old_vi]:
            raise RuntimeError(
                f"vertex {new_vi}: bone {bone!r} != source old vertex "
                f"{old_vi} bone {src_physique_names[old_vi]!r}"
            )

    for new_fi, old_fi in enumerate(new_to_old_f):
        src = src_faces[old_fi]
        old_a, old_b, old_c, src_mat = struct.unpack_from("<4H", src, 0)
        expected = (
            old_to_new_v[old_a],
            old_to_new_v[old_b],
            old_to_new_v[old_c],
        )
        got = tuple(int(x) for x in mesh.polygons[new_fi].vertices)
        if got != expected:
            raise RuntimeError(
                f"face {new_fi}: {got} != expected mapped source {expected}"
            )

        if int(mesh.polygons[new_fi].material_index) != int(src_mat):
            material_mismatches += 1

        tex_i, _ = infer_texlink_index(source_face_blob, lay["tex_off"], old_fi)
        su, sv = texlink_uv(data, lay["tex_off"], tex_i)
        buv = polygon_uvs(mesh, mesh.polygons[new_fi], uv_layer)

        for corner in range(3):
            expected_u = float(su[corner])
            expected_v = 1.0 - float(sv[corner])
            uv_max_error = max(
                uv_max_error,
                abs(buv[corner][0] - expected_u),
                abs(buv[corner][1] - expected_v),
            )

    if material_mismatches:
        raise RuntimeError(f"material mismatches={material_mismatches}")
    if uv_max_error > 2e-5:
        raise RuntimeError(f"UV mismatch too large: {uv_max_error}")

    # Rebuild arrays.
    out_vertices = b"".join(src_vertices[old_i] for old_i in new_to_old_v)

    out_faces_rows = []
    for new_fi, old_fi in enumerate(new_to_old_f):
        rec = bytearray(src_faces[old_fi])
        old_a, old_b, old_c = struct.unpack_from("<3H", rec, 0)
        new_ids = (
            old_to_new_v[old_a],
            old_to_new_v[old_b],
            old_to_new_v[old_c],
        )
        if max(new_ids) >= 65536:
            raise RuntimeError("WORD vertex-index overflow")
        struct.pack_into("<3H", rec, 0, *new_ids)
        out_faces_rows.append(bytes(rec))

    out_faces = b"".join(out_faces_rows)
    out_physique = b"".join(src_physique_raw[old_i] for old_i in new_to_old_v)

    # Keep target fixed smOBJ3D header, TexLinks and any between-array bytes exact.
    new_block = (
        lay["head"]
        + out_vertices
        + out_faces
        + src_texlinks
        + out_physique
    )

    if len(new_block) != target_row["length"]:
        raise RuntimeError(
            f"target block size changed: {len(new_block)} != {target_row['length']}"
        )

    out_data = bytearray(data)
    start = target_row["start"]
    out_data[start:start + target_row["length"]] = new_block
    out_data = bytes(out_data)

    # Strong byte-level invariants.
    if len(out_data) != len(data):
        raise RuntimeError("whole-file size changed")

    # Header + object table are byte-identical in this same-count gate.
    table_end = SZ_HEADER + len(rows) * SZ_OBJINFO
    if out_data[:table_end] != data[:table_end]:
        raise RuntimeError("header/object table changed unexpectedly")

    unchanged_objects = []
    for row in rows:
        if row["name"] == TARGET:
            continue
        s = row["start"]
        e = s + row["length"]
        if out_data[s:e] != data[s:e]:
            raise RuntimeError(f"unrelated object changed: {row['name']}")
        unchanged_objects.append(row["name"])

    # Output semantic parse.
    out_rows = parse_object_table(out_data)
    out_target = next(r for r in out_rows if r["name"] == TARGET)
    out_lay = object_layout(out_data, out_target)

    if (out_lay["nv"], out_lay["nf"], out_lay["nt"]) != (
        SOURCE_V, SOURCE_F, SOURCE_T
    ):
        raise RuntimeError("output semantic counts mismatch")

    # Verify vertex and Physique reorder exactly.
    for new_i, old_i in enumerate(new_to_old_v):
        ov = out_data[
            out_lay["vertex_off"] + new_i * SZ_VERTEX:
            out_lay["vertex_off"] + (new_i + 1) * SZ_VERTEX
        ]
        if ov != src_vertices[old_i]:
            raise RuntimeError(f"output vertex {new_i}: reorder verification failed")

        op = out_data[
            out_lay["physique_off"] + new_i * 32:
            out_lay["physique_off"] + (new_i + 1) * 32
        ]
        if op != src_physique_raw[old_i]:
            raise RuntimeError(f"output Physique {new_i}: reorder verification failed")

    # Verify each output face is the chosen old face with only v[0..2] remapped.
    for new_fi, old_fi in enumerate(new_to_old_f):
        got = out_data[
            out_lay["face_off"] + new_fi * SZ_FACE:
            out_lay["face_off"] + (new_fi + 1) * SZ_FACE
        ]
        expected = bytearray(src_faces[old_fi])
        a, b, c = struct.unpack_from("<3H", expected, 0)
        struct.pack_into(
            "<3H", expected, 0,
            old_to_new_v[a], old_to_new_v[b], old_to_new_v[c]
        )
        if got != bytes(expected):
            raise RuntimeError(f"output face {new_fi}: remap verification failed")

    # TexLinks must be byte-identical in this gate.
    got_tex = out_data[
        out_lay["tex_off"]:out_lay["tex_off"] + SOURCE_T * SZ_TEXLINK
    ]
    if got_tex != src_texlinks:
        raise RuntimeError("TexLink array changed unexpectedly")

    output_sha = hashlib.sha256(out_data).hexdigest()
    if output_sha == input_sha:
        raise RuntimeError("reindexed output unexpectedly byte-identical")

    out = Path(args.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(out_data)

    report = {
        "tool": "PT_SMD_FREE_TOPOLOGY_REINDEX_WRITER_V1",
        "object": TARGET,
        "operation": "FULL_REINDEX_VISUAL_EQUIVALENCE",
        "input": str(source),
        "output": str(out),
        "input_size": len(data),
        "output_size": len(out_data),
        "input_sha256": input_sha,
        "output_sha256": output_sha,
        "source_counts": {
            "vertices": SOURCE_V,
            "faces": SOURCE_F,
            "texlinks": SOURCE_T,
        },
        "output_counts": {
            "vertices": out_lay["nv"],
            "faces": out_lay["nf"],
            "texlinks": out_lay["nt"],
        },
        "reindexed_vertices": SOURCE_V,
        "reordered_faces": SOURCE_F,
        "fixed_vertex_indices": fixed_vertices,
        "fixed_face_indices": fixed_faces,
        "uv_max_error": uv_max_error,
        "material_mismatches": material_mismatches,
        "mesh_state_sha256": state_sha,
        "unrelated_object_blocks_byte_identical": len(unchanged_objects),
        "texlinks_byte_identical": True,
        "semantic_check": "PASS",
        "free_topology_reindex_export_check": "PASS",
    }

    report_path = Path(args.report).expanduser().resolve()
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print("PT_SMD_FREE_TOPOLOGY_REINDEX_WRITER_V1")
    print("object=MshC01")
    print("operation=FULL_REINDEX_VISUAL_EQUIVALENCE")
    print(f"source_counts={SOURCE_V}/{SOURCE_F}/{SOURCE_T}")
    print(f"output_counts={out_lay['nv']}/{out_lay['nf']}/{out_lay['nt']}")
    print(f"reindexed_vertices={SOURCE_V}")
    print(f"reordered_faces={SOURCE_F}")
    print(f"fixed_vertex_indices={fixed_vertices}")
    print(f"fixed_face_indices={fixed_faces}")
    print(f"uv_max_error={uv_max_error:.12g}")
    print(f"material_mismatches={material_mismatches}")
    print(f"unrelated_object_blocks_byte_identical={len(unchanged_objects)}")
    print("texlinks_byte_identical=True")
    print(f"input_size={len(data)}")
    print(f"output_size={len(out_data)}")
    print(f"input_sha256={input_sha}")
    print(f"output_sha256={output_sha}")
    print(f"report={report_path}")
    print("semantic_check=PASS")
    print("free_topology_reindex_export_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(
            f"free_topology_reindex_export_check=FAIL: "
            f"{type(exc).__name__}: {exc}"
        )
        raise

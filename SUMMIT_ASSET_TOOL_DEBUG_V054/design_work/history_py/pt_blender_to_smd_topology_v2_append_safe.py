#!/usr/bin/env python3
"""
Summit Tale / Priestess C01 - append-safe topology exporter V2

Scope:
  object: MsmC01 only
  source: pristine MfbC01.smd (known SHA256)

Supports:
  - original 125 vertices MUST remain present, same indices, same positions,
    same rigid Physique assignments
  - new vertices may be appended (index >= 125)
  - faces may be rebuilt freely from those vertices
  - all faces must remain triangles
  - material must remain SMD material 1
  - one UV layer
  - every new vertex must have exactly one rigid vertex group at weight 1.0
  - new vertex bone must exist in m5.smb

This is intended for controlled Subdivide / Extrude-style experiments where
the original source vertices are not moved or deleted.

Normals:
  - original 125 smVERTEX normals are preserved byte-for-byte
  - new vertex normals use the proven BONE_ROW + equal-weight geometric
    direction
  - magnitude is the median original normal magnitude for the assigned bone
    (global median fallback)

Not supported yet:
  - deleting/reindexing original vertices
  - moving original vertices
  - changing old Physique assignments
  - non-triangle polygons
  - multiple UV layers / multiple TexLink layers
  - changing MsmC01 material away from material id 1
  - other C01 objects / LODs
  - animation tracks in the target object

This tool writes a NEW SMD. It never modifies the client.
"""

import argparse
import hashlib
import json
import math
import statistics
import struct
import sys
from collections import defaultdict
from pathlib import Path

import bpy
from mathutils import Vector


HEADER = b"SMD Model data Ver 0.62"
SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"

SZ_HDR = 556
SZ_INFO = 40
SZ_OBJ = 2236
SZ_VERT = 24
SZ_FACE = 36
SZ_TEX = 32
SZ_ROT = 20
SZ_POS = 16
SZ_SCALE = 16
SZ_MAT = 64

O_VERTEX_PTR = 4
O_FACE_PTR = 8
O_TEX_PTR = 12
O_PHYS_PTR = 16
O_VERTEX_CAP = 76
O_FACE_CAP = 80
O_NV = 84
O_NF = 88
O_NT = 92
O_NAME = 172
O_TM = 240
O_NR = 684
O_NP = 688
O_NS = 692

F_TEX_PTR = 32
T_U = 0
T_V = 12
T_HTEX = 24
T_NEXT = 28

SOURCE_NV = 125
SOURCE_NF = 238
SOURCE_NT = 238
TARGET_OBJECT = "MsmC01"
TARGET_MATERIAL = 1


def i32(buf, off):
    return struct.unpack_from("<i", buf, off)[0]


def u32(buf, off):
    return struct.unpack_from("<I", buf, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def sha(data):
    return hashlib.sha256(data).hexdigest()


def parse_file(data):
    if len(data) < SZ_HDR or not data.startswith(HEADER):
        raise ValueError("unexpected SMD/SMB header")

    nobj = i32(data, 24)
    infos = []
    for idx in range(nobj):
        off = SZ_HDR + idx * SZ_INFO
        raw = data[off:off+SZ_INFO]
        infos.append({
            "index": idx,
            "table_off": off,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })

    for idx, inf in enumerate(infos):
        if inf["length"] <= 0:
            raise ValueError(f"{inf['name']}: invalid object length")
        end = inf["start"] + inf["length"]
        if idx + 1 < len(infos):
            if end != infos[idx+1]["start"]:
                raise ValueError(f"{inf['name']}: non-contiguous object table")
        elif end != len(data):
            raise ValueError("last object does not close file")

    return infos


def layout(start, nv, nf, nt, nr, np, ns, phys):
    v = start + SZ_OBJ
    f = v + nv * SZ_VERT
    t = f + nf * SZ_FACE
    r = t + nt * SZ_TEX
    p = r + nr * SZ_ROT
    s = p + np * SZ_POS
    prev = s + ns * SZ_SCALE
    phy = prev + nr * SZ_MAT if phys else None
    end = phy + nv * 32 if phys else prev + nr * SZ_MAT
    return dict(vertices=v, faces=f, tex=t, rot=r, pos=p, scale=s,
                prev=prev, physique=phy, end=end)


def parse_object(data, infos, name):
    inf = next((x for x in infos if x["name"] == name), None)
    if not inf:
        raise ValueError(f"object {name!r} not found")

    raw = data[inf["start"]:inf["start"]+SZ_OBJ]
    if len(raw) != SZ_OBJ:
        raise ValueError(f"{name}: truncated object header")
    if cstr(raw[O_NAME:O_NAME+32]) != name:
        raise ValueError(f"{name}: NodeName mismatch")

    nv = i32(raw, O_NV)
    nf = i32(raw, O_NF)
    nt = i32(raw, O_NT)
    nr = i32(raw, O_NR)
    np = i32(raw, O_NP)
    ns = i32(raw, O_NS)
    phys = u32(raw, O_PHYS_PTR) != 0

    lay = layout(inf["start"], nv, nf, nt, nr, np, ns, phys)
    if lay["end"] != inf["start"] + inf["length"]:
        raise ValueError(f"{name}: serialized length mismatch")

    verts = [
        struct.unpack_from("<6i", data, lay["vertices"] + i*SZ_VERT)
        for i in range(nv)
    ]

    faces = []
    for i in range(nf):
        off = lay["faces"] + i*SZ_FACE
        r = data[off:off+SZ_FACE]
        a,b,c,mat = struct.unpack_from("<4H", r, 0)
        faces.append(dict(raw=r, verts=(a,b,c), material=mat,
                          lp_tex=u32(r, F_TEX_PTR)))

    tex = []
    for i in range(nt):
        off = lay["tex"] + i*SZ_TEX
        r = data[off:off+SZ_TEX]
        tex.append(dict(
            raw=r,
            u=struct.unpack_from("<3f", r, T_U),
            v=struct.unpack_from("<3f", r, T_V),
            hTexture=u32(r, T_HTEX),
            NextTex=u32(r, T_NEXT),
        ))

    bones = []
    if phys:
        for i in range(nv):
            off = lay["physique"] + i*32
            bones.append(cstr(data[off:off+32]))

    return dict(info=inf, raw=raw, nv=nv, nf=nf, nt=nt, nr=nr, np=np, ns=ns,
                phys=phys, layout=lay, vertices=verts, faces=faces, texlinks=tex,
                bones=bones, texbase=u32(raw, O_TEX_PTR))


def parse_skeleton(data):
    infos = parse_file(data)
    out = {}
    for inf in infos:
        raw = data[inf["start"]:inf["start"]+SZ_OBJ]
        name = cstr(raw[O_NAME:O_NAME+32])
        if name in out:
            raise ValueError(f"duplicate skeleton node {name!r}")
        out[name] = struct.unpack_from("<16i", raw, O_TM)
    return out


def m(tm, r, c):
    return tm[(r-1)*4 + (c-1)]


def world_fixed(local, tm):
    x,y,z = map(int, local)
    rx = x*m(tm,1,1) + y*m(tm,2,1) + z*m(tm,3,1)
    ry = x*m(tm,1,2) + y*m(tm,2,2) + z*m(tm,3,2)
    rz = x*m(tm,1,3) + y*m(tm,2,3) + z*m(tm,3,3)
    return (
        (rx >> 8) + m(tm,4,1),
        (ry >> 8) + m(tm,4,2),
        (rz >> 8) + m(tm,4,3),
    )


def inv_row3(vec, tm):
    a,b,c = m(tm,1,1)/256.0, m(tm,1,2)/256.0, m(tm,1,3)/256.0
    d,e,f = m(tm,2,1)/256.0, m(tm,2,2)/256.0, m(tm,2,3)/256.0
    g,h,i = m(tm,3,1)/256.0, m(tm,3,2)/256.0, m(tm,3,3)/256.0

    det = a*(e*i-f*h) - b*(d*i-f*g) + c*(d*h-e*g)
    if abs(det) < 1e-12:
        raise ValueError("singular bone matrix")

    inv = (
        ((e*i-f*h)/det, (c*h-b*i)/det, (b*f-c*e)/det),
        ((f*g-d*i)/det, (a*i-c*g)/det, (c*d-a*f)/det),
        ((d*h-e*g)/det, (b*g-a*h)/det, (a*e-b*d)/det),
    )
    x,y,z = vec
    return (
        x*inv[0][0] + y*inv[1][0] + z*inv[2][0],
        x*inv[0][1] + y*inv[1][1] + z*inv[2][1],
        x*inv[0][2] + y*inv[1][2] + z*inv[2][2],
    )


def inverse_point(world, tm):
    v = (
        world[0] - m(tm,4,1)/256.0,
        world[1] - m(tm,4,2)/256.0,
        world[2] - m(tm,4,3)/256.0,
    )
    return inv_row3(v, tm)


def fit_local(world, tm, radius=4):
    q = inverse_point(world, tm)
    center = tuple(int(round(x*256.0)) for x in q)

    best = None
    for dx in range(-radius, radius+1):
        for dy in range(-radius, radius+1):
            for dz in range(-radius, radius+1):
                cand = (center[0]+dx, center[1]+dy, center[2]+dz)
                wf = world_fixed(cand, tm)
                represented = tuple(x/256.0 for x in wf)
                err2 = sum((represented[k]-world[k])**2 for k in range(3))
                score = (err2, cand)
                if best is None or score < best[0]:
                    best = (score, cand, represented)

    return best[1], best[2], math.sqrt(best[0][0])


def normalize(v):
    mag = math.sqrt(sum(float(x)*float(x) for x in v))
    if mag <= 1e-20:
        raise ValueError("zero-length normal")
    return tuple(float(x)/mag for x in v)


def tri_normal(coords):
    a,b,c = map(Vector, coords)
    n = (b-a).cross(c-a)
    if n.length <= 1e-12:
        raise ValueError("degenerate triangle")
    n.normalize()
    return (float(n.x), float(n.y), float(n.z))


def rigid_bone(obj, vidx):
    rows = []
    for g in obj.data.vertices[vidx].groups:
        if g.weight > 1e-8:
            if g.group >= len(obj.vertex_groups):
                raise ValueError(f"vertex {vidx}: invalid group index")
            rows.append((obj.vertex_groups[g.group].name, float(g.weight)))

    if len(rows) != 1 or abs(rows[0][1] - 1.0) > 1e-5:
        raise ValueError(
            f"vertex {vidx}: requires exactly one rigid group with weight 1.0"
        )
    return rows[0][0]


def bone_slot(name):
    raw = name.encode("latin-1")
    if len(raw) >= 32:
        raise ValueError(f"bone name too long: {name!r}")
    return raw + b"\0"*(32-len(raw))


def face_template(template, verts, texptr):
    r = bytearray(template)
    struct.pack_into("<3H", r, 0, *verts)
    struct.pack_into("<H", r, 6, TARGET_MATERIAL)
    struct.pack_into("<I", r, F_TEX_PTR, texptr)
    return bytes(r)


def tex_template(template, uvs):
    r = bytearray(template)
    struct.pack_into("<3f", r, T_U, *(x[0] for x in uvs))
    struct.pack_into("<3f", r, T_V, *(x[1] for x in uvs))
    struct.pack_into("<I", r, T_NEXT, 0)
    return bytes(r)


def blender_uvs_smd(mesh, poly, uv_layer):
    return [
        (float(uv_layer.data[li].uv.x), 1.0-float(uv_layer.data[li].uv.y))
        for li in poly.loop_indices
    ]


def geometric_equal_normals(mesh):
    accum = [Vector((0.0,0.0,0.0)) for _ in mesh.vertices]
    incident = [0]*len(mesh.vertices)

    for p in mesh.polygons:
        ids = tuple(int(v) for v in p.vertices)
        coords = [tuple(float(x) for x in mesh.vertices[v].co) for v in ids]
        n = Vector(tri_normal(coords))
        for v in ids:
            accum[v] += n
            incident[v] += 1

    out = []
    for i, (v,n) in enumerate(zip(accum, incident)):
        if n == 0 or v.length <= 1e-12:
            raise ValueError(f"vertex {i}: cannot derive equal-weight normal")
        v.normalize()
        out.append((float(v.x), float(v.y), float(v.z)))
    return out


def verify_output(data, target_name, expect_v, expect_f):
    infos = parse_file(data)
    obj = parse_object(data, infos, target_name)
    if (obj["nv"],obj["nf"],obj["nt"]) != (expect_v,expect_f,expect_f):
        raise ValueError("output count reparse mismatch")

    base = obj["texbase"]
    for fi, face in enumerate(obj["faces"]):
        if any(v >= obj["nv"] for v in face["verts"]):
            raise ValueError(f"output face {fi}: vertex out of range")
        if face["material"] != TARGET_MATERIAL:
            raise ValueError(f"output face {fi}: material changed")
        d = face["lp_tex"] - base
        if d < 0 or d % SZ_TEX:
            raise ValueError(f"output face {fi}: invalid TexLink pointer")
        ti = d // SZ_TEX
        if not 0 <= ti < obj["nt"]:
            raise ValueError(f"output face {fi}: TexLink out of range")
        if obj["texlinks"][ti]["NextTex"] != 0:
            raise ValueError(f"output TexLink {ti}: unexpected layer chain")
    if len(obj["bones"]) != expect_v or any(not b for b in obj["bones"]):
        raise ValueError("output Physique invalid")
    return infos, obj


def args_parse(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--skeleton", required=True)
    p.add_argument("--object", default=TARGET_OBJECT)
    p.add_argument("--out", required=True)
    p.add_argument("--report")
    p.add_argument("--max-fit-error", type=float, default=0.02)
    return p.parse_args(argv)


def main(argv):
    args = args_parse(argv)
    if args.object != TARGET_OBJECT:
        raise ValueError("V2 currently supports only MsmC01")

    smd_path = Path(args.smd).expanduser().resolve()
    skel_path = Path(args.skeleton).expanduser().resolve()
    out_path = Path(args.out).expanduser().resolve()

    src = smd_path.read_bytes()
    if sha(src) != SOURCE_SHA:
        raise ValueError("source SMD is not the approved pristine MfbC01.smd")

    infos = parse_file(src)
    src_obj = parse_object(src, infos, TARGET_OBJECT)
    if (src_obj["nv"],src_obj["nf"],src_obj["nt"]) != (SOURCE_NV,SOURCE_NF,SOURCE_NT):
        raise ValueError("unexpected pristine MsmC01 counts")
    if not src_obj["phys"] or src_obj["nr"] or src_obj["np"] or src_obj["ns"]:
        raise ValueError("unexpected MsmC01 Physique/track state")

    if i32(src_obj["raw"], O_VERTEX_CAP) != SOURCE_NV or \
       i32(src_obj["raw"], O_FACE_CAP) != SOURCE_NF:
        raise ValueError("unexpected source capacity mirrors")

    # Specific C01/MsmC01 material/TexLink gate.
    mats = {f["material"] for f in src_obj["faces"]}
    htex = {t["hTexture"] for t in src_obj["texlinks"]}
    if mats != {TARGET_MATERIAL}:
        raise ValueError(f"expected MsmC01 source material {{1}}, got {mats}")
    if len(htex) != 1:
        raise ValueError("MsmC01 source hTexture is not constant")
    if any(t["NextTex"] != 0 for t in src_obj["texlinks"]):
        raise ValueError("MsmC01 source unexpectedly has multi-layer TexLinks")

    skeleton = parse_skeleton(skel_path.read_bytes())

    obj = bpy.data.objects.get(TARGET_OBJECT)
    if obj is None or obj.type != "MESH":
        raise ValueError("MsmC01 Blender mesh not found")
    mesh = obj.data
    mesh.update()

    if len(mesh.vertices) < SOURCE_NV:
        raise ValueError("original vertices were deleted")
    if len(mesh.vertices) > 65535:
        raise ValueError("smFACE vertex indices are 16-bit; vertex count exceeds 65535")
    if len(mesh.polygons) <= 0 or len(mesh.polygons) > 2_000_000_000:
        raise ValueError("invalid face count")
    if len(mesh.uv_layers) != 1 or mesh.uv_layers.active is None:
        raise ValueError("V2 requires exactly one active UV layer")

    # Triangles + material gate.
    for p in mesh.polygons:
        if len(p.vertices) != 3:
            raise ValueError(
                f"polygon {p.index}: non-triangle ({len(p.vertices)} corners); triangulate first"
            )
        if int(p.material_index) != TARGET_MATERIAL:
            raise ValueError(
                f"polygon {p.index}: material_index={p.material_index}; "
                f"V2 requires material {TARGET_MATERIAL}"
            )

    # Verify original 125 vertex identity by both bind position and Physique.
    max_old_error = 0.0
    for vi in range(SOURCE_NV):
        source_bone = src_obj["bones"][vi]
        actual_bone = rigid_bone(obj, vi)
        if actual_bone != source_bone:
            raise ValueError(
                f"old vertex {vi}: Physique changed {source_bone!r}->{actual_bone!r}"
            )
        tm = skeleton.get(source_bone)
        if tm is None:
            raise ValueError(f"old vertex {vi}: missing skeleton bone {source_bone!r}")
        wf = world_fixed(src_obj["vertices"][vi][:3], tm)
        expected = tuple(x/256.0 for x in wf)
        actual = tuple(float(x) for x in mesh.vertices[vi].co)
        err = math.sqrt(sum((actual[k]-expected[k])**2 for k in range(3)))
        max_old_error = max(max_old_error, err)
        if err > 1e-6:
            raise ValueError(
                f"old vertex {vi} moved by {err:.9g}; "
                "V2 append-safe mode forbids moving original vertices"
            )

    # All new vertices must be valid rigid Physique bones.
    new_bones = []
    for vi in range(SOURCE_NV, len(mesh.vertices)):
        bone = rigid_bone(obj, vi)
        if bone not in skeleton:
            raise ValueError(f"new vertex {vi}: bone {bone!r} missing from skeleton")
        new_bones.append(bone)

    equal_world_normals = geometric_equal_normals(mesh)

    # Median source normal magnitude per bone, global fallback.
    mags_by_bone = defaultdict(list)
    global_mags = []
    for vi, sv in enumerate(src_obj["vertices"]):
        nx,ny,nz = sv[3:6]
        mag = math.sqrt(nx*nx+ny*ny+nz*nz)
        mags_by_bone[src_obj["bones"][vi]].append(mag)
        global_mags.append(mag)
    global_median = statistics.median(global_mags)

    # Old vertices are byte-exact; append only new ones.
    vertices_blob = bytearray(
        src[src_obj["layout"]["vertices"]:src_obj["layout"]["faces"]]
    )
    fit_errors = []
    normal_scales = []
    new_vertex_rows = []

    for vi in range(SOURCE_NV, len(mesh.vertices)):
        bone = new_bones[vi-SOURCE_NV]
        tm = skeleton[bone]
        world = tuple(float(x) for x in mesh.vertices[vi].co)
        local, represented, fit = fit_local(world, tm)
        if fit > args.max_fit_error:
            raise ValueError(
                f"new vertex {vi}: fit_error {fit:.9g} > {args.max_fit_error}"
            )

        local_dir = normalize(inv_row3(equal_world_normals[vi], tm))
        scale = statistics.median(mags_by_bone.get(bone, [])) \
            if mags_by_bone.get(bone) else global_median
        normal = tuple(int(round(x*scale)) for x in local_dir)

        vertices_blob.extend(struct.pack(
            "<6i", local[0],local[1],local[2],normal[0],normal[1],normal[2]
        ))
        fit_errors.append(fit)
        normal_scales.append(scale)
        new_vertex_rows.append({
            "index": vi,
            "bone": bone,
            "world": list(world),
            "local_fixed": list(local),
            "represented_world": list(represented),
            "fit_error": fit,
            "normal_local": list(normal),
            "normal_scale": scale,
        })

    # Rebuild every face and TexLink from Blender.
    texbase = src_obj["texbase"]
    face_raw_template = src_obj["faces"][0]["raw"]
    tex_raw_template = src_obj["texlinks"][0]["raw"]
    uv_layer = mesh.uv_layers.active

    faces_blob = bytearray()
    tex_blob = bytearray()

    for fi, poly in enumerate(mesh.polygons):
        verts = tuple(int(v) for v in poly.vertices)
        if any(v > 65535 for v in verts):
            raise ValueError(f"face {fi}: vertex index exceeds uint16")
        ptr = texbase + fi*SZ_TEX
        if ptr > 0xFFFFFFFF:
            raise ValueError("historical TexLink pointer overflow")

        faces_blob.extend(face_template(face_raw_template, verts, ptr))
        tex_blob.extend(tex_template(
            tex_raw_template,
            blender_uvs_smd(mesh, poly, uv_layer),
        ))

    # Physique = pristine names + new Blender assignments.
    phys_blob = bytearray(
        src[src_obj["layout"]["physique"]:src_obj["layout"]["end"]]
    )
    for bone in new_bones:
        phys_blob.extend(bone_slot(bone))

    nv = len(mesh.vertices)
    nf = len(mesh.polygons)
    nt = nf

    raw_obj = bytearray(src_obj["raw"])
    struct.pack_into("<i", raw_obj, O_VERTEX_CAP, nv)
    struct.pack_into("<i", raw_obj, O_FACE_CAP, nf)
    struct.pack_into("<i", raw_obj, O_NV, nv)
    struct.pack_into("<i", raw_obj, O_NF, nf)
    struct.pack_into("<i", raw_obj, O_NT, nt)

    new_block = (
        bytes(raw_obj)
        + bytes(vertices_blob)
        + bytes(faces_blob)
        + bytes(tex_blob)
        + bytes(phys_blob)
    )

    expected_block = SZ_OBJ + nv*SZ_VERT + nf*SZ_FACE + nt*SZ_TEX + nv*32
    if len(new_block) != expected_block:
        raise ValueError("new MsmC01 block length mismatch")

    # Rebuild object table, preserve all unrelated object blocks literally.
    prefix = bytearray(src[:infos[0]["start"]])
    blocks = []
    cursor = infos[0]["start"]
    table_rows = []

    for inf in infos:
        block = new_block if inf["name"] == TARGET_OBJECT else \
            src[inf["start"]:inf["start"]+inf["length"]]
        blocks.append(block)
        struct.pack_into("<i", prefix, inf["table_off"]+32, len(block))
        struct.pack_into("<i", prefix, inf["table_off"]+36, cursor)
        table_rows.append({
            "name": inf["name"],
            "old_start": inf["start"],
            "new_start": cursor,
            "old_length": inf["length"],
            "new_length": len(block),
        })
        cursor += len(block)

    out = bytes(prefix) + b"".join(blocks)

    # Independent reparse.
    out_infos, out_obj = verify_output(out, TARGET_OBJECT, nv, nf)

    # Unrelated blocks byte-identical.
    for old_inf, new_inf in zip(infos, out_infos):
        if old_inf["name"] == TARGET_OBJECT:
            continue
        old_b = src[old_inf["start"]:old_inf["start"]+old_inf["length"]]
        new_b = out[new_inf["start"]:new_inf["start"]+new_inf["length"]]
        if old_b != new_b:
            raise ValueError(f"unrelated object changed: {old_inf['name']}")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(out)

    report = {
        "tool": "PT_BLENDER_TO_SMD_TOPOLOGY_V2_APPEND_SAFE",
        "scope": "MsmC01 append-safe topology",
        "blend": bpy.data.filepath,
        "source_smd": str(smd_path),
        "source_sha256": sha(src),
        "output": str(out_path),
        "output_sha256": sha(out),
        "source_counts": {"vertices": SOURCE_NV, "faces": SOURCE_NF, "texlinks": SOURCE_NT},
        "output_counts": {"vertices": nv, "faces": nf, "texlinks": nt},
        "new_vertices": nv-SOURCE_NV,
        "face_delta": nf-SOURCE_NF,
        "input_size": len(src),
        "output_size": len(out),
        "growth_bytes": len(out)-len(src),
        "max_old_vertex_error": max_old_error,
        "max_new_vertex_fit_error": max(fit_errors) if fit_errors else 0.0,
        "new_vertex_rows": new_vertex_rows,
        "normal_mode": "preserve original normals; new=BONE_ROW+EQUAL",
        "normal_scale_mode": "median source normal magnitude for assigned bone",
        "material_id": TARGET_MATERIAL,
        "hTexture": next(iter(htex)),
        "object_table": table_rows,
        "semantic_check": "PASS",
        "append_safe_topology_export_check": "PASS",
    }

    report_path = Path(args.report).expanduser().resolve() if args.report \
        else out_path.with_suffix(".json")
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print("PT_BLENDER_TO_SMD_TOPOLOGY_V2_APPEND_SAFE")
    print("object=MsmC01")
    print(f"source_counts={SOURCE_NV}/{SOURCE_NF}/{SOURCE_NT}")
    print(f"output_counts={nv}/{nf}/{nt}")
    print(f"new_vertices={nv-SOURCE_NV}")
    print(f"face_delta={nf-SOURCE_NF:+d}")
    print(f"max_old_vertex_error={max_old_error:.12g}")
    print(f"max_new_vertex_fit_error={(max(fit_errors) if fit_errors else 0.0):.12g}")
    print(f"input_size={len(src)}")
    print(f"output_size={len(out)}")
    print(f"growth_bytes={len(out)-len(src):+d}")
    print(f"input_sha256={sha(src)}")
    print(f"output_sha256={sha(out)}")
    print(f"report={report_path}")
    print("semantic_check=PASS")
    print("append_safe_topology_export_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"append_safe_topology_export_check=FAIL: {type(exc).__name__}: {exc}")
        raise

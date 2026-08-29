#!/usr/bin/env python3
"""
Summit Tale / Priestess C01 - controlled Blender -> SMD topology serializer V1

SUPPORTED OPERATION ONLY:
    MsmC01
    SPLIT_ONE_FACE_1_TO_3
    pristine 125 vertices / 238 faces / 238 texlinks
        ->
    126 vertices / 240 faces / 240 texlinks

Expected Blender input is produced by:
    pt_c01_topology_make_split_v1.py

Design:
  - original MfbC01.smd is the binary template
  - all unrelated object blocks remain byte-identical
  - target smOBJ3D raw struct is preserved except required count/capacity fields
  - old vertices remain byte-identical
  - exactly one new smVERTEX is appended
  - source face is replaced in-place, two child faces appended
  - source TexLink is replaced in-place, two child TexLinks appended
  - new Physique bone name is appended
  - smDFILE_OBJINFO Length / ObjFilePoint are rebuilt
  - material/header region is otherwise preserved

This is NOT a generic topology exporter.
It does NOT install anything into the game client.
"""

import argparse
import hashlib
import json
import math
import struct
import sys
from pathlib import Path

import bpy
from mathutils import Vector


SMD_HEADER_TEXT = b"SMD Model data Ver 0.62"
EXPECTED_SOURCE_SHA256 = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"

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

# smOBJ3D i686 offsets
O_OFF_VERTEX_PTR = 4
O_OFF_FACE_PTR = 8
O_OFF_TEXLINK_PTR = 12
O_OFF_PHYSIQUE_PTR = 16

# These two serialized fields mirror nVertex/nFace on all 15 objects in
# the pristine C01 asset and act as allocation/capacity values.
O_OFF_VERTEX_CAPACITY = 76
O_OFF_FACE_CAPACITY = 80

O_OFF_NVERTEX = 84
O_OFF_NFACE = 88
O_OFF_NTEXLINK = 92
O_OFF_NODENAME = 172
O_OFF_TM = 240
O_OFF_TMROTCNT = 684
O_OFF_TMPOSCNT = 688
O_OFF_TMSCALECNT = 692

F_OFF_VERTS = 0
F_OFF_T = 8
F_OFF_TEXLINK_PTR = 32

T_OFF_U = 0
T_OFF_V = 12
T_OFF_HTEXTURE = 24
T_OFF_NEXT = 28


def i32(buf, off):
    return struct.unpack_from("<i", buf, off)[0]


def u32(buf, off):
    return struct.unpack_from("<I", buf, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while True:
            b = f.read(1024 * 1024)
            if not b:
                break
            h.update(b)
    return h.hexdigest()


def parse_header_and_infos(data):
    if len(data) < SIZE_DFILE_HEADER:
        raise ValueError("file smaller than smDFILE_HEADER")
    if not data.startswith(SMD_HEADER_TEXT):
        raise ValueError("unexpected SMD header")

    obj_counter = i32(data, 24)
    mat_counter = i32(data, 28)
    mat_file_point = i32(data, 32)
    first_obj_point = i32(data, 36)

    if obj_counter != 15:
        raise ValueError(f"expected C01 ObjCounter=15, got {obj_counter}")
    if mat_counter != 6:
        raise ValueError(f"expected C01 MatCounter=6, got {mat_counter}")

    table_end = SIZE_DFILE_HEADER + obj_counter * SIZE_DFILE_OBJINFO
    if table_end != mat_file_point:
        raise ValueError("object table does not end at MatFilePoint")
    if not (mat_file_point <= first_obj_point <= len(data)):
        raise ValueError("invalid First_ObjInfoPoint")

    infos = []
    for index in range(obj_counter):
        off = SIZE_DFILE_HEADER + index * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        info = {
            "index": index,
            "table_off": off,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        }
        infos.append(info)

    if infos[0]["start"] != first_obj_point:
        raise ValueError("first object start != First_ObjInfoPoint")

    for i, info in enumerate(infos):
        if info["length"] <= 0:
            raise ValueError(f"{info['name']}: invalid object length")
        end = info["start"] + info["length"]
        if i + 1 < len(infos):
            if end != infos[i + 1]["start"]:
                raise ValueError(f"{info['name']}: object blocks are not contiguous")
        elif end != len(data):
            raise ValueError("last object does not end at file size")

    return {
        "obj_counter": obj_counter,
        "mat_counter": mat_counter,
        "mat_file_point": mat_file_point,
        "first_obj_point": first_obj_point,
        "infos": infos,
    }


def object_layout(start, nv, nf, nt, nr, np, ns, has_physique):
    vertices = start + SIZE_OBJ3D
    faces = vertices + nv * SIZE_VERTEX
    texlinks = faces + nf * SIZE_FACE
    tmrot = texlinks + nt * SIZE_TEXLINK
    tmpos = tmrot + nr * SIZE_TM_ROT
    tmscale = tmpos + np * SIZE_TM_POS
    prevrot = tmscale + ns * SIZE_TM_SCALE
    physique = prevrot + nr * SIZE_MATRIX if has_physique else None
    end = physique + nv * 32 if has_physique else prevrot + nr * SIZE_MATRIX
    return {
        "vertices": vertices,
        "faces": faces,
        "texlinks": texlinks,
        "tmrot": tmrot,
        "tmpos": tmpos,
        "tmscale": tmscale,
        "prevrot": prevrot,
        "physique": physique,
        "end": end,
    }


def parse_object(data, meta, object_name):
    info = next((x for x in meta["infos"] if x["name"] == object_name), None)
    if info is None:
        raise ValueError(f"object {object_name!r} not found")

    start = info["start"]
    raw = data[start:start + SIZE_OBJ3D]
    if len(raw) != SIZE_OBJ3D:
        raise ValueError(f"{object_name}: truncated smOBJ3D")

    node_name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
    if node_name != object_name:
        raise ValueError("object table / NodeName mismatch")

    nv = i32(raw, O_OFF_NVERTEX)
    nf = i32(raw, O_OFF_NFACE)
    nt = i32(raw, O_OFF_NTEXLINK)
    nr = i32(raw, O_OFF_TMROTCNT)
    np = i32(raw, O_OFF_TMPOSCNT)
    ns = i32(raw, O_OFF_TMSCALECNT)
    has_physique = u32(raw, O_OFF_PHYSIQUE_PTR) != 0

    if min(nv, nf, nt, nr, np, ns) < 0:
        raise ValueError(f"{object_name}: negative count")
    if not has_physique:
        raise ValueError(f"{object_name}: Physique is required")

    layout = object_layout(start, nv, nf, nt, nr, np, ns, has_physique)
    if layout["end"] != start + info["length"]:
        raise ValueError(
            f"{object_name}: length mismatch computed={layout['end']-start} "
            f"table={info['length']}"
        )

    vertices = [
        struct.unpack_from("<6i", data, layout["vertices"] + i * SIZE_VERTEX)
        for i in range(nv)
    ]

    faces = []
    for i in range(nf):
        off = layout["faces"] + i * SIZE_FACE
        raw_face = data[off:off + SIZE_FACE]
        a, b, c, mat = struct.unpack_from("<4H", raw_face, 0)
        lp_tex = u32(raw_face, F_OFF_TEXLINK_PTR)
        faces.append({
            "raw": raw_face,
            "verts": (a, b, c),
            "material": mat,
            "lp_tex": lp_tex,
        })

    texlinks = []
    for i in range(nt):
        off = layout["texlinks"] + i * SIZE_TEXLINK
        raw_tex = data[off:off + SIZE_TEXLINK]
        u = struct.unpack_from("<3f", raw_tex, T_OFF_U)
        v = struct.unpack_from("<3f", raw_tex, T_OFF_V)
        htex = u32(raw_tex, T_OFF_HTEXTURE)
        next_tex = u32(raw_tex, T_OFF_NEXT)
        texlinks.append({
            "raw": raw_tex,
            "u": u,
            "v": v,
            "hTexture": htex,
            "NextTex": next_tex,
        })

    bones = []
    p = layout["physique"]
    for i in range(nv):
        bone_raw = data[p + i * 32:p + (i + 1) * 32]
        bones.append(cstr(bone_raw))

    tex_base = u32(raw, O_OFF_TEXLINK_PTR)

    return {
        "info": info,
        "raw": raw,
        "layout": layout,
        "nv": nv,
        "nf": nf,
        "nt": nt,
        "nr": nr,
        "np": np,
        "ns": ns,
        "has_physique": has_physique,
        "vertices": vertices,
        "faces": faces,
        "texlinks": texlinks,
        "bones": bones,
        "tex_base": tex_base,
    }


def parse_skeleton_tms(data):
    meta = parse_header_and_infos_skeleton(data)
    out = {}
    for info in meta["infos"]:
        raw = data[info["start"]:info["start"] + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise ValueError(f"skeleton object {info['name']!r}: truncated")
        name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        if name != info["name"]:
            raise ValueError("skeleton table / NodeName mismatch")
        if name in out:
            raise ValueError(f"duplicate skeleton bone {name!r}")
        out[name] = struct.unpack_from("<16i", raw, O_OFF_TM)
    return out


def parse_header_and_infos_skeleton(data):
    if len(data) < SIZE_DFILE_HEADER or not data.startswith(SMD_HEADER_TEXT):
        raise ValueError("unexpected SMB/SMD skeleton header")
    n = i32(data, 24)
    infos = []
    for i in range(n):
        off = SIZE_DFILE_HEADER + i * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        infos.append({
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })
    return {"infos": infos}


def mval(tm, row, col):
    return tm[(row - 1) * 4 + (col - 1)]


def transform_point_fixed(local_xyz, tm):
    x, y, z = (int(local_xyz[0]), int(local_xyz[1]), int(local_xyz[2]))
    rx = x*mval(tm,1,1) + y*mval(tm,2,1) + z*mval(tm,3,1)
    ry = x*mval(tm,1,2) + y*mval(tm,2,2) + z*mval(tm,3,2)
    rz = x*mval(tm,1,3) + y*mval(tm,2,3) + z*mval(tm,3,3)
    return (
        (rx >> 8) + mval(tm,4,1),
        (ry >> 8) + mval(tm,4,2),
        (rz >> 8) + mval(tm,4,3),
    )


def inverse_3x3_row_vector(vec, tm):
    a,b,c = mval(tm,1,1)/256.0, mval(tm,1,2)/256.0, mval(tm,1,3)/256.0
    d,e,f = mval(tm,2,1)/256.0, mval(tm,2,2)/256.0, mval(tm,2,3)/256.0
    g,h,i = mval(tm,3,1)/256.0, mval(tm,3,2)/256.0, mval(tm,3,3)/256.0

    det = a*(e*i-f*h) - b*(d*i-f*g) + c*(d*h-e*g)
    if abs(det) < 1e-12:
        raise ValueError("singular bone 3x3 matrix")

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


def inverse_point_float(world_xyz, tm):
    translated = (
        world_xyz[0] - mval(tm,4,1)/256.0,
        world_xyz[1] - mval(tm,4,2)/256.0,
        world_xyz[2] - mval(tm,4,3)/256.0,
    )
    return inverse_3x3_row_vector(translated, tm)


def choose_local_fixed(target_world, tm, reference_local, radius=4):
    lf = inverse_point_float(target_world, tm)
    center = tuple(int(round(v * 256.0)) for v in lf)

    best = None
    for dx in range(-radius, radius + 1):
        for dy in range(-radius, radius + 1):
            for dz in range(-radius, radius + 1):
                cand = (center[0]+dx, center[1]+dy, center[2]+dz)
                wf = transform_point_fixed(cand, tm)
                represented = tuple(x/256.0 for x in wf)
                err2 = sum((represented[k]-target_world[k])**2 for k in range(3))
                ref2 = sum((cand[k]-reference_local[k])**2 for k in range(3))
                score = (err2, ref2, cand)
                if best is None or score < best[0]:
                    best = (score, cand, represented)

    return best[1], best[2], math.sqrt(best[0][0])


def normalize(v):
    x,y,z = (float(v[0]), float(v[1]), float(v[2]))
    mag = math.sqrt(x*x+y*y+z*z)
    if mag <= 1e-20:
        raise ValueError("cannot normalize zero vector")
    return (x/mag, y/mag, z/mag)


def face_unit_normal(a, b, c):
    va, vb, vc = Vector(a), Vector(b), Vector(c)
    n = (vb-va).cross(vc-va)
    if n.length <= 1e-12:
        raise ValueError("degenerate split child face")
    n.normalize()
    return (float(n.x), float(n.y), float(n.z))


def make_new_local_normal(mesh, child_faces, tm, source_normals):
    # Proven probe result: serialized normal transformed with BONE_ROW most closely
    # matches equal-weight geometric vertex normals. Reconstruct that convention
    # for the new vertex.
    world_face_normals = []
    for tri in child_faces:
        coords = [tuple(float(x) for x in mesh.vertices[i].co) for i in tri]
        world_face_normals.append(face_unit_normal(*coords))

    sx = sum(n[0] for n in world_face_normals)
    sy = sum(n[1] for n in world_face_normals)
    sz = sum(n[2] for n in world_face_normals)
    world_equal = normalize((sx,sy,sz))

    local_dir = normalize(inverse_3x3_row_vector(world_equal, tm))

    source_mags = [
        math.sqrt(float(nx*nx + ny*ny + nz*nz))
        for nx,ny,nz in source_normals
    ]
    scale = sum(source_mags) / len(source_mags)

    return tuple(int(round(c * scale)) for c in local_dir), world_equal, scale


def blender_face_uvs_smd(mesh, poly, uv_layer):
    out = []
    for li in poly.loop_indices:
        uv = uv_layer.data[li].uv
        out.append((float(uv.x), 1.0 - float(uv.y)))
    return out


def rigid_bone(obj, vidx):
    v = obj.data.vertices[vidx]
    rows = [(obj.vertex_groups[g.group].name, float(g.weight))
            for g in v.groups if g.weight > 1e-8]
    if len(rows) != 1 or abs(rows[0][1] - 1.0) > 1e-5:
        raise ValueError(f"vertex {vidx}: expected exactly one rigid group weight=1")
    return rows[0][0]


def encode_bone_name(name):
    raw = name.encode("latin-1")
    if len(raw) >= 32:
        raise ValueError(f"bone name too long for 32-byte Physique slot: {name!r}")
    return raw + b"\0" * (32 - len(raw))


def patch_face(template, verts, tex_ptr):
    raw = bytearray(template)
    material = struct.unpack_from("<H", raw, 6)[0]
    struct.pack_into("<3H", raw, F_OFF_VERTS, *verts)
    struct.pack_into("<H", raw, 6, material)
    struct.pack_into("<I", raw, F_OFF_TEXLINK_PTR, tex_ptr)
    return bytes(raw)


def patch_texlink(template, smd_uvs):
    if len(smd_uvs) != 3:
        raise ValueError("TexLink requires 3 UV corners")
    raw = bytearray(template)
    struct.pack_into("<3f", raw, T_OFF_U, *(uv[0] for uv in smd_uvs))
    struct.pack_into("<3f", raw, T_OFF_V, *(uv[1] for uv in smd_uvs))
    # hTexture is preserved from source template.
    struct.pack_into("<I", raw, T_OFF_NEXT, 0)
    return bytes(raw)


def validate_semantics(data):
    meta = parse_header_and_infos(data)
    total_v = total_f = total_t = 0

    for info in meta["infos"]:
        obj = parse_object(data, meta, info["name"])
        total_v += obj["nv"]
        total_f += obj["nf"]
        total_t += obj["nt"]

        if obj["nt"] < obj["nf"]:
            raise ValueError(f"{info['name']}: fewer TexLinks than faces")

        base = obj["tex_base"]
        tex_refs = []
        for fi, face in enumerate(obj["faces"]):
            if any(v >= obj["nv"] for v in face["verts"]):
                raise ValueError(f"{info['name']} face {fi}: invalid vertex index")
            ptr = face["lp_tex"]
            if ptr == 0:
                raise ValueError(f"{info['name']} face {fi}: null TexLink")
            delta = ptr - base
            if delta < 0 or delta % SIZE_TEXLINK:
                raise ValueError(f"{info['name']} face {fi}: invalid TexLink pointer")
            ti = delta // SIZE_TEXLINK
            if not (0 <= ti < obj["nt"]):
                raise ValueError(f"{info['name']} face {fi}: TexLink index out of range")
            tex_refs.append(ti)

        for ti, tex in enumerate(obj["texlinks"]):
            nxt = tex["NextTex"]
            if nxt:
                delta = nxt - base
                if delta < 0 or delta % SIZE_TEXLINK:
                    raise ValueError(f"{info['name']} TexLink {ti}: invalid NextTex")
                ni = delta // SIZE_TEXLINK
                if not (0 <= ni < obj["nt"]):
                    raise ValueError(f"{info['name']} TexLink {ti}: NextTex out of range")

        if len(obj["bones"]) != obj["nv"] or any(not x for x in obj["bones"]):
            raise ValueError(f"{info['name']}: invalid Physique table")

    return {
        "objects": meta["obj_counter"],
        "vertices": total_v,
        "faces": total_f,
        "texlinks": total_t,
    }


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--skeleton", required=True)
    p.add_argument("--object", default="MsmC01")
    p.add_argument("--out", required=True)
    p.add_argument("--report")
    p.add_argument("--max-fit-error", type=float, default=0.02)
    return p.parse_args(argv)


def main(argv):
    args = parse_args(argv)

    if args.object != "MsmC01":
        raise ValueError("topology V1 supports only MsmC01")

    smd_path = Path(args.smd).expanduser().resolve()
    smb_path = Path(args.skeleton).expanduser().resolve()
    out_path = Path(args.out).expanduser().resolve()

    if out_path == smd_path:
        raise ValueError("refusing in-place overwrite")

    src = smd_path.read_bytes()
    src_sha = sha256_bytes(src)
    if src_sha != EXPECTED_SOURCE_SHA256:
        raise ValueError(f"unexpected source SMD SHA256: {src_sha}")

    meta = parse_header_and_infos(src)
    target = parse_object(src, meta, args.object)

    if (target["nv"], target["nf"], target["nt"]) != (125,238,238):
        raise ValueError(
            f"expected pristine MsmC01 125/238/238, got "
            f"{target['nv']}/{target['nf']}/{target['nt']}"
        )

    # Capacity mirrors observed in every pristine C01 object.
    cap_v = i32(target["raw"], O_OFF_VERTEX_CAPACITY)
    cap_f = i32(target["raw"], O_OFF_FACE_CAPACITY)
    if (cap_v, cap_f) != (target["nv"], target["nf"]):
        raise ValueError(
            f"unexpected capacity mirror fields {cap_v}/{cap_f}; "
            "V1 refuses to infer a different layout"
        )

    if target["nr"] or target["np"] or target["ns"]:
        raise ValueError("MsmC01 unexpectedly contains animation tracks")

    skeleton = parse_skeleton_tms(smb_path.read_bytes())

    obj = bpy.data.objects.get(args.object)
    if obj is None or obj.type != "MESH":
        raise ValueError(f"Blender mesh {args.object!r} not found")
    mesh = obj.data
    mesh.update()

    # Helper metadata gate.
    if obj.get("pt_topology_operation") != "SPLIT_ONE_FACE_1_TO_3":
        raise ValueError("missing/invalid pt_topology_operation metadata")
    if int(obj.get("pt_topology_version", -1)) != 1:
        raise ValueError("unexpected topology helper version")
    if int(obj.get("pt_topology_source_vertex_count", -1)) != 125:
        raise ValueError("source vertex metadata mismatch")
    if int(obj.get("pt_topology_source_face_count", -1)) != 238:
        raise ValueError("source face metadata mismatch")
    if str(obj.get("pt_topology_source_smd_sha256", "")) != src_sha:
        raise ValueError("blend topology metadata does not match source SMD SHA256")

    source_face_index = int(obj["pt_topology_source_face_index"])
    source_tri = (
        int(obj["pt_topology_source_face_a"]),
        int(obj["pt_topology_source_face_b"]),
        int(obj["pt_topology_source_face_c"]),
    )
    new_idx = int(obj["pt_topology_new_vertex_index"])
    common_bone = str(obj["pt_topology_common_bone"])

    if len(mesh.vertices) != 126 or len(mesh.polygons) != 240:
        raise ValueError(
            f"expected helper topology 126/240, got "
            f"{len(mesh.vertices)}/{len(mesh.polygons)}"
        )
    if new_idx != 125:
        raise ValueError(f"expected appended new vertex index 125, got {new_idx}")
    if not (0 <= source_face_index < 238):
        raise ValueError("invalid source face index metadata")

    source_face = target["faces"][source_face_index]
    if tuple(source_face["verts"]) != source_tri:
        raise ValueError(
            f"source SMD face mismatch: {source_face['verts']} != {source_tri}"
        )

    a,b,c = source_tri
    expected_children = [
        (a,b,new_idx),
        (b,c,new_idx),
        (c,a,new_idx),
    ]
    actual_children = [
        tuple(int(x) for x in mesh.polygons[source_face_index].vertices),
        tuple(int(x) for x in mesh.polygons[-2].vertices),
        tuple(int(x) for x in mesh.polygons[-1].vertices),
    ]
    if actual_children != expected_children:
        raise ValueError(
            f"controlled split topology changed: "
            f"{actual_children!r} != {expected_children!r}"
        )

    # Every old non-source face must remain exactly identical.
    for fi in range(238):
        if fi == source_face_index:
            continue
        blender_tri = tuple(int(x) for x in mesh.polygons[fi].vertices)
        smd_tri = tuple(target["faces"][fi]["verts"])
        if blender_tri != smd_tri:
            raise ValueError(
                f"old face {fi} changed: Blender={blender_tri} SMD={smd_tri}"
            )

    # All 125 old vertices must still be the pristine bind-pose points.
    max_old_vertex_error = 0.0
    for vi in range(125):
        bone = target["bones"][vi]
        tm = skeleton.get(bone)
        if tm is None:
            raise ValueError(f"old vertex {vi}: missing bone {bone!r}")
        wf = transform_point_fixed(target["vertices"][vi][:3], tm)
        expected = tuple(x/256.0 for x in wf)
        actual = tuple(float(x) for x in mesh.vertices[vi].co)
        err = math.sqrt(sum((actual[k]-expected[k])**2 for k in range(3)))
        max_old_vertex_error = max(max_old_vertex_error, err)
        if err > 1e-6:
            raise ValueError(
                f"old vertex {vi} moved by {err:.9g}; "
                "topology V1 accepts the helper split only"
            )

    if [target["bones"][x] for x in source_tri] != [common_bone]*3:
        raise ValueError("source face SMD Physique is not the helper common bone")
    if rigid_bone(obj, new_idx) != common_bone:
        raise ValueError("new Blender vertex rigid bone mismatch")

    tm = skeleton.get(common_bone)
    if tm is None:
        raise ValueError(f"common bone {common_bone!r} missing from SMB")

    new_world = tuple(float(x) for x in mesh.vertices[new_idx].co)
    reference_local = tuple(
        int(round(sum(target["vertices"][v][axis] for v in source_tri) / 3.0))
        for axis in range(3)
    )
    new_local, represented_world, fit_error = choose_local_fixed(
        new_world, tm, reference_local, radius=4
    )
    if fit_error > args.max_fit_error:
        raise ValueError(
            f"new vertex fit_error {fit_error:.9g} > {args.max_fit_error}"
        )

    source_normals = [target["vertices"][v][3:6] for v in source_tri]
    new_normal, world_equal_normal, normal_scale = make_new_local_normal(
        mesh, expected_children, tm, source_normals
    )

    new_vertex_raw = struct.pack(
        "<6i",
        new_local[0], new_local[1], new_local[2],
        new_normal[0], new_normal[1], new_normal[2],
    )

    # Source face must have one direct TexLink.
    tex_base = target["tex_base"]
    if tex_base == 0:
        raise ValueError("target TexLink base pointer is null")
    src_lp = source_face["lp_tex"]
    delta = src_lp - tex_base
    if delta < 0 or delta % SIZE_TEXLINK:
        raise ValueError("source face TexLink pointer is invalid")
    source_tex_index = delta // SIZE_TEXLINK
    if not (0 <= source_tex_index < 238):
        raise ValueError("source TexLink index out of range")

    source_tex = target["texlinks"][source_tex_index]
    if source_tex["NextTex"] != 0:
        raise ValueError("source face unexpectedly has multi-layer TexLink")

    uv_layer = mesh.uv_layers.active
    if uv_layer is None:
        raise ValueError("active Blender UV layer missing")

    child_polys = [
        mesh.polygons[source_face_index],
        mesh.polygons[-2],
        mesh.polygons[-1],
    ]
    child_uvs = [blender_face_uvs_smd(mesh, p, uv_layer) for p in child_polys]

    new_tex_index_1 = 238
    new_tex_index_2 = 239
    ptr_child_0 = tex_base + source_tex_index * SIZE_TEXLINK
    ptr_child_1 = tex_base + new_tex_index_1 * SIZE_TEXLINK
    ptr_child_2 = tex_base + new_tex_index_2 * SIZE_TEXLINK
    for ptr in (ptr_child_0, ptr_child_1, ptr_child_2):
        if not (0 <= ptr <= 0xFFFFFFFF):
            raise ValueError("historical TexLink pointer overflow")

    child_face_raw = [
        patch_face(source_face["raw"], expected_children[0], ptr_child_0),
        patch_face(source_face["raw"], expected_children[1], ptr_child_1),
        patch_face(source_face["raw"], expected_children[2], ptr_child_2),
    ]
    child_tex_raw = [
        patch_texlink(source_tex["raw"], child_uvs[0]),
        patch_texlink(source_tex["raw"], child_uvs[1]),
        patch_texlink(source_tex["raw"], child_uvs[2]),
    ]

    # Build target block.
    raw_obj = bytearray(target["raw"])
    struct.pack_into("<i", raw_obj, O_OFF_VERTEX_CAPACITY, 126)
    struct.pack_into("<i", raw_obj, O_OFF_FACE_CAPACITY, 240)
    struct.pack_into("<i", raw_obj, O_OFF_NVERTEX, 126)
    struct.pack_into("<i", raw_obj, O_OFF_NFACE, 240)
    struct.pack_into("<i", raw_obj, O_OFF_NTEXLINK, 240)

    vertices_blob = bytearray(
        src[target["layout"]["vertices"]:target["layout"]["faces"]]
    )
    if len(vertices_blob) != 125 * SIZE_VERTEX:
        raise ValueError("source vertex blob size mismatch")
    vertices_blob.extend(new_vertex_raw)

    faces_blob = bytearray(
        src[target["layout"]["faces"]:target["layout"]["texlinks"]]
    )
    if len(faces_blob) != 238 * SIZE_FACE:
        raise ValueError("source face blob size mismatch")
    foff = source_face_index * SIZE_FACE
    faces_blob[foff:foff+SIZE_FACE] = child_face_raw[0]
    faces_blob.extend(child_face_raw[1])
    faces_blob.extend(child_face_raw[2])

    tex_end = target["layout"]["tmrot"]
    tex_blob = bytearray(
        src[target["layout"]["texlinks"]:tex_end]
    )
    if len(tex_blob) != 238 * SIZE_TEXLINK:
        raise ValueError("source TexLink blob size mismatch")
    toff = source_tex_index * SIZE_TEXLINK
    tex_blob[toff:toff+SIZE_TEXLINK] = child_tex_raw[0]
    tex_blob.extend(child_tex_raw[1])
    tex_blob.extend(child_tex_raw[2])

    # MsmC01 has no animation arrays, so tail between texlinks and Physique is empty.
    if target["layout"]["tmrot"] != target["layout"]["physique"]:
        raise ValueError("unexpected target track/tail layout")

    physique_blob = bytearray(
        src[target["layout"]["physique"]:target["layout"]["end"]]
    )
    if len(physique_blob) != 125 * 32:
        raise ValueError("source Physique blob size mismatch")
    physique_blob.extend(encode_bone_name(common_bone))

    new_target_block = (
        bytes(raw_obj)
        + bytes(vertices_blob)
        + bytes(faces_blob)
        + bytes(tex_blob)
        + bytes(physique_blob)
    )

    expected_target_length = (
        SIZE_OBJ3D
        + 126*SIZE_VERTEX
        + 240*SIZE_FACE
        + 240*SIZE_TEXLINK
        + 126*32
    )
    if len(new_target_block) != expected_target_length:
        raise ValueError(
            f"new target length {len(new_target_block)} != {expected_target_length}"
        )

    growth = len(new_target_block) - target["info"]["length"]
    if growth != 192:
        raise ValueError(f"expected target growth +192, got {growth:+d}")

    # Rebuild all object blocks and object table. Non-target blocks are literal copies.
    blocks = []
    for info in meta["infos"]:
        if info["name"] == args.object:
            blocks.append(new_target_block)
        else:
            blocks.append(src[info["start"]:info["start"] + info["length"]])

    prefix = bytearray(src[:meta["first_obj_point"]])
    cursor = meta["first_obj_point"]
    new_infos = []

    for old_info, block in zip(meta["infos"], blocks):
        table_off = old_info["table_off"]
        struct.pack_into("<i", prefix, table_off + 32, len(block))
        struct.pack_into("<i", prefix, table_off + 36, cursor)
        new_infos.append({
            "name": old_info["name"],
            "old_start": old_info["start"],
            "new_start": cursor,
            "old_length": old_info["length"],
            "new_length": len(block),
        })
        cursor += len(block)

    out_data = bytes(prefix) + b"".join(blocks)

    expected_file_size = len(src) + 192
    if len(out_data) != expected_file_size:
        raise ValueError(
            f"output size {len(out_data)} != expected {expected_file_size}"
        )

    # Structural + semantic independent reparse of the output bytes.
    semantic = validate_semantics(out_data)
    out_meta = parse_header_and_infos(out_data)
    out_target = parse_object(out_data, out_meta, args.object)

    if (out_target["nv"], out_target["nf"], out_target["nt"]) != (126,240,240):
        raise ValueError("output target counts mismatch")

    if semantic != {
        "objects": 15,
        "vertices": 1445,
        "faces": 2539,
        "texlinks": 2539,
    }:
        raise ValueError(f"unexpected global semantic totals: {semantic!r}")

    # All blocks after MsmC01 must remain byte-identical after accounting for shift.
    for old_info, new_info in zip(meta["infos"], out_meta["infos"]):
        if old_info["name"] == args.object:
            continue
        old_block = src[old_info["start"]:old_info["start"]+old_info["length"]]
        new_block = out_data[new_info["start"]:new_info["start"]+new_info["length"]]
        if old_block != new_block:
            raise ValueError(f"unrelated object block changed: {old_info['name']}")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(out_data)

    report = {
        "tool": "PT_BLENDER_TO_SMD_TOPOLOGY_V1",
        "operation": "SPLIT_ONE_FACE_1_TO_3",
        "blend": bpy.data.filepath,
        "source_smd": str(smd_path),
        "source_sha256": src_sha,
        "output": str(out_path),
        "output_sha256": sha256_bytes(out_data),
        "input_size": len(src),
        "output_size": len(out_data),
        "growth_bytes": growth,
        "object": args.object,
        "source_counts": {"vertices":125,"faces":238,"texlinks":238},
        "output_counts": {"vertices":126,"faces":240,"texlinks":240},
        "global_output_counts": semantic,
        "source_face_index": source_face_index,
        "source_face": list(source_tri),
        "child_faces": [list(x) for x in expected_children],
        "source_texlink_index": source_tex_index,
        "child_texlink_indices": [source_tex_index,238,239],
        "common_bone": common_bone,
        "new_vertex_index": new_idx,
        "new_vertex_world": list(new_world),
        "new_vertex_local_fixed": list(new_local),
        "represented_world": list(represented_world),
        "fit_error": fit_error,
        "new_normal_local": list(new_normal),
        "new_normal_world_equal": list(world_equal_normal),
        "normal_scale": normal_scale,
        "max_old_vertex_error": max_old_vertex_error,
        "object_table": new_infos,
        "semantic_check": "PASS",
        "topology_export_check": "PASS",
    }

    report_path = Path(args.report).expanduser().resolve() if args.report else \
        out_path.with_suffix(".json")
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print("PT_BLENDER_TO_SMD_TOPOLOGY_V1")
    print("operation=SPLIT_ONE_FACE_1_TO_3")
    print("object=", args.object)
    print("source_counts=125/238/238")
    print("output_counts=126/240/240")
    print("global_counts=1445/2539/2539")
    print("source_face_index=", source_face_index)
    print("source_face=", source_tri)
    print("child_faces=", expected_children)
    print("source_texlink_index=", source_tex_index)
    print("child_texlink_indices=", (source_tex_index,238,239))
    print("common_bone=", repr(common_bone))
    print("new_vertex_index=", new_idx)
    print("new_vertex_local_fixed=", new_local)
    print("new_vertex_fit_error=", f"{fit_error:.12g}")
    print("new_normal_local=", new_normal)
    print("new_normal_world_equal=", tuple(round(x,9) for x in world_equal_normal))
    print("normal_scale=", f"{normal_scale:.9g}")
    print("input_size=", len(src))
    print("output_size=", len(out_data))
    print("growth_bytes=", growth)
    print("input_sha256=", src_sha)
    print("output_sha256=", sha256_bytes(out_data))
    print("report=", report_path)
    print("semantic_check=PASS")
    print("topology_export_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"topology_export_check=FAIL: {type(exc).__name__}: {exc}")
        raise

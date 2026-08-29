#!/usr/bin/env python3
"""
PT SMD MODEL 0.62 — FREE MESH WRITER V2.2

First generic artist-oriented MshC01 writer.

Input requirements:
  - arbitrary triangular MshC01 Blender mesh
  - 3..30000 vertices
  - 1..30000 faces
  - exactly one active UV layer
  - exactly one rigid bone group at weight 1.0 per vertex
  - bone names must exist in the supplied SMB skeleton
  - face material IDs must already exist in pristine MshC01 for V2

Crucially, this writer has NO old_to_new mapping and NO append-only rule.
Every smVERTEX record is rebuilt from Blender.
Every smFACE is rebuilt from Blender.
Every smTEXLINK is rebuilt from Blender.
Every Physique assignment is rebuilt from Blender vertex groups.

The pristine SMD is used only as a container/template for:
  - fixed smOBJ3D header fields not yet fully regenerated
  - material/face/TexLink record templates
  - material -> hTexture mapping
  - normal magnitude calibration
  - the other 14 object blocks, kept byte-identical

It writes a NEW SMD only. It never writes the game client.
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
from mathutils import Matrix, Vector

HEADER = b"SMD Model data Ver 0.62"
SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
TARGET = "MshC01"

SZ_HDR = 556
SZ_INFO = 40
SZ_OBJ = 2236
SZ_VERT = 24
SZ_FACE = 36
SZ_TEX = 32
SZ_ROT = 20
SZ_POS = 16
SZ_SCALE = 16
SZ_MATRIX = 64

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

MAX_VERTICES = 30000
MAX_FACES = 30000


def i32(b, o):
    return struct.unpack_from("<i", b, o)[0]


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def cstr(b):
    return b.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def sha(b):
    return hashlib.sha256(b).hexdigest()


def parse_file(data):
    if len(data) < SZ_HDR or not data.startswith(HEADER):
        raise ValueError("unexpected SMD/SMB header")

    n = i32(data, 24)
    if not 1 <= n <= 128:
        raise ValueError(f"implausible object count {n}")

    infos = []
    for idx in range(n):
        off = SZ_HDR + idx * SZ_INFO
        r = data[off:off + SZ_INFO]
        infos.append(dict(
            index=idx,
            table_off=off,
            name=cstr(r[:32]),
            length=i32(r, 32),
            start=i32(r, 36),
        ))

    for idx, inf in enumerate(infos):
        if inf["length"] <= 0:
            raise ValueError(f"{inf['name']}: bad object length")
        end = inf["start"] + inf["length"]
        if idx + 1 < len(infos):
            if end != infos[idx + 1]["start"]:
                raise ValueError(f"{inf['name']}: non-contiguous objects")
        elif end != len(data):
            raise ValueError("last object does not close file")
    return infos


def obj_layout(start, nv, nf, nt, nr, np, ns, phys):
    v = start + SZ_OBJ
    f = v + nv * SZ_VERT
    t = f + nf * SZ_FACE
    r = t + nt * SZ_TEX
    p = r + nr * SZ_ROT
    s = p + np * SZ_POS
    prev = s + ns * SZ_SCALE
    phy = prev + nr * SZ_MATRIX if phys else None
    end = phy + nv * 32 if phys else prev + nr * SZ_MATRIX
    return dict(
        vertices=v, faces=f, tex=t, rot=r, pos=p, scale=s,
        prev=prev, physique=phy, end=end
    )


def parse_object(data, infos, name):
    inf = next((x for x in infos if x["name"] == name), None)
    if not inf:
        raise ValueError(f"object {name!r} missing")

    raw = data[inf["start"]:inf["start"] + SZ_OBJ]
    if cstr(raw[O_NAME:O_NAME + 32]) != name:
        raise ValueError(f"{name}: NodeName mismatch")

    nv = i32(raw, O_NV)
    nf = i32(raw, O_NF)
    nt = i32(raw, O_NT)
    nr = i32(raw, O_NR)
    np = i32(raw, O_NP)
    ns = i32(raw, O_NS)
    phys = u32(raw, O_PHYS_PTR) != 0

    lay = obj_layout(inf["start"], nv, nf, nt, nr, np, ns, phys)
    if lay["end"] != inf["start"] + inf["length"]:
        raise ValueError(f"{name}: serialized length mismatch")

    verts = [
        struct.unpack_from("<6i", data, lay["vertices"] + i * SZ_VERT)
        for i in range(nv)
    ]

    faces = []
    for i in range(nf):
        off = lay["faces"] + i * SZ_FACE
        r = data[off:off + SZ_FACE]
        a, b, c, mat = struct.unpack_from("<4H", r, 0)
        faces.append(dict(
            raw=r, verts=(a, b, c), material=mat,
            lp_tex=u32(r, F_TEX_PTR)
        ))

    texlinks = []
    for i in range(nt):
        off = lay["tex"] + i * SZ_TEX
        r = data[off:off + SZ_TEX]
        texlinks.append(dict(
            raw=r,
            u=struct.unpack_from("<3f", r, T_U),
            v=struct.unpack_from("<3f", r, T_V),
            hTexture=u32(r, T_HTEX),
            NextTex=u32(r, T_NEXT),
        ))

    bones = []
    if phys:
        for i in range(nv):
            off = lay["physique"] + i * 32
            bones.append(cstr(data[off:off + 32]))

    return dict(
        info=inf, raw=raw, nv=nv, nf=nf, nt=nt,
        nr=nr, np=np, ns=ns, phys=phys, layout=lay,
        vertices=verts, faces=faces, texlinks=texlinks,
        bones=bones, texbase=u32(raw, O_TEX_PTR)
    )


def parse_skeleton(data):
    infos = parse_file(data)
    out = {}
    for inf in infos:
        raw = data[inf["start"]:inf["start"] + SZ_OBJ]
        name = cstr(raw[O_NAME:O_NAME + 32])
        if name in out:
            raise ValueError(f"duplicate skeleton node {name!r}")
        out[name] = struct.unpack_from("<16i", raw, O_TM)
    return out


def m(tm, r, c):
    return tm[(r - 1) * 4 + (c - 1)]


def world_fixed(local, tm):
    x, y, z = map(int, local)
    rx = x * m(tm, 1, 1) + y * m(tm, 2, 1) + z * m(tm, 3, 1)
    ry = x * m(tm, 1, 2) + y * m(tm, 2, 2) + z * m(tm, 3, 2)
    rz = x * m(tm, 1, 3) + y * m(tm, 2, 3) + z * m(tm, 3, 3)
    return (
        (rx >> 8) + m(tm, 4, 1),
        (ry >> 8) + m(tm, 4, 2),
        (rz >> 8) + m(tm, 4, 3),
    )


def inv_row3(vec, tm):
    a, b, c = m(tm, 1, 1) / 256.0, m(tm, 1, 2) / 256.0, m(tm, 1, 3) / 256.0
    d, e, f = m(tm, 2, 1) / 256.0, m(tm, 2, 2) / 256.0, m(tm, 2, 3) / 256.0
    g, h, i = m(tm, 3, 1) / 256.0, m(tm, 3, 2) / 256.0, m(tm, 3, 3) / 256.0

    det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g)
    if abs(det) < 1e-12:
        raise ValueError("singular bone matrix")

    inv = (
        ((e * i - f * h) / det, (c * h - b * i) / det, (b * f - c * e) / det),
        ((f * g - d * i) / det, (a * i - c * g) / det, (c * d - a * f) / det),
        ((d * h - e * g) / det, (b * g - a * h) / det, (a * e - b * d) / det),
    )
    x, y, z = vec
    return (
        x * inv[0][0] + y * inv[1][0] + z * inv[2][0],
        x * inv[0][1] + y * inv[1][1] + z * inv[2][1],
        x * inv[0][2] + y * inv[1][2] + z * inv[2][2],
    )


def inverse_point(world, tm):
    return inv_row3(
        (
            world[0] - m(tm, 4, 1) / 256.0,
            world[1] - m(tm, 4, 2) / 256.0,
            world[2] - m(tm, 4, 3) / 256.0,
        ),
        tm,
    )


def fit_local(world, tm, radius=4):
    q = inverse_point(world, tm)
    center = tuple(int(round(x * 256.0)) for x in q)
    best = None

    for dx in range(-radius, radius + 1):
        for dy in range(-radius, radius + 1):
            for dz in range(-radius, radius + 1):
                cand = (center[0] + dx, center[1] + dy, center[2] + dz)
                wf = world_fixed(cand, tm)
                represented = tuple(x / 256.0 for x in wf)
                err2 = sum((represented[k] - world[k]) ** 2 for k in range(3))
                score = (err2, cand)
                if best is None or score < best[0]:
                    best = (score, cand, represented)

    return best[1], best[2], math.sqrt(best[0][0])


def normalize(v):
    mag = math.sqrt(sum(float(x) * float(x) for x in v))
    if mag <= 1e-20:
        raise ValueError("zero-length normal")
    return tuple(float(x) / mag for x in v)


def build_export_triangles(mesh):
    """
    Build a deterministic Blender LoopTriangle view for SMD serialization.
    Artist quads/ngons remain untouched in the .blend.
    """
    mesh.calc_loop_triangles()
    out = []

    for tri in mesh.loop_triangles:
        verts = tuple(int(v) for v in tri.vertices)
        loops = tuple(int(li) for li in tri.loops)
        if len(verts) != 3 or len(loops) != 3:
            raise ValueError("Blender produced an invalid LoopTriangle")

        pi = int(tri.polygon_index)
        if not 0 <= pi < len(mesh.polygons):
            raise ValueError(f"LoopTriangle polygon index out of range: {pi}")

        out.append({
            "vertices": verts,
            "loops": loops,
            "polygon_index": pi,
            "material_index": int(mesh.polygons[pi].material_index),
        })

    if not out:
        raise ValueError("triangulated export view is empty")

    return out


def equal_normals(mesh, export_triangles):
    acc = [Vector((0.0, 0.0, 0.0)) for _ in mesh.vertices]
    cnt = [0] * len(mesh.vertices)

    for fi, tri in enumerate(export_triangles):
        ids = tri["vertices"]
        a, b, c = [Vector(mesh.vertices[v].co) for v in ids]
        n = (b - a).cross(c - a)
        if n.length <= 1e-12:
            raise ValueError(
                f"export triangle {fi} / polygon {tri['polygon_index']}: "
                "degenerate triangle"
            )
        n.normalize()
        for vi in ids:
            acc[vi] += n
            cnt[vi] += 1

    out = []
    for vi, n in enumerate(acc):
        if cnt[vi] == 0 or n.length <= 1e-12:
            raise ValueError(f"vertex {vi}: no geometric normal")
        n.normalize()
        out.append((float(n.x), float(n.y), float(n.z)))
    return out


def rigid_bone(obj, vi):
    rows = []
    for e in obj.data.vertices[vi].groups:
        if e.weight <= 1e-8:
            continue
        if not (0 <= e.group < len(obj.vertex_groups)):
            raise ValueError(f"vertex {vi}: invalid vertex group index")
        rows.append((obj.vertex_groups[e.group].name, float(e.weight)))

    if len(rows) != 1 or abs(rows[0][1] - 1.0) > 1e-5:
        raise ValueError(
            f"vertex {vi}: expected exactly one rigid group at weight 1.0; got {rows}"
        )
    return rows[0][0]


def bone_slot(name):
    raw = name.encode("latin-1")
    if len(raw) >= 32:
        raise ValueError(f"bone name too long: {name!r}")
    return raw + b"\0" * (32 - len(raw))


def face_blob(template, verts, material, texptr):
    r = bytearray(template)
    struct.pack_into("<3H", r, 0, *verts)
    struct.pack_into("<H", r, 6, material)
    struct.pack_into("<I", r, F_TEX_PTR, texptr)
    return bytes(r)


def tex_blob(template, uvs, htex):
    r = bytearray(template)
    struct.pack_into("<3f", r, T_U, *(x[0] for x in uvs))
    struct.pack_into("<3f", r, T_V, *(x[1] for x in uvs))
    struct.pack_into("<I", r, T_HTEX, htex)
    struct.pack_into("<I", r, T_NEXT, 0)
    return bytes(r)


def blender_uvs_smd_from_loops(uv_layer, loop_indices):
    if len(loop_indices) != 3:
        raise ValueError("SMD triangle requires exactly three UV corners")
    return [
        (
            float(uv_layer.data[li].uv.x),
            1.0 - float(uv_layer.data[li].uv.y),
        )
        for li in loop_indices
    ]


def material_templates(src_obj):
    base = src_obj["texbase"]
    by_mat = {}
    hsets = defaultdict(set)

    for f in src_obj["faces"]:
        delta = f["lp_tex"] - base
        if delta < 0 or delta % SZ_TEX:
            raise ValueError("source face has invalid TexLink pointer")
        ti = delta // SZ_TEX
        if not 0 <= ti < src_obj["nt"]:
            raise ValueError("source TexLink index out of range")
        tex = src_obj["texlinks"][ti]
        if tex["NextTex"] != 0:
            raise ValueError("V2 target source has multi-layer TexLink")
        mat = f["material"]
        hsets[mat].add(tex["hTexture"])
        by_mat.setdefault(mat, (f["raw"], tex["raw"]))

    for mat, hs in hsets.items():
        if len(hs) != 1:
            raise ValueError(
                f"material {mat}: multiple source hTexture values {sorted(hs)}"
            )
    return by_mat, {mat: next(iter(hs)) for mat, hs in hsets.items()}


def verify_output(data, nv, nf):
    infos = parse_file(data)
    o = parse_object(data, infos, TARGET)

    if (o["nv"], o["nf"], o["nt"]) != (nv, nf, nf):
        raise ValueError("output counts mismatch")

    base = o["texbase"]
    for fi, f in enumerate(o["faces"]):
        if any(v >= nv for v in f["verts"]):
            raise ValueError(f"face {fi}: vertex OOB")
        d = f["lp_tex"] - base
        if d < 0 or d % SZ_TEX:
            raise ValueError(f"face {fi}: bad TexLink pointer")
        ti = d // SZ_TEX
        if not 0 <= ti < o["nt"]:
            raise ValueError(f"face {fi}: TexLink OOB")
        if o["texlinks"][ti]["NextTex"] != 0:
            raise ValueError(f"face {fi}: unexpected TexLink chain")

    if len(o["bones"]) != nv or any(not b for b in o["bones"]):
        raise ValueError("output Physique invalid")

    return infos, o



def sync_mesh_from_edit_mode(obj):
    """
    Commit the active Edit BMesh into the Mesh RNA datablock in memory.

    The .blend is not saved or modified on disk. Blender 5.x keeps live Edit
    Mode geometry/UV data in BMesh; the Mesh RNA UV array can legitimately
    report length 0 until Object Mode is entered.
    """
    original_mode = obj.mode
    synchronized = False

    if obj.mode == "EDIT":
        bpy.context.view_layer.objects.active = obj
        if not obj.select_get():
            obj.select_set(True)

        result = bpy.ops.object.mode_set(mode="OBJECT")
        if "FINISHED" not in result or obj.mode != "OBJECT":
            raise ValueError(
                f"failed to synchronize Edit Mode mesh: result={result}, "
                f"mode={obj.mode}"
            )
        synchronized = True

    obj.data.update()

    return {
        "original_mode": original_mode,
        "current_mode": obj.mode,
        "edit_mode_synchronized": synchronized,
    }


def require_uv_storage(mesh):
    if len(mesh.uv_layers) != 1:
        raise ValueError(
            f"requires exactly one UV layer; found {len(mesh.uv_layers)}"
        )

    uv = mesh.uv_layers.active
    if uv is None:
        raise ValueError("UV layer exists but no active UV layer is selected")

    loop_count = len(mesh.loops)
    uv_count = len(uv.data)

    if loop_count <= 0:
        raise ValueError(
            f"mesh has no corner loops: loops={loop_count}, "
            f"faces={len(mesh.polygons)}"
        )

    if uv_count != loop_count:
        raise ValueError(
            "UV storage is not synchronized/complete after Object Mode sync: "
            f"uv_layer={uv.name!r}, uv_data={uv_count}, "
            f"mesh_loops={loop_count}, faces={len(mesh.polygons)}"
        )

    return uv


def object_matrix_is_identity(obj, eps=1e-7):
    ident = Matrix.Identity(4)
    for r in range(4):
        for c in range(4):
            if abs(float(obj.matrix_world[r][c] - ident[r][c])) > eps:
                return False
    return True


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--skeleton", required=True)
    p.add_argument("--object", default=TARGET)
    p.add_argument("--out", required=True)
    p.add_argument("--report")
    p.add_argument("--max-fit-error", type=float, default=0.02)
    return p.parse_args(argv)


def main(argv):
    a = parse_args(argv)
    if a.object != TARGET:
        raise ValueError("FREE MESH WRITER V2.2 currently supports only MshC01")

    smd = Path(a.smd).expanduser().resolve()
    skel = Path(a.skeleton).expanduser().resolve()
    outp = Path(a.out).expanduser().resolve()

    src = smd.read_bytes()
    if sha(src) != SOURCE_SHA:
        raise ValueError("source is not approved pristine MfbC01.smd")

    infos = parse_file(src)
    so = parse_object(src, infos, TARGET)
    if not so["phys"]:
        raise ValueError("pristine MshC01 has no Physique")
    if so["nr"] or so["np"] or so["ns"]:
        raise ValueError("MshC01 unexpectedly has animation arrays")

    templates, htex_by_mat = material_templates(so)
    allowed_materials = set(templates)

    skeleton = parse_skeleton(skel.read_bytes())

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise ValueError("MshC01 Blender mesh missing")

    mode_sync = sync_mesh_from_edit_mode(obj)

    mesh = obj.data
    mesh.update()
    nv = len(mesh.vertices)
    source_polygons = len(mesh.polygons)

    if not 3 <= nv <= MAX_VERTICES:
        raise ValueError(f"vertex count {nv} outside safe range 3..{MAX_VERTICES}")
    if source_polygons < 1:
        raise ValueError("mesh has no polygons")
    if nv > 65535:
        raise ValueError("vertex count exceeds uint16 face-index format")

    uv = require_uv_storage(mesh)
    export_triangles = build_export_triangles(mesh)
    nf = len(export_triangles)

    if not 1 <= nf <= MAX_FACES:
        raise ValueError(
            f"triangulated export face count {nf} outside safe range "
            f"1..{MAX_FACES}"
        )

    if not object_matrix_is_identity(obj):
        raise ValueError(
            "MshC01 object transform is not identity; apply transforms "
            "(Ctrl+A) before export in V2.2"
        )

    for fi, tri in enumerate(export_triangles):
        mat = tri["material_index"]
        if mat not in allowed_materials:
            raise ValueError(
                f"export triangle {fi} / polygon {tri['polygon_index']}: "
                f"material {mat} absent from pristine MshC01; "
                f"V2.2 allowed={sorted(allowed_materials)}"
            )

    bones = []
    for vi in range(nv):
        bone = rigid_bone(obj, vi)
        if bone not in skeleton:
            raise ValueError(f"vertex {vi}: bone {bone!r} missing from skeleton")
        bones.append(bone)

    # Geometry-derived equal-weight normals, proven for newly-created topology.
    eqn = equal_normals(mesh, export_triangles)

    mags_by_bone = defaultdict(list)
    all_mags = []
    for vi, sv in enumerate(so["vertices"]):
        nx, ny, nz = sv[3:6]
        mag = math.sqrt(nx * nx + ny * ny + nz * nz)
        if mag > 0:
            mags_by_bone[so["bones"][vi]].append(mag)
            all_mags.append(mag)
    if not all_mags:
        raise ValueError("source normal calibration is empty")
    fallback_mag = statistics.median(all_mags)

    vertices = bytearray()
    physique = bytearray()
    fit_errors = []
    normal_scales = []
    vertex_rows = []

    for vi in range(nv):
        bone = bones[vi]
        tm = skeleton[bone]
        world = tuple(float(x) for x in mesh.vertices[vi].co)

        local, represented, fit = fit_local(world, tm)
        if fit > a.max_fit_error:
            raise ValueError(
                f"vertex {vi}: bind-position fit {fit} exceeds "
                f"--max-fit-error {a.max_fit_error}"
            )

        local_dir = normalize(inv_row3(eqn[vi], tm))
        scale = (
            statistics.median(mags_by_bone[bone])
            if mags_by_bone.get(bone)
            else fallback_mag
        )
        normal = tuple(int(round(x * scale)) for x in local_dir)

        vertices.extend(struct.pack("<6i", *local, *normal))
        physique.extend(bone_slot(bone))
        fit_errors.append(fit)
        normal_scales.append(scale)

        vertex_rows.append(dict(
            index=vi,
            bone=bone,
            world=list(world),
            local_fixed=list(local),
            represented_world=list(represented),
            fit_error=fit,
            normal_local=list(normal),
            normal_scale=scale,
        ))

    faces = bytearray()
    tex = bytearray()

    for fi, tri in enumerate(export_triangles):
        ids = tri["vertices"]
        if any(v < 0 or v >= nv for v in ids):
            raise ValueError(f"export triangle {fi}: invalid vertex index")
        if any(v > 65535 for v in ids):
            raise ValueError("uint16 face-index overflow")

        mat = tri["material_index"]
        face_template, tex_template = templates[mat]

        ptr = so["texbase"] + fi * SZ_TEX
        if ptr > 0xFFFFFFFF:
            raise ValueError("historical TexLink pointer overflow")

        faces.extend(face_blob(face_template, ids, mat, ptr))
        tex.extend(
            tex_blob(
                tex_template,
                blender_uvs_smd_from_loops(uv, tri["loops"]),
                htex_by_mat[mat],
            )
        )

    nt = nf

    raw = bytearray(so["raw"])
    for off, val in (
        (O_VERTEX_CAP, nv),
        (O_FACE_CAP, nf),
        (O_NV, nv),
        (O_NF, nf),
        (O_NT, nt),
    ):
        struct.pack_into("<i", raw, off, val)

    block = (
        bytes(raw)
        + bytes(vertices)
        + bytes(faces)
        + bytes(tex)
        + bytes(physique)
    )

    expected_size = (
        SZ_OBJ
        + nv * SZ_VERT
        + nf * SZ_FACE
        + nt * SZ_TEX
        + nv * 32
    )
    if len(block) != expected_size:
        raise ValueError(
            f"new MshC01 block size {len(block)} != expected {expected_size}"
        )

    prefix = bytearray(src[:infos[0]["start"]])
    blocks = []
    cursor = infos[0]["start"]
    table = []

    for inf in infos:
        if inf["name"] == TARGET:
            b = block
        else:
            b = src[inf["start"]:inf["start"] + inf["length"]]

        blocks.append(b)
        struct.pack_into("<i", prefix, inf["table_off"] + 32, len(b))
        struct.pack_into("<i", prefix, inf["table_off"] + 36, cursor)
        table.append(dict(
            name=inf["name"],
            old_start=inf["start"],
            new_start=cursor,
            old_length=inf["length"],
            new_length=len(b),
        ))
        cursor += len(b)

    out = bytes(prefix) + b"".join(blocks)

    out_infos, out_obj = verify_output(out, nv, nf)

    unrelated_ok = 0
    for old, new in zip(infos, out_infos):
        if old["name"] == TARGET:
            continue
        old_block = src[old["start"]:old["start"] + old["length"]]
        new_block = out[new["start"]:new["start"] + new["length"]]
        if old_block != new_block:
            raise ValueError(f"unrelated object changed: {old['name']}")
        unrelated_ok += 1

    outp.parent.mkdir(parents=True, exist_ok=True)
    outp.write_bytes(out)

    report = dict(
        tool="PT_SMD_MODEL_062_FREE_MESH_WRITER_V2_2",
        blend=bpy.data.filepath,
        object=TARGET,
        mode_sync=mode_sync,
        source_smd=str(smd),
        source_sha256=sha(src),
        output=str(outp),
        output_sha256=sha(out),
        source_counts=dict(
            vertices=so["nv"], faces=so["nf"], texlinks=so["nt"]
        ),
        artist_mesh=dict(
            vertices=nv,
            source_polygons=source_polygons,
            export_triangles=nf,
            triangulation_delta=nf-source_polygons,
        ),
        output_counts=dict(vertices=nv, faces=nf, texlinks=nt),
        vertex_delta=nv - so["nv"],
        face_delta=nf - so["nf"],
        artist_mesh_triangulated_in_memory=True,
        artist_mesh_saved_topology_unchanged=True,
        all_vertices_rebuilt=True,
        all_faces_rebuilt=True,
        all_texlinks_rebuilt=True,
        all_physique_rebuilt=True,
        max_vertex_fit_error=max(fit_errors) if fit_errors else 0.0,
        mean_vertex_fit_error=(
            sum(fit_errors) / len(fit_errors) if fit_errors else 0.0
        ),
        allowed_materials=sorted(allowed_materials),
        hTexture_by_material={str(k): v for k, v in htex_by_mat.items()},
        unique_rigid_bones=sorted(set(bones)),
        rigid_bone_count=len(set(bones)),
        unrelated_object_blocks_byte_identical=unrelated_ok,
        input_size=len(src),
        output_size=len(out),
        growth_bytes=len(out) - len(src),
        object_table=table,
        vertex_rows=vertex_rows,
        semantic_check="PASS",
        free_mesh_writer_check="PASS",
    )

    rp = (
        Path(a.report).expanduser().resolve()
        if a.report
        else outp.with_suffix(".json")
    )
    rp.parent.mkdir(parents=True, exist_ok=True)
    rp.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print("PT_SMD_MODEL_062_FREE_MESH_WRITER_V2_2")
    print("object=MshC01")
    print(f"original_object_mode={mode_sync['original_mode']}")
    print(f"current_object_mode={mode_sync['current_mode']}")
    print(f"edit_mode_synchronized={mode_sync['edit_mode_synchronized']}")
    print(f"source_counts={so['nv']}/{so['nf']}/{so['nt']}")
    print(f"artist_vertices={nv}")
    print(f"artist_source_polygons={source_polygons}")
    print(f"export_triangles={nf}")
    print(f"triangulation_delta={nf-source_polygons:+d}")
    print(f"output_counts={nv}/{nf}/{nt}")
    print(f"vertex_delta={nv-so['nv']:+d}")
    print(f"face_delta={nf-so['nf']:+d}")
    print("artist_mesh_triangulated_in_memory=True")
    print("artist_mesh_saved_topology_unchanged=True")
    print("all_vertices_rebuilt=True")
    print("all_faces_rebuilt=True")
    print("all_texlinks_rebuilt=True")
    print("all_physique_rebuilt=True")
    print(f"rigid_bone_count={len(set(bones))}")
    print(f"unique_rigid_bones={sorted(set(bones))}")
    print(f"allowed_materials={sorted(allowed_materials)}")
    print(
        f"max_vertex_fit_error="
        f"{(max(fit_errors) if fit_errors else 0.0):.12g}"
    )
    print(
        f"mean_vertex_fit_error="
        f"{(sum(fit_errors)/len(fit_errors) if fit_errors else 0.0):.12g}"
    )
    print(f"unrelated_object_blocks_byte_identical={unrelated_ok}")
    print(f"input_size={len(src)}")
    print(f"output_size={len(out)}")
    print(f"growth_bytes={len(out)-len(src):+d}")
    print(f"input_sha256={sha(src)}")
    print(f"output_sha256={sha(out)}")
    print(f"report={rp}")
    print("semantic_check=PASS")
    print("free_mesh_writer_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"free_mesh_writer_check=FAIL: {type(exc).__name__}: {exc}")
        raise

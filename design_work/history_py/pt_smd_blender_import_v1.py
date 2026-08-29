#!/usr/bin/env python3
# Priston Tale proprietary SMD Model data Ver 0.62 -> Blender
# V1 scope:
#   - read-only on source/client assets
#   - geometry
#   - material IDs
#   - runtime UV -> Blender UV origin conversion
#   - serialized texture filenames
#   - protected 14-byte-prefixed BMP decoding to an external cache
#   - rigid Physique names -> Blender vertex groups weight 1.0
#   - saves a .blend
#
# Not V1:
#   - runtime armature / linked M5Bip skeleton
#   - animation
#   - exact smOBJ3D transform evaluation
#   - multi-layer material rendering (MfbC01 has exactly one TexLink per face)

import argparse
import hashlib
import re
import struct
import sys
from pathlib import Path, PureWindowsPath

try:
    import bpy
except ImportError:
    bpy = None


SMD_HEADER_TEXT = b"SMD Model data Ver 0.62"

SIZE_DFILE_HEADER = 556
SIZE_DFILE_OBJINFO = 40

SIZE_MATERIAL_GROUP = 88
SIZE_MATERIAL = 320

SIZE_OBJ3D = 2236
SIZE_VERTEX = 24
SIZE_FACE = 36
SIZE_TEXLINK = 32
SIZE_TM_ROT = 20
SIZE_TM_POS = 16
SIZE_TM_SCALE = 16
SIZE_MATRIX = 64

MG_OFF_MATERIAL_COUNT = 8

M_OFF_INUSE = 0
M_OFF_TEXTURE_COUNTER = 4
M_OFF_TEXTURE_TYPE = 112
M_OFF_BLEND_TYPE = 116
M_OFF_SHADE = 120
M_OFF_TWO_SIDE = 124
M_OFF_TRANSPARENCY = 144
M_OFF_SELF_ILLUM = 148
M_OFF_ANIM_TEX_COUNTER = 304

O_OFF_TEXLINK_PTR = 12
O_OFF_PHYSIQUE_PTR = 16
O_OFF_NVERTEX = 84
O_OFF_NFACE = 88
O_OFF_NTEXLINK = 92
O_OFF_NODENAME = 172
O_OFF_NODEPARENT = 204
O_OFF_TMROTCNT = 684
O_OFF_TMPOSCNT = 688
O_OFF_TMSCALECNT = 692

BLEND_NAMES = {
    0: "NONE",
    1: "ALPHA",
    2: "COLOR",
    3: "SHADOW",
    4: "LAMP",
    5: "ADDCOLOR",
    6: "INVSHADOW",
}


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def f32(data, off):
    return struct.unpack_from("<f", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def decode_cstrings(block, expected_count):
    out = []
    pos = 0
    for _ in range(expected_count):
        end = block.find(b"\0", pos)
        if end < 0:
            raise ValueError("unterminated serialized material string")
        out.append(block[pos:end].decode("latin-1", errors="replace"))
        pos = end + 1
    if pos != len(block):
        raise ValueError(
            f"serialized material string block mismatch: consumed={pos} size={len(block)}"
        )
    return out


def ptr_to_tex_index(ptr, old_base, count):
    if ptr == 0:
        return None
    if old_base == 0 or ptr < old_base:
        raise ValueError(
            f"invalid serialized TexLink pointer 0x{ptr:08X}, base=0x{old_base:08X}"
        )
    delta = ptr - old_base
    if delta % SIZE_TEXLINK:
        raise ValueError(
            f"unaligned TexLink pointer delta={delta}, stride={SIZE_TEXLINK}"
        )
    idx = delta // SIZE_TEXLINK
    if idx >= count:
        raise ValueError(f"TexLink index out of range: {idx}/{count}")
    return idx


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

    infos = []
    table_start = SIZE_DFILE_HEADER
    table_end = table_start + obj_counter * SIZE_DFILE_OBJINFO
    if table_end != mat_file_point:
        raise ValueError(
            f"object table end {table_end} != MatFilePoint {mat_file_point}"
        )

    for i in range(obj_counter):
        off = table_start + i * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        infos.append({
            "index": i,
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


def parse_materials(data, meta):
    start = meta["mat_file_point"]
    end = meta["first_obj_point"]

    group = data[start:start + SIZE_MATERIAL_GROUP]
    if len(group) != SIZE_MATERIAL_GROUP:
        raise ValueError("truncated smMATERIAL_GROUP")

    material_count = u32(group, MG_OFF_MATERIAL_COUNT)
    if material_count != meta["mat_counter"]:
        raise ValueError(
            f"MaterialCount {material_count} != header MatCounter {meta['mat_counter']}"
        )

    cursor = start + SIZE_MATERIAL_GROUP
    materials = []

    for idx in range(material_count):
        raw = data[cursor:cursor + SIZE_MATERIAL]
        if len(raw) != SIZE_MATERIAL:
            raise ValueError(f"truncated smMATERIAL[{idx}]")
        cursor += SIZE_MATERIAL

        in_use = u32(raw, M_OFF_INUSE)
        texture_counter = u32(raw, M_OFF_TEXTURE_COUNTER)
        texture_type = u32(raw, M_OFF_TEXTURE_TYPE)
        blend_type = u32(raw, M_OFF_BLEND_TYPE)
        shade = u32(raw, M_OFF_SHADE)
        two_side = u32(raw, M_OFF_TWO_SIDE)
        transparency = f32(raw, M_OFF_TRANSPARENCY)
        self_illum = f32(raw, M_OFF_SELF_ILLUM)
        anim_count = u32(raw, M_OFF_ANIM_TEX_COUNTER)

        if texture_counter > 8:
            raise ValueError(f"material {idx}: TextureCounter={texture_counter} > 8")
        if anim_count > 32:
            raise ValueError(f"material {idx}: AnimTexCounter={anim_count} > 32")

        textures = []
        anim_textures = []

        if in_use:
            if cursor + 4 > end:
                raise ValueError(f"material {idx}: missing StrLen")
            strlen = i32(data, cursor)
            cursor += 4
            if strlen < 0 or cursor + strlen > end:
                raise ValueError(f"material {idx}: invalid StrLen={strlen}")

            block = data[cursor:cursor + strlen]
            cursor += strlen
            strings = decode_cstrings(
                block, 2 * (texture_counter + anim_count)
            )

            p = 0
            for _ in range(texture_counter):
                textures.append((strings[p], strings[p + 1]))
                p += 2
            for _ in range(anim_count):
                anim_textures.append((strings[p], strings[p + 1]))
                p += 2

        materials.append({
            "index": idx,
            "in_use": in_use,
            "texture_counter": texture_counter,
            "texture_type": texture_type,
            "blend_type": blend_type,
            "shade": shade,
            "two_side": two_side,
            "transparency": transparency,
            "self_illum": self_illum,
            "anim_count": anim_count,
            "textures": textures,
            "anim_textures": anim_textures,
        })

    if cursor != end:
        raise ValueError(
            f"material region did not close: cursor={cursor}, expected={end}"
        )

    return materials


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


def parse_objects(data, meta):
    objects = []

    for info in meta["infos"]:
        start = info["start"]
        raw = data[start:start + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise ValueError(f"{info['name']}: truncated smOBJ3D")

        old_tex_base = u32(raw, O_OFF_TEXLINK_PTR)
        physique_ptr = u32(raw, O_OFF_PHYSIQUE_PTR)

        nv = i32(raw, O_OFF_NVERTEX)
        nf = i32(raw, O_OFF_NFACE)
        nt = i32(raw, O_OFF_NTEXLINK)
        nr = i32(raw, O_OFF_TMROTCNT)
        np = i32(raw, O_OFF_TMPOSCNT)
        ns = i32(raw, O_OFF_TMSCALECNT)

        node_name = cstr(raw[O_OFF_NODENAME:O_OFF_NODENAME + 32])
        node_parent = cstr(raw[O_OFF_NODEPARENT:O_OFF_NODEPARENT + 32])

        if min(nv, nf, nt, nr, np, ns) < 0:
            raise ValueError(f"{node_name}: negative serialized count")

        has_physique = physique_ptr != 0
        layout = object_layout(start, nv, nf, nt, nr, np, ns, has_physique)

        if layout["end"] != start + info["length"]:
            raise ValueError(
                f"{node_name}: length mismatch "
                f"{layout['end'] - start} != {info['length']}"
            )

        vertices = []
        for i in range(nv):
            off = layout["vertices"] + i * SIZE_VERTEX
            x, y, z, nx, ny, nz = struct.unpack_from("<6i", data, off)
            vertices.append((x / 256.0, y / 256.0, z / 256.0))

        texlinks = []
        for i in range(nt):
            off = layout["texlinks"] + i * SIZE_TEXLINK
            u = struct.unpack_from("<3f", data, off)
            v = struct.unpack_from("<3f", data, off + 12)
            htexture = u32(data, off + 24)
            next_ptr = u32(data, off + 28)
            texlinks.append({
                "u": u,
                "v": v,
                "htexture": htexture,
                "next_ptr": next_ptr,
            })

        faces = []
        for i in range(nf):
            off = layout["faces"] + i * SIZE_FACE
            a, b, c, mat = struct.unpack_from("<4H", data, off)
            if a >= nv or b >= nv or c >= nv:
                raise ValueError(f"{node_name}: invalid face vertex at {i}")
            if mat >= meta["mat_counter"]:
                raise ValueError(f"{node_name}: invalid material ID {mat}")

            face_ptr = u32(data, off + 32)
            first = ptr_to_tex_index(face_ptr, old_tex_base, nt)

            chain = []
            seen = set()
            cur = first
            while cur is not None:
                if cur in seen:
                    raise ValueError(f"{node_name}: TexLink loop at face {i}")
                seen.add(cur)
                chain.append(cur)
                next_ptr = texlinks[cur]["next_ptr"]
                cur = ptr_to_tex_index(next_ptr, old_tex_base, nt) if next_ptr else None
                if len(chain) > 64:
                    raise ValueError(f"{node_name}: unreasonable TexLink chain")

            faces.append({
                "verts": (a, b, c),
                "material": mat,
                "texlinks": chain,
            })

        bones = None
        if has_physique:
            bones = []
            p = layout["physique"]
            for i in range(nv):
                bones.append(cstr(data[p + i * 32:p + (i + 1) * 32]))

        objects.append({
            "name": node_name or info["name"],
            "parent": node_parent,
            "vertices": vertices,
            "faces": faces,
            "texlinks": texlinks,
            "bones": bones,
            "has_physique": has_physique,
        })

    return objects


def resolve_case_insensitive(root, relative_windows_name):
    if not relative_windows_name:
        return None

    p = PureWindowsPath(relative_windows_name)
    parts = list(p.parts)

    if p.drive:
        parts = parts[1:]

    cur = root
    for part in parts:
        exact = cur / part
        if exact.exists():
            cur = exact
            continue

        if not cur.is_dir():
            return None

        target = part.lower()
        matches = [x for x in cur.iterdir() if x.name.lower() == target]
        if not matches:
            return None
        matches.sort(key=lambda x: x.name)
        cur = matches[0]

    return cur if cur.exists() else None


def decoded_cache_name(src, cache_dir):
    key = hashlib.sha1(str(src).encode("utf-8")).hexdigest()[:10]
    return cache_dir / f"{src.stem}.{key}.decoded{src.suffix.lower()}"


def prepare_bmp(src, cache_dir):
    data = bytearray(src.read_bytes())

    if data[:2] == b"BM":
        return src, "BMP_STANDARD"

    if len(data) < 54:
        return src, "BMP_UNKNOWN_SHORT"

    dib = int.from_bytes(data[14:18], "little")
    if dib < 40 or dib > 124:
        return src, "BMP_UNKNOWN_HEADER"

    w = int.from_bytes(data[18:22], "little", signed=True)
    h = int.from_bytes(data[22:26], "little", signed=True)
    planes = int.from_bytes(data[26:28], "little")
    bpp = int.from_bytes(data[28:30], "little")
    compression = int.from_bytes(data[30:34], "little")

    if not (1 <= abs(w) <= 16384 and 1 <= abs(h) <= 16384):
        return src, "BMP_UNKNOWN_DIMENSIONS"
    if planes != 1 or bpp not in (1, 4, 8, 16, 24, 32):
        return src, "BMP_UNKNOWN_FORMAT"

    pixel_offset = 14 + dib
    if dib == 40 and compression == 3 and bpp in (16, 32):
        pixel_offset += 12
    if bpp <= 8:
        clr_used = int.from_bytes(data[46:50], "little") if len(data) >= 50 else 0
        palette_entries = clr_used if clr_used else (1 << bpp)
        pixel_offset += palette_entries * 4

    data[:14] = struct.pack(
        "<2sIHHI", b"BM", len(data), 0, 0, pixel_offset
    )

    cache_dir.mkdir(parents=True, exist_ok=True)
    out = decoded_cache_name(src, cache_dir)
    out.write_bytes(data)
    return out, f"BMP_PRISTON_PREFIX14_{w}x{h}_{bpp}bpp"


def prepare_texture(src, cache_dir):
    if src is None:
        return None, "NOT_FOUND"
    if src.suffix.lower() == ".bmp":
        return prepare_bmp(src, cache_dir)
    return src, "AS_IS"


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def ensure_collection(name):
    col = bpy.data.collections.get(name)
    if col is None:
        col = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(col)
    return col


def safe_name(s):
    s = re.sub(r"[^A-Za-z0-9_.-]+", "_", s or "Material")
    return s[:100]


def create_blender_materials(materials, client_root, cache_dir, pack_images=True):
    out = []

    for m in materials:
        tex_name = m["textures"][0][0] if m["textures"] else ""
        src = resolve_case_insensitive(client_root, tex_name) if tex_name else None
        prepared, decode_tag = prepare_texture(src, cache_dir)

        label = PureWindowsPath(tex_name).name if tex_name else "NO_TEXTURE"
        mat = bpy.data.materials.new(
            name=f"SMD_MAT_{m['index']:02d}_{safe_name(label)}"
        )
        mat.use_nodes = True

        mat["pt_smd_material_id"] = m["index"]
        mat["pt_smd_in_use"] = int(m["in_use"])
        mat["pt_smd_texture_name"] = tex_name
        mat["pt_smd_texture_decode"] = decode_tag
        mat["pt_smd_blend_type"] = int(m["blend_type"])
        mat["pt_smd_blend_name"] = BLEND_NAMES.get(m["blend_type"], "UNKNOWN")
        mat["pt_smd_two_side"] = int(m["two_side"])
        mat["pt_smd_shade"] = int(m["shade"])
        mat["pt_smd_transparency"] = float(m["transparency"])
        mat["pt_smd_self_illum"] = float(m["self_illum"])

        if hasattr(mat, "use_backface_culling"):
            try:
                mat.use_backface_culling = not bool(m["two_side"])
            except Exception:
                pass

        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        bsdf = nodes.get("Principled BSDF")

        if bsdf:
            if "Base Color" in bsdf.inputs:
                bsdf.inputs["Base Color"].default_value = (1.0, 1.0, 1.0, 1.0)
            if "Roughness" in bsdf.inputs:
                bsdf.inputs["Roughness"].default_value = 0.7
            if "Alpha" in bsdf.inputs:
                bsdf.inputs["Alpha"].default_value = max(
                    0.0, min(1.0, 1.0 - m["transparency"])
                )

        load_status = "NO_TEXTURE"

        if prepared is not None:
            try:
                image = bpy.data.images.load(str(prepared), check_existing=False)
                image.reload()
                if int(image.size[0]) <= 0 or int(image.size[1]) <= 0:
                    raise RuntimeError(f"invalid image size {tuple(image.size)}")
                if len(image.pixels) > 0:
                    _ = image.pixels[0]

                try:
                    image.colorspace_settings.name = "sRGB"
                except Exception:
                    pass

                tex = nodes.new("ShaderNodeTexImage")
                tex.name = "PT_SMD_DIFFUSE"
                tex.label = Path(prepared).name
                tex.image = image

                if bsdf:
                    links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
                    if "Alpha" in tex.outputs and "Alpha" in bsdf.inputs:
                        links.new(tex.outputs["Alpha"], bsdf.inputs["Alpha"])

                if pack_images:
                    try:
                        image.pack()
                    except Exception as exc:
                        print(f"WARN image.pack failed: {prepared}: {exc}")

                load_status = "LOADED"
            except Exception as exc:
                load_status = f"LOAD_ERROR:{type(exc).__name__}:{exc}"

        mat["pt_smd_texture_load"] = load_status
        print(
            f"MATERIAL {m['index']}: tex={tex_name!r} "
            f"resolved={str(src) if src else None!r} "
            f"prepared={str(prepared) if prepared else None!r} "
            f"decode={decode_tag} load={load_status}"
        )

        out.append(mat)

    return out


def create_mesh_object(src_obj, collection, blender_materials):
    verts = src_obj["vertices"]
    face_defs = src_obj["faces"]
    faces = [x["verts"] for x in face_defs]

    mesh = bpy.data.meshes.new(f"{src_obj['name']}_Mesh")
    mesh.from_pydata(verts, [], faces)
    mesh.update(calc_edges=True)

    obj = bpy.data.objects.new(src_obj["name"], mesh)
    collection.objects.link(obj)

    for mat in blender_materials:
        mesh.materials.append(mat)

    for poly_index, fd in enumerate(face_defs):
        mesh.polygons[poly_index].material_index = fd["material"]

    uv_layer = mesh.uv_layers.new(name="SMD_UV")
    uv_faces = 0
    max_layers = 0

    for poly_index, fd in enumerate(face_defs):
        chain = fd["texlinks"]
        max_layers = max(max_layers, len(chain))
        if not chain:
            continue

        tl = src_obj["texlinks"][chain[0]]
        coords = [
            (float(tl["u"][i]), 1.0 - float(tl["v"][i]))
            for i in range(3)
        ]

        poly = mesh.polygons[poly_index]
        for loop_idx, uv in zip(poly.loop_indices, coords):
            uv_layer.data[loop_idx].uv = uv
        uv_faces += 1

    groups = {}
    if src_obj["bones"] is not None:
        for vidx, bone_name in enumerate(src_obj["bones"]):
            if not bone_name:
                continue
            group = groups.get(bone_name)
            if group is None:
                group = obj.vertex_groups.new(name=bone_name)
                groups[bone_name] = group
            group.add([vidx], 1.0, "REPLACE")

    obj["pt_smd_node_parent"] = src_obj["parent"]
    obj["pt_smd_vertex_count"] = len(verts)
    obj["pt_smd_face_count"] = len(faces)
    obj["pt_smd_texlink_count"] = len(src_obj["texlinks"])
    obj["pt_smd_uv_faces"] = uv_faces
    obj["pt_smd_max_tex_layers"] = max_layers
    obj["pt_smd_has_physique"] = int(src_obj["has_physique"])
    obj["pt_smd_vertex_group_count"] = len(groups)
    obj["pt_smd_coordinate_rule"] = "XYZ=int(ASE_XYZ*256); Blender=XYZ/256"
    obj["pt_smd_uv_rule"] = "Blender_U=SMD_U; Blender_V=1-SMD_V"

    return obj, {
        "name": src_obj["name"],
        "vertices": len(verts),
        "faces": len(faces),
        "uv_faces": uv_faces,
        "max_layers": max_layers,
        "vertex_groups": len(groups),
    }


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--client-root", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--cache")
    p.add_argument("--no-pack", action="store_true")
    return p.parse_args(argv)


def main(argv):
    if bpy is None:
        raise RuntimeError("This script must run inside Blender")

    args = parse_args(argv)
    smd_path = Path(args.smd).resolve()
    client_root = Path(args.client_root).resolve()
    out_path = Path(args.out).resolve()
    cache_dir = (
        Path(args.cache).resolve()
        if args.cache
        else out_path.parent / "_pt_smd_texture_cache"
    )

    data = smd_path.read_bytes()
    meta = parse_header_and_infos(data)
    materials = parse_materials(data, meta)
    objects = parse_objects(data, meta)

    print("PT_SMD_BLENDER_IMPORT_V1")
    print("SMD        :", smd_path)
    print("Client root:", client_root)
    print("Output     :", out_path)
    print("Objects    :", len(objects))
    print("Materials  :", len(materials))

    clear_scene()
    collection = ensure_collection("PT_SMD_IMPORT_V1")

    blender_materials = create_blender_materials(
        materials,
        client_root,
        cache_dir,
        pack_images=not args.no_pack,
    )

    stats = []
    for src_obj in objects:
        obj, info = create_mesh_object(
            src_obj, collection, blender_materials
        )
        stats.append(info)
        print(
            f"OBJECT {info['name']}: "
            f"V={info['vertices']} F={info['faces']} "
            f"UV={info['uv_faces']} layers={info['max_layers']} "
            f"groups={info['vertex_groups']}"
        )

    total_v = sum(x["vertices"] for x in stats)
    total_f = sum(x["faces"] for x in stats)
    total_uv = sum(x["uv_faces"] for x in stats)

    bpy.context.scene["pt_smd_source"] = str(smd_path)
    bpy.context.scene["pt_smd_version"] = "SMD Model data Ver 0.62"
    bpy.context.scene["pt_smd_object_count"] = len(objects)
    bpy.context.scene["pt_smd_material_count"] = len(materials)
    bpy.context.scene["pt_smd_total_vertices"] = total_v
    bpy.context.scene["pt_smd_total_faces"] = total_f
    bpy.context.scene["pt_smd_total_uv_faces"] = total_uv

    out_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out_path))

    print("SUMMARY")
    print(f"objects={len(objects)}")
    print(f"materials={len(materials)}")
    print(f"vertices={total_v}")
    print(f"faces={total_f}")
    print(f"uv_faces={total_uv}")
    print(f"blend={out_path}")
    print("import_check=PASS")
    return 0


if __name__ == "__main__":
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] if "--" in argv else []
    raise SystemExit(main(argv))

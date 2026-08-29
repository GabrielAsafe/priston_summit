# pt_ase_import_v2.py
# Summit Tale / Priston Tale ASE -> Blender
#
# V3 - V2.2 geometry/UV/materials/textures + Biped armature hierarchy.
# Read-only importer: it does NOT export back to Priston formats.
# V3 imports NODE_NAME/NODE_PARENT/NODE_TM positions for Bip* nodes.
# PHYSIQUE/vertex weights are intentionally deferred to V4.
#
# Features:
#   - imports non-Bip/non-Dummy GEOMOBJECT meshes that have UVs (default)
#   - preserves MESH_MTLID per face
#   - parses MATERIAL / SUBMATERIAL / MATERIAL_REF
#   - resolves BITMAP references against the Summit client tree
#   - decodes the proven 14-byte-prefixed Priston BMP layout when detected
#   - decodes the Summit/Priston protected-TGA layout observed in runtime assets:
#       18-byte proprietary replacement header
#       + raw BGR/BGRA pixel payload
#       + optional 26-byte TGA 2.0 footer
#     V2.2 only auto-decodes square power-of-two 24/32-bit textures.
#   - forces Blender to actually decode image pixels during import, avoiding
#     false-positive LOADED states from lazy image datablocks.
#   - creates Blender node materials and packs loaded images into the .blend
#
# Usage (Flatpak Blender):
#   flatpak run --filesystem=home org.blender.Blender \
#     --background --python pt_ase_import_v2.py -- \
#     --ase "/path/file.ASE" \
#     --client-root "/path/cliente" \
#     --out "/path/output.blend"

import argparse
import bpy
import hashlib
import re
import struct
import sys
from pathlib import Path

FLOAT = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"
IMAGE_EXTS = {".bmp", ".tga", ".png", ".jpg", ".jpeg", ".dds"}


# -----------------------------------------------------------------------------
# ASE balanced-block helpers
# -----------------------------------------------------------------------------

def _balanced_block(text, start, brace_pos):
    depth = 0
    i = brace_pos
    in_quote = False
    escaped = False

    while i < len(text):
        ch = text[i]

        if in_quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_quote = False
        else:
            if ch == '"':
                in_quote = True
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return text[start:i + 1]
        i += 1

    raise ValueError("ASE block without matching closing brace")


def extract_plain_blocks(text, keyword):
    """Extract exact blocks like *GEOMOBJECT { or *MESH {."""
    rx = re.compile(
        rf"(?mi)^[ \t]*\*{re.escape(keyword)}[ \t]*\{{"
    )
    out = []
    for m in rx.finditer(text):
        brace = text.find("{", m.start(), m.end())
        out.append(_balanced_block(text, m.start(), brace))
    return out


def extract_indexed_blocks(text, keyword):
    """Extract exact indexed blocks like *MATERIAL 0 {, *SUBMATERIAL 1 {."""
    rx = re.compile(
        rf"(?mi)^[ \t]*\*{re.escape(keyword)}[ \t]+(\d+)[ \t]*\{{"
    )
    out = []
    for m in rx.finditer(text):
        brace = text.find("{", m.start(), m.end())
        out.append((int(m.group(1)), _balanced_block(text, m.start(), brace)))
    return out


def first_quoted(block, key):
    m = re.search(
        re.escape(key) + r'\s+"([^"]*)"',
        block,
        flags=re.IGNORECASE,
    )
    return m.group(1) if m else None


def first_int(block, key, default=None):
    m = re.search(
        re.escape(key) + r"\s+(-?\d+)",
        block,
        flags=re.IGNORECASE,
    )
    return int(m.group(1)) if m else default


def first_vec3(block, key, default=None):
    m = re.search(
        re.escape(key) + rf"\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
        block,
        flags=re.IGNORECASE,
    )
    if not m:
        return default
    return tuple(float(m.group(i)) for i in (1, 2, 3))


# -----------------------------------------------------------------------------
# ASE mesh parsers
# -----------------------------------------------------------------------------

def parse_vertices(mesh_block):
    out = {}
    rx = re.compile(
        rf"\*MESH_VERTEX\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
        re.IGNORECASE,
    )
    for m in rx.finditer(mesh_block):
        out[int(m.group(1))] = (
            float(m.group(2)),
            float(m.group(3)),
            float(m.group(4)),
        )
    return out


def parse_faces(mesh_block):
    """face_index -> ((a,b,c), mtl_id)"""
    out = {}
    face_rx = re.compile(
        r"\*MESH_FACE\s+(\d+):"
        r".*?\bA:\s*(\d+)"
        r".*?\bB:\s*(\d+)"
        r".*?\bC:\s*(\d+)",
        re.IGNORECASE,
    )
    mtl_rx = re.compile(r"\*MESH_MTLID\s+(\d+)", re.IGNORECASE)

    for line in mesh_block.splitlines():
        m = face_rx.search(line)
        if not m:
            continue
        idx = int(m.group(1))
        tri = (int(m.group(2)), int(m.group(3)), int(m.group(4)))
        mm = mtl_rx.search(line)
        mtl_id = int(mm.group(1)) if mm else 0
        out[idx] = (tri, mtl_id)
    return out


def parse_tverts(mesh_block):
    out = {}
    rx = re.compile(
        rf"\*MESH_TVERT\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
        re.IGNORECASE,
    )
    for m in rx.finditer(mesh_block):
        out[int(m.group(1))] = (
            float(m.group(2)),
            float(m.group(3)),
            float(m.group(4)),
        )
    return out


def parse_tfaces(mesh_block):
    out = {}
    rx = re.compile(
        r"\*MESH_TFACE\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)",
        re.IGNORECASE,
    )
    for m in rx.finditer(mesh_block):
        out[int(m.group(1))] = (
            int(m.group(2)),
            int(m.group(3)),
            int(m.group(4)),
        )
    return out


def dense_vertex_list(vertex_map):
    if not vertex_map:
        return []
    max_idx = max(vertex_map)
    verts = [(0.0, 0.0, 0.0)] * (max_idx + 1)
    for idx, co in vertex_map.items():
        verts[idx] = co
    return verts


# -----------------------------------------------------------------------------
# ASE material parsing
# -----------------------------------------------------------------------------

def material_desc(block, idx):
    return {
        "index": idx,
        "name": first_quoted(block, "*MATERIAL_NAME") or f"Material_{idx}",
        "bitmap": first_quoted(block, "*BITMAP"),
        "diffuse": first_vec3(block, "*MATERIAL_DIFFUSE", (0.8, 0.8, 0.8)),
        "transparency": float(
            re.search(rf"\*MATERIAL_TRANSPARENCY\s+({FLOAT})", block, re.I).group(1)
        ) if re.search(rf"\*MATERIAL_TRANSPARENCY\s+({FLOAT})", block, re.I) else 0.0,
        "submaterials": {},
    }


def parse_materials(text):
    result = {}

    for mid, block in extract_indexed_blocks(text, "MATERIAL"):
        desc = material_desc(block, mid)

        for sid, sblock in extract_indexed_blocks(block, "SUBMATERIAL"):
            sdesc = material_desc(sblock, sid)
            desc["submaterials"][sid] = sdesc

        result[mid] = desc

    return result


def select_material_desc(materials, material_ref, mesh_mtl_id):
    top = materials.get(material_ref)
    if top is None:
        return None
    if top["submaterials"]:
        return top["submaterials"].get(mesh_mtl_id, top)
    return top


# -----------------------------------------------------------------------------
# Client texture index / path resolution
# -----------------------------------------------------------------------------

def normalize_ase_path(raw):
    if not raw:
        return ""
    return raw.replace("\\", "/").strip()


def build_image_index(client_root):
    by_basename = {}
    total = 0

    for p in client_root.rglob("*"):
        if not p.is_file():
            continue
        if p.suffix.lower() not in IMAGE_EXTS:
            continue
        total += 1
        by_basename.setdefault(p.name.lower(), []).append(p)

    return by_basename, total


def path_hint_parts(raw):
    norm = normalize_ase_path(raw)
    parts = [p for p in norm.split("/") if p]
    low = [p.lower() for p in parts]

    markers = {"char", "field", "image", "effect", "startimage", "game", "cselect"}
    for i, part in enumerate(low):
        if part in markers:
            return parts[i:]
    return [parts[-1]] if parts else []


def resolve_texture(raw, client_root, ase_dir, image_index):
    if not raw:
        return None, []

    raw_norm = normalize_ase_path(raw)
    basename = Path(raw_norm).name
    candidates = list(image_index.get(basename.lower(), []))
    hint = path_hint_parts(raw)
    hint_lower = "/".join(x.lower() for x in hint)

    def score(p):
        s = 0
        pstr = str(p).replace("\\", "/").lower()

        try:
            rel = p.relative_to(client_root)
            relstr = str(rel).replace("\\", "/").lower()
        except Exception:
            relstr = pstr

        if p.parent == ase_dir:
            s += 500

        if hint_lower and relstr.endswith(hint_lower):
            s += 1000

        if "/char/tmabcd/" in pstr:
            s += 100

        bad_words = (
            "disabled",
            "backup",
            ".before_",
            "before_design_test",
            "design_work",
            "from_pristontale_master",
        )
        for word in bad_words:
            if word in pstr:
                s -= 1000

        # Prefer shorter canonical-looking paths if all else is equal.
        s -= len(relstr) / 10000.0
        return s

    candidates.sort(key=score, reverse=True)
    return (candidates[0] if candidates else None), candidates


# -----------------------------------------------------------------------------
# Priston image decoding/probing
# -----------------------------------------------------------------------------

def cache_name(src, suffix_extra, cache_dir):
    key = hashlib.sha1(str(src).encode("utf-8")).hexdigest()[:10]
    return cache_dir / f"{src.stem}.{key}{suffix_extra}{src.suffix.lower()}"


def decode_prefixed_bmp(src, cache_dir):
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

    # Compute a conventional BMP pixel offset.
    pixel_offset = 14 + dib

    if dib == 40 and compression == 3 and bpp in (16, 32):
        pixel_offset += 12  # RGB bit masks after BITMAPINFOHEADER

    if bpp <= 8:
        clr_used = int.from_bytes(data[46:50], "little") if len(data) >= 50 else 0
        palette_entries = clr_used if clr_used else (1 << bpp)
        pixel_offset += palette_entries * 4

    decoded = bytearray(data)
    decoded[:14] = struct.pack(
        "<2sIHHI",
        b"BM",
        len(decoded),
        0,
        0,
        pixel_offset,
    )

    out = cache_name(src, ".decoded", cache_dir)
    out.write_bytes(decoded)

    tag = f"BMP_PRISTON_PREFIX14_{w}x{h}_{bpp}bpp"
    return out, tag


def tga_header_at(data, off):
    if len(data) < off + 18:
        return None

    id_len = data[off + 0]
    cmap_type = data[off + 1]
    image_type = data[off + 2]
    cmap_len = int.from_bytes(data[off + 5:off + 7], "little")
    cmap_depth = data[off + 7]
    w = int.from_bytes(data[off + 12:off + 14], "little")
    h = int.from_bytes(data[off + 14:off + 16], "little")
    bpp = data[off + 16]

    if cmap_type not in (0, 1):
        return None
    if image_type not in (1, 2, 3, 9, 10, 11):
        return None
    if not (1 <= w <= 16384 and 1 <= h <= 16384):
        return None
    if bpp not in (8, 15, 16, 24, 32):
        return None

    cmap_bytes = 0
    if cmap_type == 1:
        cmap_bytes = cmap_len * ((cmap_depth + 7) // 8)

    data_start = off + 18 + id_len + cmap_bytes
    if data_start >= len(data):
        return None

    # For uncompressed truecolor/greyscale, verify there are enough pixel bytes.
    if image_type in (2, 3):
        bytes_pp = (bpp + 7) // 8
        expected = w * h * bytes_pp
        if data_start + expected > len(data):
            return None

    return {
        "offset": off,
        "type": image_type,
        "width": w,
        "height": h,
        "bpp": bpp,
    }


def _is_power_of_two(v):
    return v > 0 and (v & (v - 1)) == 0


def _standard_tga_header(width, height, bpp, top_origin=False):
    """
    Build an 18-byte uncompressed true-color TGA header.

    TGA true-color pixel byte order is BGR/BGRA, which matches the raw
    payload pattern observed in the protected Summit assets.

    We default to bottom-left origin (classic TGA). If a texture is visually
    inverted in Blender, use the ASE importer's existing --flip-v option
    rather than rewriting source pixels.
    """
    descriptor = 8 if bpp == 32 else 0
    if top_origin:
        descriptor |= 0x20

    return struct.pack(
        "<BBBHHBHHHHBB",
        0,          # ID length
        0,          # no color map
        2,          # uncompressed true-color
        0,          # color-map first entry
        0,          # color-map length
        0,          # color-map entry depth
        0,          # X origin
        0,          # Y origin
        width,
        height,
        bpp,
        descriptor,
    )


def _has_tga2_footer(data):
    """
    TGA 2.0 footer is 26 bytes:
      4-byte extension-area offset
      4-byte developer-directory offset
      18-byte signature "TRUEVISION-XFILE.\\0"

    Summit samples proved this exact trailer:
      00 00 00 00 00 00 00 00
      54 52 55 45 56 49 53 49 4f 4e 2d 58 46 49 4c 45 2e 00
    """
    if len(data) < 26:
        return False

    footer = data[-26:]
    signature = b"TRUEVISION-XFILE.\x00"

    return footer[8:] == signature


def _infer_square_tga_payload(payload):
    """
    Conservative V2.2 inference.

    Returns (width, height, bpp) only when payload size maps exactly to a
    square power-of-two 24-bit or 32-bit image.

    This intentionally avoids guessing rectangular dimensions such as
    64x128 until the proprietary 18-byte header encoding is understood.
    """
    for bpp, bytes_pp in ((32, 4), (24, 3)):
        if len(payload) % bytes_pp:
            continue

        pixels = len(payload) // bytes_pp
        side = int(pixels ** 0.5)

        if side * side != pixels:
            continue
        if not _is_power_of_two(side):
            continue
        if not (1 <= side <= 16384):
            continue

        return side, side, bpp

    return None


def decode_prefixed_tga(src, cache_dir):
    data = src.read_bytes()

    # Normal TGA: leave untouched.
    standard = tga_header_at(data, 0)
    if standard:
        return (
            src,
            f"TGA_STANDARD_{standard['width']}x"
            f"{standard['height']}_{standard['bpp']}bpp"
        )

    if len(data) < 18:
        return src, "TGA_UNKNOWN_SHORT"

    # ------------------------------------------------------------------
    # Summit protected-TGA layout discovered from char/tmABCD assets.
    #
    # Observed example:
    #   total size = 262188
    #              = 18 + 262144 + 26
    #
    # The first 18 bytes are identical proprietary bytes:
    #   47 38 06 09 10 19 24 31 40 51 64 79 90 aa c4 e2 20 29
    #
    # For pr100_yun01/02, byte 18 onward visibly has BGRA-like pixel data.
    #
    # If a standard TGA 2.0 footer exists, exclude its 26 bytes.
    # Otherwise try the full remainder after the proprietary 18-byte header.
    # ------------------------------------------------------------------

    footer26 = _has_tga2_footer(data)

    if footer26:
        payload = data[18:-26]
        layout = "CUSTOM18_FOOTER26"
    else:
        payload = data[18:]
        layout = "CUSTOM18_NOFOOTER"

    inferred = _infer_square_tga_payload(payload)

    if not inferred:
        return (
            src,
            f"TGA_UNKNOWN_HEADER_{layout}_PAYLOAD{len(payload)}"
        )

    width, height, bpp = inferred

    decoded = (
        _standard_tga_header(width, height, bpp, top_origin=False)
        + payload
    )

    out = cache_name(src, ".decoded", cache_dir)
    out.write_bytes(decoded)

    # Validate our own output structurally before Blender sees it.
    check = tga_header_at(decoded, 0)
    if not check:
        return src, "TGA_REBUILD_INTERNAL_VALIDATION_FAILED"

    tag = (
        f"TGA_PRISTON_{layout}_"
        f"{width}x{height}_{bpp}bpp"
    )

    return out, tag


def prepare_texture(src, cache_dir):
    ext = src.suffix.lower()
    try:
        if ext == ".bmp":
            return decode_prefixed_bmp(src, cache_dir)
        if ext == ".tga":
            return decode_prefixed_tga(src, cache_dir)
        return src, "STANDARD_OTHER"
    except Exception as exc:
        return src, f"DECODE_ERROR:{type(exc).__name__}:{exc}"


# -----------------------------------------------------------------------------
# Blender material creation
# -----------------------------------------------------------------------------

def make_blender_material(name, desc, texture_path, decode_tag, pack_images=True):
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True

    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    bsdf = nodes.get("Principled BSDF")

    diffuse = desc.get("diffuse") if desc else (0.8, 0.8, 0.8)
    transparency = desc.get("transparency", 0.0) if desc else 0.0

    if bsdf:
        if "Base Color" in bsdf.inputs:
            bsdf.inputs["Base Color"].default_value = (
                float(diffuse[0]), float(diffuse[1]), float(diffuse[2]), 1.0
            )
        if "Roughness" in bsdf.inputs:
            bsdf.inputs["Roughness"].default_value = 0.7
        if "Alpha" in bsdf.inputs:
            bsdf.inputs["Alpha"].default_value = max(0.0, min(1.0, 1.0 - transparency))

    mat["pt_ase_bitmap"] = desc.get("bitmap") if desc else ""
    mat["pt_texture_decode"] = decode_tag or ""

    if texture_path is None:
        return mat, None, "NO_TEXTURE"

    try:
        image = bpy.data.images.load(str(texture_path), check_existing=False)

        # Blender can create an Image datablock lazily even when the bytes are
        # not decodable. Force a real decode now so LOADED means LOADED.
        image.reload()

        iw, ih = int(image.size[0]), int(image.size[1])
        if iw <= 0 or ih <= 0:
            raise RuntimeError(
                f"decoded image has invalid size {iw}x{ih}: {texture_path}"
            )

        # Touch one pixel to force pixel-buffer decoding in background mode.
        if len(image.pixels) > 0:
            _ = image.pixels[0]

        try:
            image.colorspace_settings.name = "sRGB"
        except Exception:
            pass

        tex = nodes.new("ShaderNodeTexImage")
        tex.name = "PT_ASE_DIFFUSE"
        tex.label = Path(texture_path).name
        tex.image = image

        if bsdf:
            links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
            if "Alpha" in tex.outputs and "Alpha" in bsdf.inputs:
                links.new(tex.outputs["Alpha"], bsdf.inputs["Alpha"])

        if pack_images:
            try:
                image.pack()
            except Exception as exc:
                print(f"  WARN pack failed: {texture_path}: {exc}")

        return mat, image, "LOADED"

    except Exception as exc:
        return mat, None, f"LOAD_ERROR:{type(exc).__name__}:{exc}"


# -----------------------------------------------------------------------------
# Scene import
# -----------------------------------------------------------------------------

def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def ensure_collection(name="PT_ASE_IMPORT_V2"):
    col = bpy.data.collections.get(name)
    if col is None:
        col = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(col)
    return col


def safe_name(s):
    s = re.sub(r"[^A-Za-z0-9_.-]+", "_", s or "Material")
    return s[:80]


def import_geomobject(
    block,
    collection,
    materials,
    client_root,
    ase_dir,
    image_index,
    cache_dir,
    import_zero_uv=False,
    flip_v=False,
    pack_images=True,
):
    node_name = first_quoted(block, "*NODE_NAME") or "Unnamed"
    low = node_name.lower()

    if low.startswith("bip") or low.startswith("dummy"):
        return None

    mesh_blocks = extract_plain_blocks(block, "MESH")
    if not mesh_blocks:
        return None

    mesh_block = mesh_blocks[0]
    verts_map = parse_vertices(mesh_block)
    faces_map = parse_faces(mesh_block)
    tverts = parse_tverts(mesh_block)
    tfaces = parse_tfaces(mesh_block)

    if not verts_map or not faces_map:
        return None
    if not import_zero_uv and not tverts:
        return None

    material_ref = first_int(block, "*MATERIAL_REF", 0)

    verts = dense_vertex_list(verts_map)
    face_indices = sorted(faces_map)
    faces = [faces_map[i][0] for i in face_indices]

    max_vert = len(verts) - 1
    for face_idx, tri in zip(face_indices, faces):
        for vi in tri:
            if vi < 0 or vi > max_vert:
                raise ValueError(
                    f"{node_name}: face {face_idx} invalid vertex {vi}; 0..{max_vert}"
                )

    mesh = bpy.data.meshes.new(f"{node_name}_Mesh")
    mesh.from_pydata(verts, [], faces)
    mesh.update(calc_edges=True)

    obj = bpy.data.objects.new(node_name, mesh)
    collection.objects.link(obj)

    # UV
    uv_faces_applied = 0
    if tverts and tfaces:
        uv_layer = mesh.uv_layers.new(name="ASE_UV")
        for poly_index, face_idx in enumerate(face_indices):
            tf = tfaces.get(face_idx)
            if tf is None:
                continue
            poly = mesh.polygons[poly_index]
            if len(poly.loop_indices) != 3:
                continue
            coords = []
            valid = True
            for tvi in tf:
                tv = tverts.get(tvi)
                if tv is None:
                    valid = False
                    break
                u = tv[0]
                v = 1.0 - tv[1] if flip_v else tv[1]
                coords.append((u, v))
            if not valid:
                continue
            for loop_idx, uv in zip(poly.loop_indices, coords):
                uv_layer.data[loop_idx].uv = uv
            uv_faces_applied += 1

    # Material slots, one per MESH_MTLID used by this object.
    used_mtl_ids = sorted({faces_map[i][1] for i in face_indices})
    slot_by_id = {}
    material_results = []

    for mesh_mtl_id in used_mtl_ids:
        desc = select_material_desc(materials, material_ref, mesh_mtl_id)
        raw_bitmap = desc.get("bitmap") if desc else None
        resolved = None
        candidates = []
        prepared = None
        decode_tag = "NO_BITMAP"

        if raw_bitmap:
            resolved, candidates = resolve_texture(
                raw_bitmap,
                client_root,
                ase_dir,
                image_index,
            )

        if resolved:
            prepared, decode_tag = prepare_texture(resolved, cache_dir)

        desc_name = desc.get("name") if desc else "MissingMaterial"
        tex_label = Path(normalize_ase_path(raw_bitmap)).name if raw_bitmap else "no_bitmap"
        mat_name = safe_name(
            f"ASE_M{material_ref}_S{mesh_mtl_id}_{desc_name}_{tex_label}"
        )

        mat, image, load_status = make_blender_material(
            mat_name,
            desc or {"bitmap": raw_bitmap},
            prepared if resolved else None,
            decode_tag,
            pack_images=pack_images,
        )

        slot_by_id[mesh_mtl_id] = len(mesh.materials)
        mesh.materials.append(mat)

        material_results.append({
            "mtl_id": mesh_mtl_id,
            "material_ref": material_ref,
            "material_name": desc_name,
            "bitmap_raw": raw_bitmap,
            "resolved": str(resolved) if resolved else None,
            "candidate_count": len(candidates),
            "prepared": str(prepared) if prepared else None,
            "decode": decode_tag,
            "load": load_status,
        })

    for poly_index, face_idx in enumerate(face_indices):
        mesh_mtl_id = faces_map[face_idx][1]
        mesh.polygons[poly_index].material_index = slot_by_id[mesh_mtl_id]

    # Custom metadata for later V3/V4/debugging.
    obj["pt_ase_node_name"] = node_name
    obj["pt_ase_material_ref"] = material_ref
    obj["pt_ase_vertex_count"] = len(verts_map)
    obj["pt_ase_face_count"] = len(faces_map)
    obj["pt_ase_tvert_count"] = len(tverts)
    obj["pt_ase_tface_count"] = len(tfaces)
    obj["pt_ase_uv_faces_applied"] = uv_faces_applied

    return {
        "name": node_name,
        "verts": len(verts_map),
        "faces": len(faces_map),
        "tverts": len(tverts),
        "tfaces": len(tfaces),
        "uv_faces_applied": uv_faces_applied,
        "material_ref": material_ref,
        "materials": material_results,
    }



# -----------------------------------------------------------------------------
# V3: Biped skeleton / NODE hierarchy
# -----------------------------------------------------------------------------

def parse_biped_nodes(geom_blocks):
    """
    Read Bip* nodes from GEOMOBJECT blocks.

    V3 deliberately uses only:
      *NODE_NAME
      *NODE_PARENT
      *NODE_TM
      *TM_ROW3 (fallback: *TM_POS)

    TM_ROW0/1/2 are preserved as metadata for later investigation, but V3 does
    not use them to define Blender bone roll/orientation yet.
    """
    nodes = []
    seen = set()

    for order, block in enumerate(geom_blocks):
        name = first_quoted(block, "*NODE_NAME")
        if not name or not name.lower().startswith("bip"):
            continue

        if name in seen:
            # Keep first occurrence to avoid duplicate Blender bone names.
            continue
        seen.add(name)

        parent = first_quoted(block, "*NODE_PARENT")

        tm_blocks = extract_plain_blocks(block, "NODE_TM")
        tm = tm_blocks[0] if tm_blocks else block

        pos = first_vec3(tm, "*TM_ROW3", None)
        pos_source = "TM_ROW3"

        if pos is None:
            pos = first_vec3(tm, "*TM_POS", None)
            pos_source = "TM_POS"

        row0 = first_vec3(tm, "*TM_ROW0", None)
        row1 = first_vec3(tm, "*TM_ROW1", None)
        row2 = first_vec3(tm, "*TM_ROW2", None)

        nodes.append({
            "order": order,
            "name": name,
            "parent": parent,
            "pos": pos,
            "pos_source": pos_source if pos is not None else "MISSING",
            "row0": row0,
            "row1": row1,
            "row2": row2,
        })

    return nodes


def _vsub(a, b):
    return (
        a[0] - b[0],
        a[1] - b[1],
        a[2] - b[2],
    )


def _vadd(a, b):
    return (
        a[0] + b[0],
        a[1] + b[1],
        a[2] + b[2],
    )


def _vmul(a, s):
    return (a[0] * s, a[1] * s, a[2] * s)


def _vlen(v):
    return (v[0] ** 2 + v[1] ** 2 + v[2] ** 2) ** 0.5


def _vnorm(v):
    n = _vlen(v)
    if n <= 1.0e-12:
        return None
    return (v[0] / n, v[1] / n, v[2] / n)


def _distance(a, b):
    return _vlen(_vsub(a, b))


def _preferred_child(parent_name, children):
    """
    Pick a visually sensible continuation for a Biped display bone.

    This does NOT affect the hierarchy or future Physique mapping. It only
    chooses where the Blender edit-bone tail is drawn when a node has several
    children.
    """
    low = parent_name.lower()

    preferred_terms = []

    if low == "bip01":
        preferred_terms = ["pelvis"]
    elif low.endswith("pelvis"):
        preferred_terms = ["spine"]
    elif low.endswith("spine"):
        preferred_terms = ["spine1", "neck"]
    elif low.endswith("spine1"):
        preferred_terms = ["neck"]
    elif low.endswith("neck"):
        preferred_terms = ["head"]
    elif "clavicle" in low:
        preferred_terms = ["upperarm"]
    elif "upperarm" in low:
        preferred_terms = ["forearm"]
    elif "forearm" in low:
        preferred_terms = ["hand"]
    elif "thigh" in low:
        preferred_terms = ["calf"]
    elif "calf" in low:
        preferred_terms = ["foot"]
    elif low.endswith("foot"):
        preferred_terms = ["toe"]
    elif "tail" in low:
        preferred_terms = ["tail"]

    for term in preferred_terms:
        for child in children:
            if term in child["name"].lower():
                return child

    # Prefer a child at a nonzero distance.
    valid = [
        c for c in children
        if c.get("pos") is not None
    ]

    return valid[0] if valid else None


def _fallback_tail(node, node_by_name, display_len):
    """
    End bones need nonzero length in Blender.

    Prefer continuing the direction from the parent. If that is unavailable,
    use TM_ROW0 as a display direction, then +Z.
    """
    head = node["pos"]
    parent = node_by_name.get(node.get("parent"))

    if parent and parent.get("pos") is not None:
        direction = _vnorm(_vsub(head, parent["pos"]))
        if direction is not None:
            return _vadd(head, _vmul(direction, display_len))

    row0 = node.get("row0")
    if row0 is not None:
        direction = _vnorm(row0)
        if direction is not None:
            return _vadd(head, _vmul(direction, display_len))

    return (head[0], head[1], head[2] + display_len)


def create_biped_armature(
    biped_nodes,
    collection,
    name="PT_BIPED",
):
    """
    Create a Blender Armature matching the ASE Bip* node hierarchy.

    V3 scope:
      - node position
      - node name
      - parent hierarchy
      - source matrix rows stored as metadata

    Not in V3:
      - exact bind orientation/roll
      - Physique weights
      - animation controllers
    """
    valid_nodes = [n for n in biped_nodes if n.get("pos") is not None]

    if not valid_nodes:
        return None, {
            "total_nodes": len(biped_nodes),
            "valid_nodes": 0,
            "missing_positions": len(biped_nodes),
            "bones": 0,
            "parent_links": 0,
        }

    node_by_name = {n["name"]: n for n in valid_nodes}

    children_by_name = {}
    for node in valid_nodes:
        parent = node.get("parent")
        if parent in node_by_name:
            children_by_name.setdefault(parent, []).append(node)

    # Estimate a safe display length from parent-child distances.
    distances = []
    for node in valid_nodes:
        parent = node_by_name.get(node.get("parent"))
        if parent:
            d = _distance(node["pos"], parent["pos"])
            if d > 1.0e-5:
                distances.append(d)

    if distances:
        distances.sort()
        median = distances[len(distances) // 2]
        display_len = max(median * 0.30, 0.01)
    else:
        display_len = 1.0

    arm_data = bpy.data.armatures.new(f"{name}_Data")
    arm_obj = bpy.data.objects.new(name, arm_data)
    collection.objects.link(arm_obj)

    arm_obj.show_in_front = True
    arm_obj.data.display_type = "STICK"

    bpy.context.view_layer.objects.active = arm_obj
    arm_obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")

    edit_by_name = {}

    # Create bones first.
    for node in valid_nodes:
        eb = arm_data.edit_bones.new(node["name"])
        eb.head = node["pos"]

        children = children_by_name.get(node["name"], [])
        child = _preferred_child(node["name"], children)

        tail = None
        if child and child.get("pos") is not None:
            if _distance(node["pos"], child["pos"]) > 1.0e-5:
                tail = child["pos"]

        if tail is None:
            tail = _fallback_tail(
                node,
                node_by_name,
                display_len,
            )

        # Blender rejects zero-length edit bones.
        if _distance(node["pos"], tail) <= 1.0e-5:
            tail = (
                node["pos"][0],
                node["pos"][1],
                node["pos"][2] + display_len,
            )

        eb.tail = tail
        eb.use_connect = False

        edit_by_name[node["name"]] = eb

    # Parent hierarchy is applied separately.
    parent_links = 0

    for node in valid_nodes:
        eb = edit_by_name[node["name"]]
        parent_name = node.get("parent")

        if parent_name in edit_by_name:
            eb.parent = edit_by_name[parent_name]
            eb.use_connect = False
            parent_links += 1

    bpy.ops.object.mode_set(mode="OBJECT")

    # Preserve ASE metadata on Blender Bone datablocks.
    for node in valid_nodes:
        bone = arm_data.bones.get(node["name"])
        if bone is None:
            continue

        bone["pt_ase_node_name"] = node["name"]
        bone["pt_ase_node_parent"] = node.get("parent") or ""
        bone["pt_ase_pos_source"] = node.get("pos_source") or ""

        p = node.get("pos")
        if p:
            bone["pt_ase_tm_row3"] = list(p)

        for key in ("row0", "row1", "row2"):
            value = node.get(key)
            if value is not None:
                bone[f"pt_ase_tm_{key}"] = list(value)

    missing = [n for n in biped_nodes if n.get("pos") is None]

    # Bounding box of Biped node positions for diagnostics.
    xs = [n["pos"][0] for n in valid_nodes]
    ys = [n["pos"][1] for n in valid_nodes]
    zs = [n["pos"][2] for n in valid_nodes]

    stats = {
        "total_nodes": len(biped_nodes),
        "valid_nodes": len(valid_nodes),
        "missing_positions": len(missing),
        "bones": len(valid_nodes),
        "parent_links": parent_links,
        "display_len": display_len,
        "bbox_min": (min(xs), min(ys), min(zs)),
        "bbox_max": (max(xs), max(ys), max(zs)),
        "missing_names": [n["name"] for n in missing],
    }

    return arm_obj, stats


# -----------------------------------------------------------------------------
# Main
# -----------------------------------------------------------------------------

def main():
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    parser = argparse.ArgumentParser(description="Priston Tale ASE -> Blender V3")
    parser.add_argument("--ase", required=True)
    parser.add_argument("--client-root", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--cache")
    parser.add_argument("--all-meshes", action="store_true")
    parser.add_argument("--flip-v", action="store_true")
    parser.add_argument("--no-pack", action="store_true")
    args = parser.parse_args(argv)

    ase_path = Path(args.ase).expanduser().resolve()
    client_root = Path(args.client_root).expanduser().resolve()
    out_path = Path(args.out).expanduser().resolve()
    cache_dir = (
        Path(args.cache).expanduser().resolve()
        if args.cache
        else out_path.parent / "decoded_textures_v2"
    )

    if not ase_path.is_file():
        raise SystemExit(f"ASE not found: {ase_path}")
    if not client_root.is_dir():
        raise SystemExit(f"client root not found: {client_root}")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    cache_dir.mkdir(parents=True, exist_ok=True)

    print("=" * 80)
    print("PT ASE IMPORT V3 - TEXTURED MESH + BIPED SKELETON")
    print("=" * 80)
    print("ASE        :", ase_path)
    print("CLIENT ROOT:", client_root)
    print("OUT        :", out_path)
    print("CACHE      :", cache_dir)
    print()

    text = ase_path.read_text(encoding="latin-1", errors="replace")

    geom_blocks = extract_plain_blocks(text, "GEOMOBJECT")
    materials = parse_materials(text)

    print("GEOMOBJECT blocks:", len(geom_blocks))
    print("Top materials    :", len(materials))
    for mid in sorted(materials):
        m = materials[mid]
        print(
            f"  MATERIAL {mid}: name={m['name']!r} "
            f"submaterials={len(m['submaterials'])} bitmap={m['bitmap']!r}"
        )
        for sid in sorted(m["submaterials"]):
            sm = m["submaterials"][sid]
            print(
                f"    SUB {sid}: name={sm['name']!r} bitmap={sm['bitmap']!r}"
            )

    print()
    print("Indexing client textures...")
    image_index, image_count = build_image_index(client_root)
    print("Indexed image files:", image_count)
    print("Unique basenames   :", len(image_index))
    print()

    clear_scene()
    collection = ensure_collection("PT_ASE_IMPORT_V3")

    imported = []
    skipped = 0

    for block in geom_blocks:
        info = import_geomobject(
            block,
            collection,
            materials,
            client_root,
            ase_path.parent,
            image_index,
            cache_dir,
            import_zero_uv=args.all_meshes,
            flip_v=args.flip_v,
            pack_images=not args.no_pack,
        )
        if info is None:
            skipped += 1
        else:
            imported.append(info)

    # V3: parse and create Biped armature after meshes are imported.
    biped_nodes = parse_biped_nodes(geom_blocks)
    armature_obj, skeleton_stats = create_biped_armature(
        biped_nodes,
        collection,
        name="PT_BIPED",
    )

    print()
    print("=" * 80)
    print("BIPED / SKELETON RESULT")
    print("=" * 80)
    print("Bip nodes found     :", skeleton_stats["total_nodes"])
    print("Bones created       :", skeleton_stats["bones"])
    print("Parent links        :", skeleton_stats["parent_links"])
    print("Missing positions   :", skeleton_stats["missing_positions"])
    if "display_len" in skeleton_stats:
        print("Display tail length :", skeleton_stats["display_len"])
        print("Biped bbox min      :", skeleton_stats["bbox_min"])
        print("Biped bbox max      :", skeleton_stats["bbox_max"])

    if skeleton_stats.get("missing_names"):
        print("Missing-position nodes:")
        for name in skeleton_stats["missing_names"]:
            print("  ", name)

    if biped_nodes:
        print()
        print("First Biped nodes:")
        for node in biped_nodes[:20]:
            print(
                f"  {node['name']!r} "
                f"parent={node.get('parent')!r} "
                f"pos={node.get('pos')!r} "
                f"source={node.get('pos_source')}"
            )

    print()
    print("=" * 80)
    print("IMPORT RESULT")
    print("=" * 80)
    print("Imported:", len(imported))
    print("Skipped :", skipped)
    print()

    loaded = 0
    missing = 0
    failed = 0

    for info in imported:
        print(
            f"{info['name']}: V={info['verts']} F={info['faces']} "
            f"TV={info['tverts']} TF={info['tfaces']} "
            f"UVfaces={info['uv_faces_applied']} MATREF={info['material_ref']}"
        )

        for mr in info["materials"]:
            print(
                f"  MTLID {mr['mtl_id']} -> "
                f"name={mr['material_name']!r} "
                f"bitmap={mr['bitmap_raw']!r}"
            )
            print(
                f"           resolved={mr['resolved']!r} "
                f"candidates={mr['candidate_count']}"
            )
            print(
                f"           decode={mr['decode']} load={mr['load']}"
            )

            if mr["load"] == "LOADED":
                loaded += 1
            elif mr["resolved"] is None:
                missing += 1
            else:
                failed += 1

    print()
    print("Material texture loads:")
    print("  LOADED :", loaded)
    print("  MISSING:", missing)
    print("  FAILED :", failed)

    if imported or armature_obj:
        bpy.ops.object.select_all(action="DESELECT")
        for obj in collection.objects:
            obj.select_set(True)

        if armature_obj is not None:
            bpy.context.view_layer.objects.active = armature_obj
        elif len(collection.objects):
            bpy.context.view_layer.objects.active = collection.objects[0]

    # Store source metadata at scene level.
    bpy.context.scene["pt_ase_source"] = str(ase_path)
    bpy.context.scene["pt_client_root"] = str(client_root)
    bpy.context.scene["pt_importer_version"] = "V3"
    bpy.context.scene["pt_biped_bones"] = skeleton_stats["bones"]
    bpy.context.scene["pt_biped_parent_links"] = skeleton_stats["parent_links"]

    bpy.ops.wm.save_as_mainfile(filepath=str(out_path))

    print()
    print("BLEND saved:", out_path)
    print("=" * 80)


if __name__ == "__main__":
    main()

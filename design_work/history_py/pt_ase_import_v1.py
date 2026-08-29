# pt_ase_import_v1.py
# Summit Tale / Priston Tale ASE -> Blender
#
# V1: read-only inspection importer
# Imports:
#   - GEOMOBJECT mesh vertices/faces
#   - UV (MESH_TVERT / MESH_TFACE)
#   - MESH_MTLID as placeholder Blender materials
# Skips:
#   - Bip* geometry
#   - Dummy*
#   - Armature/Physique (V2)
#   - Real material/bitmap parsing (V2)
#
# Usage:
# blender --background --python pt_ase_import_v1.py -- \
#   --ase "/path/to/file.ASE" \
#   --out "/path/to/output.blend"

import bpy
import re
import sys
import argparse
from pathlib import Path

FLOAT = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


def extract_braced_blocks(text, token):
    blocks = []
    pos = 0
    while True:
        start = text.find(token, pos)
        if start < 0:
            break
        brace = text.find("{", start + len(token))
        if brace < 0:
            break
        depth = 0
        i = brace
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
                        blocks.append(text[start:i + 1])
                        pos = i + 1
                        break
            i += 1
        else:
            raise ValueError(f"Bloco sem fechamento: {token}")
    return blocks


def get_first_quoted(block, key):
    m = re.search(re.escape(key) + r'\s+"([^"]*)"', block, flags=re.IGNORECASE)
    return m.group(1) if m else None


def parse_vertices(mesh_block):
    result = {}
    rx = re.compile(rf"\*MESH_VERTEX\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})", re.IGNORECASE)
    for m in rx.finditer(mesh_block):
        result[int(m.group(1))] = (float(m.group(2)), float(m.group(3)), float(m.group(4)))
    return result


def parse_faces(mesh_block):
    result = {}
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
        result[idx] = (tri, mtl_id)
    return result


def parse_tverts(mesh_block):
    result = {}
    rx = re.compile(rf"\*MESH_TVERT\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})", re.IGNORECASE)
    for m in rx.finditer(mesh_block):
        result[int(m.group(1))] = (float(m.group(2)), float(m.group(3)), float(m.group(4)))
    return result


def parse_tfaces(mesh_block):
    result = {}
    rx = re.compile(r"\*MESH_TFACE\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)", re.IGNORECASE)
    for m in rx.finditer(mesh_block):
        result[int(m.group(1))] = (int(m.group(2)), int(m.group(3)), int(m.group(4)))
    return result


def dense_vertex_list(vertex_map):
    if not vertex_map:
        return []
    max_idx = max(vertex_map)
    verts = [(0.0, 0.0, 0.0)] * (max_idx + 1)
    for idx, co in vertex_map.items():
        verts[idx] = co
    return verts


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def ensure_collection(name="PT_ASE_IMPORT"):
    col = bpy.data.collections.get(name)
    if col is None:
        col = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(col)
    return col


def make_placeholder_material(mtl_id):
    name = f"ASE_MTLID_{mtl_id:03d}"
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name=name)
    return mat


def import_geomobject(block, collection, import_zero_uv=False, flip_v=False):
    node_name = get_first_quoted(block, "*NODE_NAME") or "Unnamed"
    low = node_name.lower()
    if low.startswith("bip") or low.startswith("dummy"):
        return None

    mesh_blocks = extract_braced_blocks(block, "*MESH")
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

    verts = dense_vertex_list(verts_map)
    face_indices = sorted(faces_map)
    faces = [faces_map[i][0] for i in face_indices]

    max_vert = len(verts) - 1
    for face_idx, tri in zip(face_indices, faces):
        for vi in tri:
            if vi < 0 or vi > max_vert:
                raise ValueError(
                    f"{node_name}: face {face_idx} referencia vertex inválido {vi}; range 0..{max_vert}"
                )

    mesh = bpy.data.meshes.new(f"{node_name}_Mesh")
    mesh.from_pydata(verts, [], faces)
    mesh.update(calc_edges=True)

    obj = bpy.data.objects.new(node_name, mesh)
    collection.objects.link(obj)

    used_mtl_ids = sorted({faces_map[i][1] for i in face_indices})
    slot_by_id = {}
    for mtl_id in used_mtl_ids:
        mat = make_placeholder_material(mtl_id)
        slot_by_id[mtl_id] = len(mesh.materials)
        mesh.materials.append(mat)

    for poly_index, face_idx in enumerate(face_indices):
        mesh.polygons[poly_index].material_index = slot_by_id[faces_map[face_idx][1]]

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

    obj["pt_ase_node_name"] = node_name
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
        "mtl_ids": used_mtl_ids,
    }


def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] if "--" in argv else []

    parser = argparse.ArgumentParser(description="Priston Tale ASE -> Blender inspection importer V1")
    parser.add_argument("--ase", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--all-meshes", action="store_true")
    parser.add_argument("--flip-v", action="store_true")
    args = parser.parse_args(argv)

    ase_path = Path(args.ase).expanduser().resolve()
    out_path = Path(args.out).expanduser().resolve()

    if not ase_path.exists():
        raise SystemExit(f"ASE inexistente: {ase_path}")

    print("=" * 72)
    print("PT ASE IMPORT V1")
    print("=" * 72)
    print("ASE :", ase_path)
    print("OUT :", out_path)
    print()

    text = ase_path.read_text(encoding="utf-8", errors="replace")
    if "*3DSMAX_ASCIIEXPORT" not in text:
        print("AVISO: header *3DSMAX_ASCIIEXPORT não encontrado.")

    blocks = extract_braced_blocks(text, "*GEOMOBJECT")
    print("GEOMOBJECT blocks:", len(blocks))

    clear_scene()
    collection = ensure_collection()

    imported = []
    skipped = 0
    for block in blocks:
        info = import_geomobject(
            block,
            collection,
            import_zero_uv=args.all_meshes,
            flip_v=args.flip_v,
        )
        if info is None:
            skipped += 1
        else:
            imported.append(info)

    print()
    print("=" * 72)
    print("RESULTADO")
    print("=" * 72)
    print("Importados:", len(imported))
    print("Ignorados :", skipped)
    print()

    for info in imported:
        print(
            f'{info["name"]}: '
            f'V={info["verts"]} '
            f'F={info["faces"]} '
            f'TV={info["tverts"]} '
            f'TF={info["tfaces"]} '
            f'UVfaces={info["uv_faces_applied"]} '
            f'MTL={info["mtl_ids"]}'
        )

    out_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out_path))

    print()
    print("BLEND salvo:", out_path)
    print("=" * 72)


if __name__ == "__main__":
    main()

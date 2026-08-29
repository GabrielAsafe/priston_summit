#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
V5c.1 - canonical MESH rebuild gate, SAME TOPOLOGY.

This is intentionally NOT the topology-changing exporter yet.

It rebuilds, for one imported mesh object:
- MESH_NUMVERTEX
- MESH_VERTEX_LIST
- MESH_NUMFACES
- MESH_FACE_LIST
- MESH_NUMTVERTEX
- MESH_TVERTLIST
- MESH_NUMTVFACES
- MESH_TFACELIST

It preserves:
- GEOMOBJECT structure
- NODE_TM
- MATERIAL_REF
- PHYSIQUE
- Biped / animation
- every unrelated template section

Important Summit loader facts already source-proved:
- runtime face uses A/B/C + MESH_MTLID;
- AB/BC/CA and MESH_SMOOTHING are ignored;
- runtime UV uses U/V + TFACE;
- TVERT W is consumed and ignored.

V5c.1 therefore canonicalizes:
- AB=1 BC=1 CA=1
- MESH_SMOOTHING 1
- W=0.0 for referenced TVERTs

Unreferenced/orphan TVERTs are preserved from the template because Blender
has no loop corresponding to them.

V5c.1 refuses any actual topology change. That comes next.
"""

import argparse
import hashlib
import re
import sys
from pathlib import Path

import bpy


FLOAT = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


# ---------------------------------------------------------------------------
# Basic helpers
# ---------------------------------------------------------------------------

def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def format_float(v):
    if abs(v) < 0.0000005:
        v = 0.0
    s = f"{float(v):.6f}".rstrip("0").rstrip(".")
    if "." not in s and "e" not in s.lower():
        s += ".0"
    return s


def newline_style(text):
    return "\r\n" if "\r\n" in text else "\n"


def find_braced_blocks(text, token):
    rx = re.compile(
        rf"(?im)^[ \t]*\*{re.escape(token)}[ \t]*\{{"
    )

    out = []

    for m in rx.finditer(text):
        brace = text.find("{", m.start(), m.end())
        if brace < 0:
            continue

        depth = 0
        quote = False
        escape = False
        i = brace

        while i < len(text):
            ch = text[i]

            if quote:
                if escape:
                    escape = False
                elif ch == "\\":
                    escape = True
                elif ch == '"':
                    quote = False
            else:
                if ch == '"':
                    quote = True
                elif ch == "{":
                    depth += 1
                elif ch == "}":
                    depth -= 1
                    if depth == 0:
                        out.append(
                            (m.start(), i + 1, text[m.start():i + 1])
                        )
                        break

            i += 1

    return out


def first_quoted(text, key):
    m = re.search(
        re.escape(key) + r'\s+"([^"]+)"',
        text,
        flags=re.IGNORECASE,
    )
    return m.group(1) if m else None


def first_mesh_span(geom_block):
    blocks = find_braced_blocks(
        geom_block,
        "MESH",
    )
    if not blocks:
        return None
    return blocks[0]


def line_indent_for_token(text, token):
    m = re.search(
        rf"(?m)^(?P<indent>[ \t]*)\*{re.escape(token)}\b",
        text,
    )
    if not m:
        raise RuntimeError(
            f"Could not determine indent for *{token}"
        )
    return m.group("indent")


# ---------------------------------------------------------------------------
# ASE parsing
# ---------------------------------------------------------------------------

VERTEX_RE = re.compile(
    rf"\*MESH_VERTEX\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
    flags=re.IGNORECASE,
)

FACE_RE = re.compile(
    r"\*MESH_FACE\s+(\d+):"
    r".*?\bA:\s*(\d+)"
    r".*?\bB:\s*(\d+)"
    r".*?\bC:\s*(\d+)",
    flags=re.IGNORECASE,
)

MTL_RE = re.compile(
    r"\*MESH_MTLID\s+(-?\d+)",
    flags=re.IGNORECASE,
)

TVERT_RE = re.compile(
    rf"\*MESH_TVERT\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
    flags=re.IGNORECASE,
)

TFACE_RE = re.compile(
    r"\*MESH_TFACE\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)",
    flags=re.IGNORECASE,
)


def parse_vertices(mesh_block):
    out = {}
    for m in VERTEX_RE.finditer(mesh_block):
        out[int(m.group(1))] = (
            float(m.group(2)),
            float(m.group(3)),
            float(m.group(4)),
        )
    return out


def parse_faces(mesh_block):
    out = {}

    for line in mesh_block.splitlines():
        m = FACE_RE.search(line)
        if not m:
            continue

        mm = MTL_RE.search(line)

        out[int(m.group(1))] = {
            "tri": (
                int(m.group(2)),
                int(m.group(3)),
                int(m.group(4)),
            ),
            "mtlid": int(mm.group(1)) if mm else None,
            "line": line,
        }

    return out


def parse_tverts(mesh_block):
    out = {}
    for m in TVERT_RE.finditer(mesh_block):
        out[int(m.group(1))] = (
            float(m.group(2)),
            float(m.group(3)),
            float(m.group(4)),
        )
    return out


def parse_tfaces(mesh_block):
    out = {}
    for m in TFACE_RE.finditer(mesh_block):
        out[int(m.group(1))] = (
            int(m.group(2)),
            int(m.group(3)),
            int(m.group(4)),
        )
    return out


# ---------------------------------------------------------------------------
# Blender material mapping
# ---------------------------------------------------------------------------

ASE_MAT_RE = re.compile(
    r"^ASE_M(-?\d+)_S(-?\d+)_",
    flags=re.IGNORECASE,
)


def slot_to_mtl_id(obj, slot_index):
    if slot_index < 0 or slot_index >= len(obj.data.materials):
        raise RuntimeError(
            f"{obj.name}: invalid material slot {slot_index}"
        )

    mat = obj.data.materials[slot_index]

    if mat is None:
        raise RuntimeError(
            f"{obj.name}: empty material slot {slot_index}"
        )

    m = ASE_MAT_RE.match(mat.name)

    if not m:
        raise RuntimeError(
            f"{obj.name}: cannot map material {mat.name!r} "
            "to ASE MESH_MTLID"
        )

    return int(m.group(2))


# ---------------------------------------------------------------------------
# Template patching helpers
# ---------------------------------------------------------------------------

def replace_count_line(mesh_text, key, value):
    rx = re.compile(
        rf"(?m)^([ \t]*\*{re.escape(key)}[ \t]+)(-?\d+)([^\r\n]*)(\r?\n|$)"
    )

    out, count = rx.subn(
        lambda m: (
            m.group(1)
            + str(value)
            + m.group(3)
            + m.group(4)
        ),
        mesh_text,
        count=1,
    )

    if count != 1:
        raise RuntimeError(
            f"Expected exactly one *{key} line, found {count}"
        )

    return out


def replace_list_block(mesh_text, token, body_lines, nl):
    blocks = find_braced_blocks(
        mesh_text,
        token,
    )

    if len(blocks) != 1:
        raise RuntimeError(
            f"Expected exactly one *{token} block, found {len(blocks)}"
        )

    start, end, old_block = blocks[0]
    indent = line_indent_for_token(
        old_block,
        token,
    )
    child = indent + "\t"

    new_block = (
        indent
        + "*"
        + token
        + " {"
        + nl
        + "".join(
            child + line + nl
            for line in body_lines
        )
        + indent
        + "}"
    )

    return (
        mesh_text[:start]
        + new_block
        + mesh_text[end:]
    )


# ---------------------------------------------------------------------------
# Validation / extraction
# ---------------------------------------------------------------------------

def get_blender_object(node_name):
    direct = bpy.data.objects.get(node_name)
    if direct is not None and direct.type == "MESH":
        return direct

    for obj in bpy.data.objects:
        if obj.type != "MESH":
            continue
        if str(obj.get("pt_ase_node_name", "")) == node_name:
            return obj

    raise RuntimeError(
        f"Blender mesh for ASE node {node_name!r} not found"
    )


def require_same_topology(obj, original_vertices, original_faces):
    if len(obj.data.vertices) != len(original_vertices):
        raise RuntimeError(
            f"{obj.name}: vertex count changed; "
            "V5c.1 is SAME-TOPOLOGY only"
        )

    if len(obj.data.polygons) != len(original_faces):
        raise RuntimeError(
            f"{obj.name}: face count changed; "
            "V5c.1 is SAME-TOPOLOGY only"
        )

    for i, poly in enumerate(obj.data.polygons):
        if len(poly.vertices) != 3:
            raise RuntimeError(
                f"{obj.name}: polygon {i} is not triangle"
            )

        expected = original_faces[i]["tri"]
        got = tuple(int(v) for v in poly.vertices)

        if got != expected:
            raise RuntimeError(
                f"{obj.name}: topology/order differs at face {i}: "
                f"Blender={got} ASE={expected}"
            )


def collect_tvert_uvs(obj, original_tverts, original_tfaces, eps=1e-5):
    uv_layer = (
        obj.data.uv_layers.get("ASE_UV")
        or obj.data.uv_layers.active
    )

    if uv_layer is None:
        raise RuntimeError(
            f"{obj.name}: no UV layer"
        )

    refs = {}

    for face_index, tf in sorted(original_tfaces.items()):
        poly = obj.data.polygons[face_index]

        if len(poly.loop_indices) != 3:
            raise RuntimeError(
                f"{obj.name}: face {face_index} not triangle"
            )

        for corner, tvert_index in enumerate(tf):
            loop_index = poly.loop_indices[corner]
            uv = uv_layer.data[loop_index].uv

            refs.setdefault(
                tvert_index,
                [],
            ).append(
                (float(uv.x), float(uv.y))
            )

    result = {}
    orphan = []

    for tvert_index, old in sorted(original_tverts.items()):
        values = refs.get(
            tvert_index,
            [],
        )

        if not values:
            # Preserve orphan TVERT exactly.
            result[tvert_index] = old
            orphan.append(tvert_index)
            continue

        rep = values[0]

        for other in values[1:]:
            if (
                abs(rep[0] - other[0]) > eps
                or abs(rep[1] - other[1]) > eps
            ):
                raise RuntimeError(
                    f"{obj.name}: TVERT {tvert_index} maps to "
                    "multiple current Blender UV values; this would "
                    "require a TVERT split and belongs to later V5c."
                )

        # Canonical W=0.0 for referenced TVERTs.
        result[tvert_index] = (
            rep[0],
            rep[1],
            0.0,
        )

    return result, orphan


# ---------------------------------------------------------------------------
# Main rebuild
# ---------------------------------------------------------------------------

def parse_args():
    argv = sys.argv

    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    p = argparse.ArgumentParser()
    p.add_argument("--template", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--object", default="MahC20")
    return p.parse_args(argv)


def main():
    args = parse_args()

    template_path = Path(
        args.template
    ).expanduser().resolve()

    out_path = Path(
        args.out
    ).expanduser().resolve()

    raw = template_path.read_bytes()
    text = raw.decode("latin-1")
    nl = newline_style(text)

    geom_hits = []

    for start, end, block in find_braced_blocks(
        text,
        "GEOMOBJECT",
    ):
        if first_quoted(block, "*NODE_NAME") == args.object:
            geom_hits.append(
                (start, end, block)
            )

    if len(geom_hits) != 1:
        raise RuntimeError(
            f"Expected one GEOMOBJECT {args.object!r}, "
            f"found {len(geom_hits)}"
        )

    geom_start, geom_end, geom_block = geom_hits[0]

    mesh_span = first_mesh_span(
        geom_block
    )

    if mesh_span is None:
        raise RuntimeError(
            f"{args.object}: MESH block not found"
        )

    mesh_start, mesh_end, mesh_block = mesh_span

    original_vertices = parse_vertices(
        mesh_block
    )
    original_faces = parse_faces(
        mesh_block
    )
    original_tverts = parse_tverts(
        mesh_block
    )
    original_tfaces = parse_tfaces(
        mesh_block
    )

    obj = get_blender_object(
        args.object
    )
    obj.data.update()

    require_same_topology(
        obj,
        original_vertices,
        original_faces,
    )

    rebuilt_tverts, orphan_tverts = collect_tvert_uvs(
        obj,
        original_tverts,
        original_tfaces,
    )

    # ------------------------------------------------------------------
    # Build canonical list contents
    # ------------------------------------------------------------------

    vertex_lines = []

    for v in obj.data.vertices:
        vertex_lines.append(
            "*MESH_VERTEX "
            + str(v.index)
            + "\t"
            + format_float(v.co.x)
            + "\t"
            + format_float(v.co.y)
            + "\t"
            + format_float(v.co.z)
        )

    face_lines = []

    for poly in obj.data.polygons:
        idx = int(poly.index)
        a, b, c = (
            int(poly.vertices[0]),
            int(poly.vertices[1]),
            int(poly.vertices[2]),
        )

        mtlid = slot_to_mtl_id(
            obj,
            int(poly.material_index),
        )

        face_lines.append(
            f"*MESH_FACE {idx}: "
            f"A: {a} B: {b} C: {c} "
            f"AB: 1 BC: 1 CA: 1\t "
            f"*MESH_SMOOTHING 1 \t"
            f"*MESH_MTLID {mtlid}"
        )

    tvert_lines = []

    for idx in sorted(rebuilt_tverts):
        u, v, w = rebuilt_tverts[idx]

        tvert_lines.append(
            "*MESH_TVERT "
            + str(idx)
            + "\t"
            + format_float(u)
            + "\t"
            + format_float(v)
            + "\t"
            + format_float(w)
        )

    tface_lines = []

    for idx in sorted(original_tfaces):
        a, b, c = original_tfaces[idx]

        tface_lines.append(
            f"*MESH_TFACE {idx}\t{a}\t{b}\t{c}"
        )

    # ------------------------------------------------------------------
    # Replace only the MESH list/count data.
    # ------------------------------------------------------------------

    new_mesh = mesh_block

    new_mesh = replace_count_line(
        new_mesh,
        "MESH_NUMVERTEX",
        len(vertex_lines),
    )
    new_mesh = replace_count_line(
        new_mesh,
        "MESH_NUMFACES",
        len(face_lines),
    )
    new_mesh = replace_count_line(
        new_mesh,
        "MESH_NUMTVERTEX",
        len(tvert_lines),
    )
    new_mesh = replace_count_line(
        new_mesh,
        "MESH_NUMTVFACES",
        len(tface_lines),
    )

    new_mesh = replace_list_block(
        new_mesh,
        "MESH_VERTEX_LIST",
        vertex_lines,
        nl,
    )
    new_mesh = replace_list_block(
        new_mesh,
        "MESH_FACE_LIST",
        face_lines,
        nl,
    )
    new_mesh = replace_list_block(
        new_mesh,
        "MESH_TVERTLIST",
        tvert_lines,
        nl,
    )
    new_mesh = replace_list_block(
        new_mesh,
        "MESH_TFACELIST",
        tface_lines,
        nl,
    )

    new_geom = (
        geom_block[:mesh_start]
        + new_mesh
        + geom_block[mesh_end:]
    )

    out_text = (
        text[:geom_start]
        + new_geom
        + text[geom_end:]
    )

    out_bytes = out_text.encode(
        "latin-1"
    )

    out_path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )
    out_path.write_bytes(
        out_bytes
    )

    # ------------------------------------------------------------------
    # Independent semantic self-check from serialized output.
    # ------------------------------------------------------------------

    out_geom_hits = []

    for start, end, block in find_braced_blocks(
        out_text,
        "GEOMOBJECT",
    ):
        if first_quoted(block, "*NODE_NAME") == args.object:
            out_geom_hits.append(block)

    if len(out_geom_hits) != 1:
        raise RuntimeError(
            "Serialized output lost target GEOMOBJECT"
        )

    out_mesh_span = first_mesh_span(
        out_geom_hits[0]
    )

    if out_mesh_span is None:
        raise RuntimeError(
            "Serialized output lost MESH block"
        )

    parsed_out_mesh = out_mesh_span[2]

    check_vertices = parse_vertices(
        parsed_out_mesh
    )
    check_faces = parse_faces(
        parsed_out_mesh
    )
    check_tverts = parse_tverts(
        parsed_out_mesh
    )
    check_tfaces = parse_tfaces(
        parsed_out_mesh
    )

    if len(check_vertices) != len(obj.data.vertices):
        raise RuntimeError(
            "Self-check vertex count mismatch"
        )

    if len(check_faces) != len(obj.data.polygons):
        raise RuntimeError(
            "Self-check face count mismatch"
        )

    if len(check_tverts) != len(original_tverts):
        raise RuntimeError(
            "Self-check TVERT count mismatch"
        )

    if check_tfaces != original_tfaces:
        raise RuntimeError(
            "Self-check TFACE mapping mismatch"
        )

    for poly in obj.data.polygons:
        idx = int(poly.index)
        expected_tri = tuple(
            int(x)
            for x in poly.vertices
        )

        got = check_faces[idx]

        if got["tri"] != expected_tri:
            raise RuntimeError(
                f"Self-check face topology mismatch at {idx}"
            )

        expected_mtlid = slot_to_mtl_id(
            obj,
            int(poly.material_index),
        )

        if got["mtlid"] != expected_mtlid:
            raise RuntimeError(
                f"Self-check MTLID mismatch at face {idx}"
            )

    print("=" * 80)
    print("PT BLENDER -> ASE V5c.1")
    print("CANONICAL FULL-MESH REBUILD / SAME TOPOLOGY")
    print("=" * 80)
    print("Template :", template_path)
    print("Output   :", out_path)
    print("Object   :", args.object)
    print()
    print("Template bytes :", len(raw))
    print("Template SHA256:", sha256_bytes(raw))
    print("Output bytes   :", len(out_bytes))
    print("Output SHA256  :", sha256_bytes(out_bytes))
    print()
    print("Vertices       :", len(vertex_lines))
    print("Faces          :", len(face_lines))
    print("TVERT          :", len(tvert_lines))
    print("TFACE          :", len(tface_lines))
    print("Orphan TVERTs  :", len(orphan_tverts), orphan_tverts[:20])
    print()
    print("Canonicalized  :")
    print("  AB/BC/CA        = 1/1/1")
    print("  MESH_SMOOTHING  = 1")
    print("  referenced W    = 0.0")
    print("  orphan TVERT    = preserved")
    print()
    print("PHYSIQUE        : preserved from template")
    print("MATERIAL_REF    : preserved from template")
    print("NODE_TM         : preserved from template")
    print()
    print("SELF-CHECK:")
    print("  serialized vertex count : OK")
    print("  serialized face topology: OK")
    print("  serialized MTLID        : OK")
    print("  serialized TVERT count  : OK")
    print("  serialized TFACE mapping: OK")
    print("=" * 80)


if __name__ == "__main__":
    main()

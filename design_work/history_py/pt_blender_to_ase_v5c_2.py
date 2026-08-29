#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
Blender -> ASE V5c.2
CONTROLLED FIRST TOPOLOGY CHANGE

Supported operation only:
    SPLIT_ONE_FACE_1_TO_3

Expected controlled edit:
    old:
        V = N
        F = M
    new:
        V = N + 1
        F = M + 2

One original triangle (A,B,C) becomes:
    (A,B,D)
    (B,C,D)
    (C,A,D)

where D is the single appended vertex.

UV:
    one appended TVERT at the face UV centroid;
    target TFACE becomes three TFACE entries.

Physique:
    all original rigid assignments must remain unchanged;
    one new rigid assignment is appended for D.

Summit loader facts already source-proved for this build:
- face runtime data uses A/B/C + MESH_MTLID;
- AB/BC/CA are ignored;
- MESH_SMOOTHING is ignored;
- TVERT uses U/V; W is consumed and ignored.

Canonical new/rebuilt values:
- AB=1 BC=1 CA=1
- MESH_SMOOTHING 1
- TVERT W=0.0

This exporter refuses any topology-dependent ASE block that has not yet
been handled explicitly (color vertices/faces, explicit normals, etc.).
"""

import argparse
import hashlib
import re
import sys
from pathlib import Path

import bpy


FLOAT = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


# =============================================================================
# Generic helpers
# =============================================================================

def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def fmt(v):
    v = float(v)
    if abs(v) < 0.0000005:
        v = 0.0
    s = f"{v:.6f}".rstrip("0").rstrip(".")
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


def first_int(text, key, default=None):
    m = re.search(
        re.escape(key) + r"\s+(-?\d+)",
        text,
        flags=re.IGNORECASE,
    )
    return int(m.group(1)) if m else default


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


# =============================================================================
# ASE parsing
# =============================================================================

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

PHYSIQUE_RIGID_RE = re.compile(
    r'\*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE'
    r'\s+(\d+)\s+"([^"]+)"',
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

        if mm is None:
            raise RuntimeError(
                f"Face {m.group(1)} has no MESH_MTLID"
            )

        out[int(m.group(1))] = {
            "tri": (
                int(m.group(2)),
                int(m.group(3)),
                int(m.group(4)),
            ),
            "mtlid": int(mm.group(1)),
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


def parse_physique(geom_block):
    blocks = find_braced_blocks(
        geom_block,
        "PHYSIQUE",
    )

    if len(blocks) != 1:
        raise RuntimeError(
            f"Expected exactly one PHYSIQUE block, found {len(blocks)}"
        )

    start, end, block = blocks[0]

    declared = first_int(
        block,
        "*PHYSIQUE_NUMVERTEXASSIGNMENT",
        None,
    )

    assignments = [
        (int(m.group(1)), m.group(2))
        for m in PHYSIQUE_RIGID_RE.finditer(block)
    ]

    return {
        "start": start,
        "end": end,
        "block": block,
        "declared": declared,
        "assignments": assignments,
    }


# =============================================================================
# Safety gates
# =============================================================================

TOPOLOGY_DEPENDENT_UNSUPPORTED = (
    "*MESH_NUMCVERTEX",
    "*MESH_CVERTLIST",
    "*MESH_NUMCVFACES",
    "*MESH_CFACELIST",
    "*MESH_NORMALS",
    "*MESH_FACENORMAL",
    "*MESH_VERTEXNORMAL",
)


def reject_unhandled_topology_blocks(mesh_block):
    found = []

    upper = mesh_block.upper()

    for token in TOPOLOGY_DEPENDENT_UNSUPPORTED:
        if token.upper() in upper:
            found.append(token)

    if found:
        raise RuntimeError(
            "V5c.2 safety stop: target MESH contains topology-dependent "
            "blocks not handled by this controlled exporter: "
            + ", ".join(found)
        )


# =============================================================================
# Blender helpers
# =============================================================================

ASE_MAT_RE = re.compile(
    r"^ASE_M(-?\d+)_S(-?\d+)_",
    flags=re.IGNORECASE,
)


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


def rigid_bone(obj, vertex_index, eps=1e-6):
    v = obj.data.vertices[vertex_index]
    active = []

    for membership in v.groups:
        group = obj.vertex_groups[membership.group]

        if not group.name.lower().startswith("bip"):
            continue

        if membership.weight <= eps:
            continue

        active.append(
            (group.name, float(membership.weight))
        )

    if len(active) != 1:
        raise RuntimeError(
            f"{obj.name}: vertex {vertex_index} must have exactly one "
            f"active Bip* group; got {active}"
        )

    name, weight = active[0]

    if abs(weight - 1.0) > 1e-5:
        raise RuntimeError(
            f"{obj.name}: vertex {vertex_index} is not rigid: "
            f"{name} weight={weight}"
        )

    return name


# =============================================================================
# Template replacement
# =============================================================================

def replace_count_line(mesh_text, key, value):
    rx = re.compile(
        rf"(?m)^([ \t]*\*{re.escape(key)}[ \t]+)"
        rf"(-?\d+)([^\r\n]*)(\r?\n|$)"
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


def append_one_physique_assignment(
    geom_block,
    physique,
    old_vertex_count,
    new_vertex_index,
    new_bone,
    nl,
):
    if physique["declared"] != old_vertex_count:
        raise RuntimeError(
            f"PHYSIQUE declared={physique['declared']} "
            f"but original vertices={old_vertex_count}"
        )

    assignments = physique["assignments"]

    if len(assignments) != old_vertex_count:
        raise RuntimeError(
            f"PHYSIQUE parsed={len(assignments)} "
            f"but original vertices={old_vertex_count}"
        )

    old_indices = [i for i, _ in assignments]

    if old_indices != list(range(old_vertex_count)):
        raise RuntimeError(
            "PHYSIQUE assignments are not dense 0..N-1"
        )

    block = physique["block"]

    count_rx = re.compile(
        r"(?m)^([ \t]*\*PHYSIQUE_NUMVERTEXASSIGNMENT[ \t]+)"
        r"(-?\d+)([^\r\n]*)(\r?\n|$)",
        flags=re.IGNORECASE,
    )

    block2, count = count_rx.subn(
        lambda m: (
            m.group(1)
            + str(old_vertex_count + 1)
            + m.group(3)
            + m.group(4)
        ),
        block,
        count=1,
    )

    if count != 1:
        raise RuntimeError(
            "Could not patch PHYSIQUE_NUMVERTEXASSIGNMENT"
        )

    assignment_matches = list(
        re.finditer(
            r'(?m)^(?P<indent>[ \t]*)'
            r'\*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE'
            r'[^\r\n]*(?:\r?\n|$)',
            block2,
            flags=re.IGNORECASE,
        )
    )

    if not assignment_matches:
        raise RuntimeError(
            "No PHYSIQUE assignment line found for indentation"
        )

    indent = assignment_matches[0].group("indent")

    # Insert immediately before the PHYSIQUE block closing brace.
    close = block2.rfind("}")
    if close < 0:
        raise RuntimeError(
            "PHYSIQUE closing brace not found"
        )

    before = block2[:close]
    after = block2[close:]

    if before and not before.endswith(("\n", "\r")):
        before += nl

    new_line = (
        indent
        + "*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE"
        + "\t"
        + str(new_vertex_index)
        + "\t\""
        + new_bone
        + "\""
        + nl
    )

    new_block = before + new_line + after

    return (
        geom_block[:physique["start"]]
        + new_block
        + geom_block[physique["end"]:]
    )


# =============================================================================
# Controlled topology validation
# =============================================================================

def uv_close(a, b, eps=1e-5):
    return (
        abs(float(a[0]) - float(b[0])) <= eps
        and abs(float(a[1]) - float(b[1])) <= eps
    )


def coord_close(a, b, eps=1e-5):
    return (
        abs(float(a[0]) - float(b[0])) <= eps
        and abs(float(a[1]) - float(b[1])) <= eps
        and abs(float(a[2]) - float(b[2])) <= eps
    )


def validate_controlled_split(
    obj,
    original_vertices,
    original_faces,
    original_tverts,
    original_tfaces,
):
    operation = str(
        obj.get("pt_v5c2_operation", "")
    )

    if operation != "SPLIT_ONE_FACE_1_TO_3":
        raise RuntimeError(
            f"{obj.name}: expected pt_v5c2_operation="
            f"SPLIT_ONE_FACE_1_TO_3, got {operation!r}"
        )

    target = int(
        obj.get("pt_v5c2_original_face", -1)
    )
    new_vertex = int(
        obj.get("pt_v5c2_new_vertex", -1)
    )
    common_bone_meta = str(
        obj.get("pt_v5c2_common_bone", "")
    )
    old_vertex_count_meta = int(
        obj.get("pt_v5c2_old_vertex_count", -1)
    )
    old_face_count_meta = int(
        obj.get("pt_v5c2_old_face_count", -1)
    )

    old_v = len(original_vertices)
    old_f = len(original_faces)
    old_tv = len(original_tverts)
    old_tf = len(original_tfaces)

    if old_vertex_count_meta != old_v:
        raise RuntimeError(
            f"Metadata old vertex count={old_vertex_count_meta}, "
            f"template={old_v}"
        )

    if old_face_count_meta != old_f:
        raise RuntimeError(
            f"Metadata old face count={old_face_count_meta}, "
            f"template={old_f}"
        )

    if new_vertex != old_v:
        raise RuntimeError(
            f"Expected appended vertex index {old_v}, "
            f"metadata says {new_vertex}"
        )

    if len(obj.data.vertices) != old_v + 1:
        raise RuntimeError(
            f"Expected {old_v + 1} Blender vertices, "
            f"got {len(obj.data.vertices)}"
        )

    if len(obj.data.polygons) != old_f + 2:
        raise RuntimeError(
            f"Expected {old_f + 2} Blender faces, "
            f"got {len(obj.data.polygons)}"
        )

    if target not in original_faces:
        raise RuntimeError(
            f"Original target face {target} not in template"
        )

    if set(original_faces) != set(range(old_f)):
        raise RuntimeError(
            "Template face indices are not dense 0..F-1"
        )

    if set(original_vertices) != set(range(old_v)):
        raise RuntimeError(
            "Template vertex indices are not dense 0..V-1"
        )

    if set(original_tfaces) != set(range(old_tf)):
        raise RuntimeError(
            "Template TFACE indices are not dense 0..TF-1"
        )

    if set(original_tverts) != set(range(old_tv)):
        raise RuntimeError(
            "Template TVERT indices are not dense 0..TV-1"
        )

    # All old coordinates must remain unchanged.
    for i in range(old_v):
        current = tuple(
            float(x)
            for x in obj.data.vertices[i].co
        )

        if not coord_close(
            current,
            original_vertices[i],
        ):
            raise RuntimeError(
                f"Old vertex {i} coordinate changed unexpectedly: "
                f"Blender={current} ASE={original_vertices[i]}"
            )

    a, b, c = original_faces[target]["tri"]

    expected_center = tuple(
        (
            original_vertices[a][axis]
            + original_vertices[b][axis]
            + original_vertices[c][axis]
        ) / 3.0
        for axis in range(3)
    )

    current_center = tuple(
        float(x)
        for x in obj.data.vertices[new_vertex].co
    )

    if not coord_close(
        current_center,
        expected_center,
    ):
        raise RuntimeError(
            f"New vertex is not target-face centroid: "
            f"Blender={current_center} expected={expected_center}"
        )

    expected_split = [
        (a, b, new_vertex),
        (b, c, new_vertex),
        (c, a, new_vertex),
    ]

    # Validate exact polygon ordering:
    # before target => same,
    # target..target+2 => split,
    # after => original shifted by +2.
    for new_face_index, poly in enumerate(obj.data.polygons):
        if len(poly.vertices) != 3:
            raise RuntimeError(
                f"Polygon {new_face_index} is not a triangle"
            )

        got = tuple(
            int(x)
            for x in poly.vertices
        )

        if new_face_index < target:
            expected = original_faces[new_face_index]["tri"]

        elif target <= new_face_index <= target + 2:
            expected = expected_split[
                new_face_index - target
            ]

        else:
            old_face_index = new_face_index - 2
            expected = original_faces[
                old_face_index
            ]["tri"]

        if got != expected:
            raise RuntimeError(
                f"Unexpected topology at new face {new_face_index}: "
                f"Blender={got} expected={expected}"
            )

    # Material IDs must be unchanged/inherited.
    for new_face_index, poly in enumerate(obj.data.polygons):
        got_mtl = slot_to_mtl_id(
            obj,
            int(poly.material_index),
        )

        if new_face_index < target:
            old_face_index = new_face_index
        elif target <= new_face_index <= target + 2:
            old_face_index = target
        else:
            old_face_index = new_face_index - 2

        expected_mtl = original_faces[
            old_face_index
        ]["mtlid"]

        if got_mtl != expected_mtl:
            raise RuntimeError(
                f"Material changed unexpectedly at face "
                f"{new_face_index}: Blender={got_mtl} "
                f"expected={expected_mtl}"
            )

    # Original Physique bone mapping must remain exact.
    physique = parse_physique_from_assignments_only = None

    # Bone of target triangle must be the same and rigid.
    target_bones = [
        rigid_bone(obj, idx)
        for idx in (a, b, c)
    ]

    if not (
        target_bones[0]
        == target_bones[1]
        == target_bones[2]
    ):
        raise RuntimeError(
            f"Target vertices no longer share one rigid bone: "
            f"{target_bones}"
        )

    common_bone = target_bones[0]

    if common_bone != common_bone_meta:
        raise RuntimeError(
            f"Common bone metadata mismatch: "
            f"Blender={common_bone!r} metadata={common_bone_meta!r}"
        )

    new_bone = rigid_bone(
        obj,
        new_vertex,
    )

    if new_bone != common_bone:
        raise RuntimeError(
            f"New vertex bone {new_bone!r} != "
            f"common bone {common_bone!r}"
        )

    # UV mapping.
    uv_layer = (
        obj.data.uv_layers.get("ASE_UV")
        or obj.data.uv_layers.active
    )

    if uv_layer is None:
        raise RuntimeError(
            f"{obj.name}: no UV layer"
        )

    target_tf = original_tfaces[target]
    ta, tb, tc = target_tf

    center_uv_expected = (
        (
            original_tverts[ta][0]
            + original_tverts[tb][0]
            + original_tverts[tc][0]
        ) / 3.0,
        (
            original_tverts[ta][1]
            + original_tverts[tb][1]
            + original_tverts[tc][1]
        ) / 3.0,
    )

    # Validate unchanged and split-face UVs.
    for new_face_index, poly in enumerate(obj.data.polygons):
        current_uvs = []

        for li in poly.loop_indices:
            uv = uv_layer.data[li].uv
            current_uvs.append(
                (float(uv.x), float(uv.y))
            )

        if new_face_index < target:
            old_face_index = new_face_index
            tf = original_tfaces[old_face_index]
            expected_uvs = [
                original_tverts[t][:2]
                for t in tf
            ]

        elif target <= new_face_index <= target + 2:
            k = new_face_index - target

            if k == 0:
                expected_uvs = [
                    original_tverts[ta][:2],
                    original_tverts[tb][:2],
                    center_uv_expected,
                ]
            elif k == 1:
                expected_uvs = [
                    original_tverts[tb][:2],
                    original_tverts[tc][:2],
                    center_uv_expected,
                ]
            else:
                expected_uvs = [
                    original_tverts[tc][:2],
                    original_tverts[ta][:2],
                    center_uv_expected,
                ]

        else:
            old_face_index = new_face_index - 2
            tf = original_tfaces[old_face_index]
            expected_uvs = [
                original_tverts[t][:2]
                for t in tf
            ]

        for corner, (got_uv, expected_uv) in enumerate(
            zip(current_uvs, expected_uvs)
        ):
            if not uv_close(
                got_uv,
                expected_uv,
            ):
                raise RuntimeError(
                    f"UV mismatch face {new_face_index} "
                    f"corner {corner}: Blender={got_uv} "
                    f"expected={expected_uv}"
                )

    return {
        "target": target,
        "new_vertex": new_vertex,
        "common_bone": common_bone,
        "expected_center": expected_center,
        "center_uv": center_uv_expected,
        "old_v": old_v,
        "old_f": old_f,
        "old_tv": old_tv,
        "old_tf": old_tf,
        "target_tri": (a, b, c),
        "target_tface": (ta, tb, tc),
    }


# =============================================================================
# Main
# =============================================================================

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

    reject_unhandled_topology_blocks(
        mesh_block
    )

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

    physique = parse_physique(
        geom_block
    )

    obj = get_blender_object(
        args.object
    )
    obj.data.update()

    info = validate_controlled_split(
        obj,
        original_vertices,
        original_faces,
        original_tverts,
        original_tfaces,
    )

    old_v = info["old_v"]
    old_f = info["old_f"]
    old_tv = info["old_tv"]
    old_tf = info["old_tf"]
    target = info["target"]
    new_vertex = info["new_vertex"]
    common_bone = info["common_bone"]
    ta, tb, tc = info["target_tface"]

    # Validate template Physique against current old vertices.
    if physique["declared"] != old_v:
        raise RuntimeError(
            f"Template PHYSIQUE declared={physique['declared']} "
            f"expected={old_v}"
        )

    if len(physique["assignments"]) != old_v:
        raise RuntimeError(
            f"Template PHYSIQUE parsed={len(physique['assignments'])} "
            f"expected={old_v}"
        )

    for vertex_index, template_bone in physique["assignments"]:
        current_bone = rigid_bone(
            obj,
            vertex_index,
        )

        if current_bone != template_bone:
            raise RuntimeError(
                f"Old PHYSIQUE changed at vertex {vertex_index}: "
                f"Blender={current_bone!r} "
                f"template={template_bone!r}"
            )

    # -------------------------------------------------------------------------
    # Build mesh data.
    # -------------------------------------------------------------------------

    vertex_lines = []

    for v in obj.data.vertices:
        vertex_lines.append(
            "*MESH_VERTEX "
            + str(v.index)
            + "\t"
            + fmt(v.co.x)
            + "\t"
            + fmt(v.co.y)
            + "\t"
            + fmt(v.co.z)
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

    # Existing TVERT indices remain semantically the same.
    # Canonicalize W to 0.0 because Summit loader discards it.
    tvert_lines = []

    for idx in range(old_tv):
        u, v, _w = original_tverts[idx]

        tvert_lines.append(
            f"*MESH_TVERT {idx}\t"
            f"{fmt(u)}\t{fmt(v)}\t0.0"
        )

    new_tvert = old_tv
    center_u, center_v = info["center_uv"]

    tvert_lines.append(
        f"*MESH_TVERT {new_tvert}\t"
        f"{fmt(center_u)}\t{fmt(center_v)}\t0.0"
    )

    tface_lines = []

    for new_face_index in range(old_f + 2):
        if new_face_index < target:
            tf = original_tfaces[
                new_face_index
            ]

        elif new_face_index == target:
            tf = (
                ta,
                tb,
                new_tvert,
            )

        elif new_face_index == target + 1:
            tf = (
                tb,
                tc,
                new_tvert,
            )

        elif new_face_index == target + 2:
            tf = (
                tc,
                ta,
                new_tvert,
            )

        else:
            old_face_index = new_face_index - 2
            tf = original_tfaces[
                old_face_index
            ]

        tface_lines.append(
            f"*MESH_TFACE {new_face_index}\t"
            f"{tf[0]}\t{tf[1]}\t{tf[2]}"
        )

    # -------------------------------------------------------------------------
    # Replace target MESH lists/counts.
    # -------------------------------------------------------------------------

    new_mesh = mesh_block

    new_mesh = replace_count_line(
        new_mesh,
        "MESH_NUMVERTEX",
        old_v + 1,
    )
    new_mesh = replace_count_line(
        new_mesh,
        "MESH_NUMFACES",
        old_f + 2,
    )
    new_mesh = replace_count_line(
        new_mesh,
        "MESH_NUMTVERTEX",
        old_tv + 1,
    )
    new_mesh = replace_count_line(
        new_mesh,
        "MESH_NUMTVFACES",
        old_tf + 2,
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

    # Re-parse Physique span inside new_geom because changing MESH text length
    # shifts local offsets.
    new_physique = parse_physique(
        new_geom
    )

    new_geom = append_one_physique_assignment(
        new_geom,
        new_physique,
        old_v,
        new_vertex,
        common_bone,
        nl,
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

    # -------------------------------------------------------------------------
    # Self-check serialized output.
    # -------------------------------------------------------------------------

    out_geom_hits = []

    for _s, _e, block in find_braced_blocks(
        out_text,
        "GEOMOBJECT",
    ):
        if first_quoted(block, "*NODE_NAME") == args.object:
            out_geom_hits.append(block)

    if len(out_geom_hits) != 1:
        raise RuntimeError(
            "Serialized output lost target GEOMOBJECT"
        )

    out_geom = out_geom_hits[0]

    out_mesh_span = first_mesh_span(
        out_geom
    )

    if out_mesh_span is None:
        raise RuntimeError(
            "Serialized output lost MESH"
        )

    out_mesh = out_mesh_span[2]

    check_v = parse_vertices(
        out_mesh
    )
    check_f = parse_faces(
        out_mesh
    )
    check_tv = parse_tverts(
        out_mesh
    )
    check_tf = parse_tfaces(
        out_mesh
    )
    check_phys = parse_physique(
        out_geom
    )

    expected = {
        "V": old_v + 1,
        "F": old_f + 2,
        "TV": old_tv + 1,
        "TF": old_tf + 2,
        "PHYS": old_v + 1,
    }

    got = {
        "V": len(check_v),
        "F": len(check_f),
        "TV": len(check_tv),
        "TF": len(check_tf),
        "PHYS": len(check_phys["assignments"]),
    }

    if got != expected:
        raise RuntimeError(
            f"Serialized count mismatch: got={got} expected={expected}"
        )

    if check_phys["declared"] != old_v + 1:
        raise RuntimeError(
            f"Serialized PHYSIQUE declared="
            f"{check_phys['declared']} expected={old_v + 1}"
        )

    # New vertex coordinate.
    if not coord_close(
        check_v[new_vertex],
        info["expected_center"],
    ):
        raise RuntimeError(
            "Serialized new vertex coordinate mismatch"
        )

    # Exact split topology.
    a, b, c = info["target_tri"]
    expected_split = [
        (a, b, new_vertex),
        (b, c, new_vertex),
        (c, a, new_vertex),
    ]

    for k in range(3):
        if check_f[target + k]["tri"] != expected_split[k]:
            raise RuntimeError(
                f"Serialized split face {target + k} mismatch"
            )

    # Exact new TFACE mapping.
    expected_split_tf = [
        (ta, tb, new_tvert),
        (tb, tc, new_tvert),
        (tc, ta, new_tvert),
    ]

    for k in range(3):
        if check_tf[target + k] != expected_split_tf[k]:
            raise RuntimeError(
                f"Serialized split TFACE {target + k} mismatch"
            )

    # New Physique assignment.
    phys_by_vertex = dict(
        check_phys["assignments"]
    )

    if phys_by_vertex.get(new_vertex) != common_bone:
        raise RuntimeError(
            f"Serialized new PHYSIQUE mismatch: "
            f"{phys_by_vertex.get(new_vertex)!r} != {common_bone!r}"
        )

    print("=" * 80)
    print("PT BLENDER -> ASE V5c.2")
    print("CONTROLLED TOPOLOGY EXPORT: 1 FACE -> 3")
    print("=" * 80)
    print("Template :", template_path)
    print("Output   :", out_path)
    print("Object   :", args.object)
    print()
    print("Operation         : SPLIT_ONE_FACE_1_TO_3")
    print("Original face     :", target)
    print("Original triangle :", info["target_tri"])
    print("New vertex        :", new_vertex)
    print("New vertex coord  :", info["expected_center"])
    print("Common bone       :", common_bone)
    print("New TVERT         :", new_tvert)
    print("Center UV         :", info["center_uv"])
    print()
    print("COUNTS:")
    print(f"  Vertices : {old_v} -> {old_v + 1}")
    print(f"  Faces    : {old_f} -> {old_f + 2}")
    print(f"  TVERT    : {old_tv} -> {old_tv + 1}")
    print(f"  TFACE    : {old_tf} -> {old_tf + 2}")
    print(f"  PHYSIQUE : {old_v} -> {old_v + 1}")
    print()
    print("REPLACEMENT FACES:")
    for k, tri in enumerate(expected_split):
        print(f"  {target + k}: {tri}")
    print()
    print("REPLACEMENT TFACES:")
    for k, tf in enumerate(expected_split_tf):
        print(f"  {target + k}: {tf}")
    print()
    print("SELF-CHECK:")
    print("  serialized counts       : OK")
    print("  new vertex coordinate   : OK")
    print("  split face topology     : OK")
    print("  split TFACE topology    : OK")
    print("  old Physique assignments: OK")
    print("  new Physique assignment : OK")
    print()
    print("Template bytes :", len(raw))
    print("Template SHA256:", sha256_bytes(raw))
    print("Output bytes   :", len(out_bytes))
    print("Output SHA256  :", sha256_bytes(out_bytes))
    print("Written        :", out_path)
    print("=" * 80)


if __name__ == "__main__":
    main()

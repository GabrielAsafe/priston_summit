#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
Blender -> ASE V5b

Template-preserving exporter with SAME TOPOLOGY.

Supported edits:
- MESH_VERTEX coordinates
- existing TVERT coordinates
- MESH_MTLID
- rigid PHYSIQUE bone assignment

Preserved / not rebuilt:
- vertex count
- face count and A/B/C topology
- AB/BC/CA
- MESH_SMOOTHING
- TVERT count / TFACE indexing
- NODE_TM
- material blocks
- Biped hierarchy
- every unrelated byte from the working ASE template

Safety:
- refuses object-level transforms
- refuses topology changes
- refuses UV edits that require splitting a shared TVERT
- refuses non-rigid/multi-bone Physique
"""

import argparse
import hashlib
import re
import sys
from pathlib import Path

import bpy


FLOAT = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


# ---------------------------------------------------------------------------
# Generic helpers
# ---------------------------------------------------------------------------

def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


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
                        out.append((m.start(), i + 1, text[m.start():i + 1]))
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


def first_mesh_block(geom_block):
    blocks = find_braced_blocks(geom_block, "MESH")
    return blocks[0][2] if blocks else None


def close2(a, b, eps):
    return (
        abs(a[0] - b[0]) <= eps
        and abs(a[1] - b[1]) <= eps
    )


def close3(a, b, eps):
    return (
        abs(a[0] - b[0]) <= eps
        and abs(a[1] - b[1]) <= eps
        and abs(a[2] - b[2]) <= eps
    )


def format_float(v):
    if abs(v) < 0.0000005:
        v = 0.0

    s = f"{v:.6f}"
    s = s.rstrip("0").rstrip(".")

    if "." not in s and "e" not in s.lower():
        s += ".0"

    return s


# ---------------------------------------------------------------------------
# ASE parsing
# ---------------------------------------------------------------------------

def parse_vertices(mesh_block):
    out = {}

    rx = re.compile(
        rf"\*MESH_VERTEX\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
        flags=re.IGNORECASE,
    )

    for m in rx.finditer(mesh_block):
        out[int(m.group(1))] = (
            float(m.group(2)),
            float(m.group(3)),
            float(m.group(4)),
        )

    return out


def parse_faces(mesh_block):
    """
    face index -> {
        tri: (a,b,c),
        mtl_id: int,
        smoothing: original text if found
    }
    """
    out = {}

    face_rx = re.compile(
        r"\*MESH_FACE\s+(\d+):"
        r".*?\bA:\s*(\d+)"
        r".*?\bB:\s*(\d+)"
        r".*?\bC:\s*(\d+)",
        flags=re.IGNORECASE,
    )

    mtl_rx = re.compile(
        r"\*MESH_MTLID\s+(\d+)",
        flags=re.IGNORECASE,
    )

    smooth_rx = re.compile(
        r"\*MESH_SMOOTHING\s+([^\t\r\n]+)",
        flags=re.IGNORECASE,
    )

    for line in mesh_block.splitlines():
        m = face_rx.search(line)
        if not m:
            continue

        idx = int(m.group(1))
        mm = mtl_rx.search(line)
        sm = smooth_rx.search(line)

        out[idx] = {
            "tri": (
                int(m.group(2)),
                int(m.group(3)),
                int(m.group(4)),
            ),
            "mtl_id": int(mm.group(1)) if mm else 0,
            "smoothing": sm.group(1).strip() if sm else None,
        }

    return out


def parse_tverts(mesh_block):
    out = {}

    rx = re.compile(
        rf"\*MESH_TVERT\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
        flags=re.IGNORECASE,
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
        flags=re.IGNORECASE,
    )

    for m in rx.finditer(mesh_block):
        out[int(m.group(1))] = (
            int(m.group(2)),
            int(m.group(3)),
            int(m.group(4)),
        )

    return out


PHYSIQUE_RIGID_RE = re.compile(
    r'\*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE'
    r'\s+(\d+)\s+"([^"]+)"',
    flags=re.IGNORECASE,
)


def parse_physique(geom_block):
    blocks = find_braced_blocks(geom_block, "PHYSIQUE")

    if not blocks:
        return {
            "present": False,
            "declared": None,
            "assignments": [],
            "block": None,
        }

    block = blocks[0][2]

    return {
        "present": True,
        "declared": first_int(
            block,
            "*PHYSIQUE_NUMVERTEXASSIGNMENT",
            None,
        ),
        "assignments": [
            (int(m.group(1)), m.group(2))
            for m in PHYSIQUE_RIGID_RE.finditer(block)
        ],
        "block": block,
    }


# ---------------------------------------------------------------------------
# Blender helpers
# ---------------------------------------------------------------------------

def imported_mesh_objects():
    out = {}

    for obj in bpy.data.objects:
        if obj.type != "MESH":
            continue

        node_name = obj.get("pt_ase_node_name")

        if node_name:
            out[str(node_name)] = obj

    return out


def is_identity_transform(obj, eps=1e-7):
    loc = obj.location
    scale = obj.scale

    if max(abs(loc.x), abs(loc.y), abs(loc.z)) > eps:
        return False

    if max(
        abs(scale.x - 1.0),
        abs(scale.y - 1.0),
        abs(scale.z - 1.0),
    ) > eps:
        return False

    q = (
        obj.rotation_quaternion
        if obj.rotation_mode == "QUATERNION"
        else obj.rotation_euler.to_quaternion()
    )

    if abs(abs(q.w) - 1.0) > eps:
        return False

    if max(abs(q.x), abs(q.y), abs(q.z)) > eps:
        return False

    return True


def slot_to_mtl_id(obj, slot_index):
    if slot_index < 0 or slot_index >= len(obj.data.materials):
        raise RuntimeError(
            f"{obj.name}: invalid Blender material slot {slot_index}"
        )

    mat = obj.data.materials[slot_index]

    if mat is None:
        raise RuntimeError(
            f"{obj.name}: material slot {slot_index} is empty"
        )

    # Importer V2/V4 names:
    # ASE_M{material_ref}_S{mesh_mtl_id}_...
    m = re.match(
        r"^ASE_M(-?\d+)_S(-?\d+)_",
        mat.name,
        flags=re.IGNORECASE,
    )

    if not m:
        raise RuntimeError(
            f"{obj.name}: cannot recover MESH_MTLID from material "
            f"{mat.name!r}. Expected ASE_M#_S#_ prefix."
        )

    expected_material_ref = int(
        obj.get("pt_ase_material_ref", int(m.group(1)))
    )

    if int(m.group(1)) != expected_material_ref:
        raise RuntimeError(
            f"{obj.name}: material slot {mat.name!r} belongs to "
            f"MATERIAL_REF {m.group(1)}, object expects "
            f"{expected_material_ref}"
        )

    return int(m.group(2))


def rigid_bone_for_vertex(obj, vertex_index, eps=1e-6):
    vertex = obj.data.vertices[vertex_index]
    active = []

    for membership in vertex.groups:
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
            f"{obj.name}: vertex {vertex_index} must have exactly "
            f"one active Bip* group for rigid Physique; got {active}"
        )

    bone_name, weight = active[0]

    if abs(weight - 1.0) > 1e-5:
        raise RuntimeError(
            f"{obj.name}: vertex {vertex_index} has non-rigid "
            f"weight {weight} in {bone_name!r}"
        )

    return bone_name


# ---------------------------------------------------------------------------
# Validation and change extraction
# ---------------------------------------------------------------------------

def validate_topology(obj, faces):
    face_indices = sorted(faces)

    if len(obj.data.polygons) != len(face_indices):
        raise RuntimeError(
            f"{obj.name}: face count changed: "
            f"Blender={len(obj.data.polygons)} ASE={len(face_indices)}"
        )

    for poly_index, face_index in enumerate(face_indices):
        poly = obj.data.polygons[poly_index]

        if len(poly.vertices) != 3:
            raise RuntimeError(
                f"{obj.name}: polygon {poly_index} is not a triangle"
            )

        got = tuple(int(v) for v in poly.vertices)
        expected = faces[face_index]["tri"]

        if got != expected:
            raise RuntimeError(
                f"{obj.name}: topology changed at face {face_index}: "
                f"Blender={got} ASE={expected}"
            )


def collect_vertex_changes(obj, original_vertices, eps):
    if len(obj.data.vertices) != len(original_vertices):
        raise RuntimeError(
            f"{obj.name}: vertex count changed"
        )

    changed = {}

    for i, v in enumerate(obj.data.vertices):
        if i not in original_vertices:
            raise RuntimeError(
                f"{obj.name}: ASE vertex index {i} missing"
            )

        old = original_vertices[i]
        new = tuple(float(x) for x in v.co)

        if not close3(old, new, eps):
            changed[i] = (old, new)

    return changed


def collect_uv_changes(
    obj,
    faces,
    tverts,
    tfaces,
    eps,
    flip_v=False,
):
    if not tverts and not tfaces:
        return {}

    uv_layer = obj.data.uv_layers.get("ASE_UV") or obj.data.uv_layers.active

    if uv_layer is None:
        raise RuntimeError(
            f"{obj.name}: ASE has UV but Blender object has no UV layer"
        )

    values_by_tvert = {}
    face_indices = sorted(faces)

    if len(obj.data.polygons) != len(face_indices):
        raise RuntimeError(
            f"{obj.name}: face count changed before UV export"
        )

    for poly_index, face_index in enumerate(face_indices):
        poly = obj.data.polygons[poly_index]
        tf = tfaces.get(face_index)

        if tf is None:
            raise RuntimeError(
                f"{obj.name}: missing TFACE for face {face_index}"
            )

        if len(poly.loop_indices) != 3:
            raise RuntimeError(
                f"{obj.name}: non-triangle polygon in UV export"
            )

        for loop_index, tvert_index in zip(
            poly.loop_indices,
            tf,
        ):
            uv = uv_layer.data[loop_index].uv

            # Convert Blender UV back to ASE convention.
            ase_uv = (
                float(uv.x),
                1.0 - float(uv.y) if flip_v else float(uv.y),
            )

            values_by_tvert.setdefault(
                tvert_index,
                [],
            ).append(ase_uv)

    changed = {}

    for tvert_index, original in tverts.items():
        current_values = values_by_tvert.get(
            tvert_index,
            [],
        )

        if not current_values:
            raise RuntimeError(
                f"{obj.name}: original TVERT {tvert_index} is not "
                f"referenced by current UV loops"
            )

        representative = current_values[0]

        # Same ASE TVERT may be referenced by several corners.
        # If those corners now differ, a TVERT split would be required.
        for other in current_values[1:]:
            if not close2(
                representative,
                other,
                eps,
            ):
                raise RuntimeError(
                    f"{obj.name}: TVERT {tvert_index} now maps to "
                    f"multiple Blender UV coordinates. This requires "
                    f"a new TVERT/TFACE topology and is not supported "
                    f"in V5b."
                )

        old2 = (original[0], original[1])

        if not close2(
            old2,
            representative,
            eps,
        ):
            changed[tvert_index] = (
                original,
                (
                    representative[0],
                    representative[1],
                    original[2],  # preserve W
                ),
            )

    return changed


def collect_material_changes(obj, faces):
    changed = {}
    face_indices = sorted(faces)

    for poly_index, face_index in enumerate(face_indices):
        poly = obj.data.polygons[poly_index]

        new_mtl_id = slot_to_mtl_id(
            obj,
            int(poly.material_index),
        )

        old_mtl_id = faces[face_index]["mtl_id"]

        if new_mtl_id != old_mtl_id:
            changed[face_index] = (
                old_mtl_id,
                new_mtl_id,
            )

    return changed


def collect_physique_changes(obj, physique):
    if not physique["present"]:
        return {}

    assignments = physique["assignments"]

    if (
        physique["declared"] is not None
        and physique["declared"] != len(assignments)
    ):
        raise RuntimeError(
            f"{obj.name}: PHYSIQUE declared="
            f"{physique['declared']} but parsed={len(assignments)}"
        )

    if len(assignments) != len(obj.data.vertices):
        raise RuntimeError(
            f"{obj.name}: V5b currently requires one rigid "
            f"PHYSIQUE assignment per mesh vertex; "
            f"assignments={len(assignments)} "
            f"vertices={len(obj.data.vertices)}"
        )

    old_by_vertex = {}

    for vertex_index, bone_name in assignments:
        if vertex_index in old_by_vertex:
            raise RuntimeError(
                f"{obj.name}: duplicate PHYSIQUE assignment "
                f"for vertex {vertex_index}"
            )
        old_by_vertex[vertex_index] = bone_name

    changed = {}

    for vertex_index in range(len(obj.data.vertices)):
        if vertex_index not in old_by_vertex:
            raise RuntimeError(
                f"{obj.name}: PHYSIQUE missing vertex {vertex_index}"
            )

        new_bone = rigid_bone_for_vertex(
            obj,
            vertex_index,
        )
        old_bone = old_by_vertex[vertex_index]

        if new_bone != old_bone:
            changed[vertex_index] = (
                old_bone,
                new_bone,
            )

    return changed


# ---------------------------------------------------------------------------
# Template patchers
# ---------------------------------------------------------------------------

VERTEX_LINE_RE = re.compile(
    rf"(?m)^"
    rf"(?P<prefix>[ \t]*\*MESH_VERTEX[ \t]+)"
    rf"(?P<idx>\d+)"
    rf"(?P<s1>[ \t]+)"
    rf"(?P<x>{FLOAT})"
    rf"(?P<s2>[ \t]+)"
    rf"(?P<y>{FLOAT})"
    rf"(?P<s3>[ \t]+)"
    rf"(?P<z>{FLOAT})"
    rf"(?P<suffix>[^\r\n]*)"
    rf"(?P<newline>\r?\n|$)"
)


TVERT_LINE_RE = re.compile(
    rf"(?m)^"
    rf"(?P<prefix>[ \t]*\*MESH_TVERT[ \t]+)"
    rf"(?P<idx>\d+)"
    rf"(?P<s1>[ \t]+)"
    rf"(?P<u>{FLOAT})"
    rf"(?P<s2>[ \t]+)"
    rf"(?P<v>{FLOAT})"
    rf"(?P<s3>[ \t]+)"
    rf"(?P<w>{FLOAT})"
    rf"(?P<suffix>[^\r\n]*)"
    rf"(?P<newline>\r?\n|$)"
)


FACE_LINE_RE = re.compile(
    r"(?m)^(?P<line>[ \t]*\*MESH_FACE[^\r\n]*)"
    r"(?P<newline>\r?\n|$)"
)


PHYSIQUE_LINE_RE = re.compile(
    r'(?m)^'
    r'(?P<prefix>[ \t]*'
    r'\*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE'
    r'[ \t]+)'
    r'(?P<idx>\d+)'
    r'(?P<between>[ \t]+)"'
    r'(?P<bone>[^"]+)"'
    r'(?P<suffix>[^\r\n]*)'
    r'(?P<newline>\r?\n|$)',
    flags=re.IGNORECASE,
)


def patch_vertex_lines(block, changes):
    if not changes:
        return block

    seen = set()

    def repl(m):
        idx = int(m.group("idx"))

        if idx not in changes:
            return m.group(0)

        old, new = changes[idx]
        seen.add(idx)

        return (
            m.group("prefix")
            + m.group("idx")
            + m.group("s1")
            + format_float(new[0])
            + m.group("s2")
            + format_float(new[1])
            + m.group("s3")
            + format_float(new[2])
            + m.group("suffix")
            + m.group("newline")
        )

    out = VERTEX_LINE_RE.sub(repl, block)

    missing = sorted(set(changes) - seen)

    if missing:
        raise RuntimeError(
            f"Changed MESH_VERTEX indices not found in ASE text: "
            f"{missing[:20]}"
        )

    return out


def patch_tvert_lines(block, changes):
    if not changes:
        return block

    seen = set()

    def repl(m):
        idx = int(m.group("idx"))

        if idx not in changes:
            return m.group(0)

        old, new = changes[idx]
        seen.add(idx)

        return (
            m.group("prefix")
            + m.group("idx")
            + m.group("s1")
            + format_float(new[0])
            + m.group("s2")
            + format_float(new[1])
            + m.group("s3")
            + format_float(new[2])
            + m.group("suffix")
            + m.group("newline")
        )

    out = TVERT_LINE_RE.sub(repl, block)

    missing = sorted(set(changes) - seen)

    if missing:
        raise RuntimeError(
            f"Changed TVERT indices not found in ASE text: "
            f"{missing[:20]}"
        )

    return out


def patch_face_material_lines(block, changes):
    if not changes:
        return block

    seen = set()

    face_idx_rx = re.compile(
        r"\*MESH_FACE\s+(\d+):",
        flags=re.IGNORECASE,
    )

    mtl_rx = re.compile(
        r"(\*MESH_MTLID\s+)(\d+)",
        flags=re.IGNORECASE,
    )

    def repl(m):
        line = m.group("line")
        fm = face_idx_rx.search(line)

        if not fm:
            return m.group(0)

        face_index = int(fm.group(1))

        if face_index not in changes:
            return m.group(0)

        old, new = changes[face_index]

        if not mtl_rx.search(line):
            raise RuntimeError(
                f"Face {face_index} has no MESH_MTLID in template"
            )

        line2, count = mtl_rx.subn(
            lambda mm: mm.group(1) + str(new),
            line,
            count=1,
        )

        if count != 1:
            raise RuntimeError(
                f"Could not patch MESH_MTLID for face {face_index}"
            )

        seen.add(face_index)

        return line2 + m.group("newline")

    out = FACE_LINE_RE.sub(repl, block)

    missing = sorted(set(changes) - seen)

    if missing:
        raise RuntimeError(
            f"Changed face MTLIDs not found in ASE text: "
            f"{missing[:20]}"
        )

    return out


def patch_physique_lines(block, changes):
    if not changes:
        return block

    seen = set()

    def repl(m):
        idx = int(m.group("idx"))

        if idx not in changes:
            return m.group(0)

        old, new = changes[idx]
        seen.add(idx)

        return (
            m.group("prefix")
            + m.group("idx")
            + m.group("between")
            + '"'
            + new
            + '"'
            + m.group("suffix")
            + m.group("newline")
        )

    out = PHYSIQUE_LINE_RE.sub(repl, block)

    missing = sorted(set(changes) - seen)

    if missing:
        raise RuntimeError(
            f"Changed PHYSIQUE indices not found in ASE text: "
            f"{missing[:20]}"
        )

    return out


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def parse_args():
    argv = sys.argv

    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    p = argparse.ArgumentParser(
        description=(
            "Summit Tale Blender -> ASE V5b "
            "same-topology attribute exporter"
        )
    )

    p.add_argument("--template", required=True)
    p.add_argument("--out", required=True)

    p.add_argument(
        "--objects",
        default="",
        help="Comma-separated NODE_NAME list; default all imported meshes.",
    )

    p.add_argument(
        "--epsilon",
        type=float,
        default=1e-5,
    )

    p.add_argument(
        "--flip-v",
        action="store_true",
        help="Use only if source .blend was imported with --flip-v.",
    )

    p.add_argument(
        "--expect-identical",
        action="store_true",
    )

    p.add_argument(
        "--max-vertex-changed",
        type=int,
        default=1000000,
    )

    p.add_argument(
        "--max-uv-changed",
        type=int,
        default=1000000,
    )

    p.add_argument(
        "--max-material-changed",
        type=int,
        default=1000000,
    )

    p.add_argument(
        "--max-physique-changed",
        type=int,
        default=1000000,
    )

    return p.parse_args(argv)


def main():
    args = parse_args()

    template_path = Path(args.template).expanduser().resolve()
    out_path = Path(args.out).expanduser().resolve()

    raw = template_path.read_bytes()
    text = raw.decode("latin-1")

    scene_objects = imported_mesh_objects()

    if args.objects.strip():
        wanted = [
            x.strip()
            for x in args.objects.split(",")
            if x.strip()
        ]
    else:
        wanted = sorted(scene_objects)

    if not wanted:
        raise RuntimeError(
            "No imported PT ASE mesh objects found in scene"
        )

    geom_blocks = find_braced_blocks(
        text,
        "GEOMOBJECT",
    )

    by_name = {}

    for start, end, block in geom_blocks:
        name = first_quoted(
            block,
            "*NODE_NAME",
        )

        if name:
            by_name.setdefault(
                name,
                [],
            ).append((start, end, block))

    print("=" * 80)
    print("PT BLENDER -> ASE V5b")
    print("SAME-TOPOLOGY ATTRIBUTE EXPORT")
    print("=" * 80)
    print("Template :", template_path)
    print("Output   :", out_path)
    print("Meshes   :", ", ".join(wanted))
    print()
    print("Template bytes :", len(raw))
    print("Template SHA256:", sha256_bytes(raw))
    print()

    replacements = []
    results = []

    totals = {
        "vertex": 0,
        "uv": 0,
        "material": 0,
        "physique": 0,
    }

    for name in wanted:
        obj = scene_objects.get(name)

        if obj is None:
            raise RuntimeError(
                f"Blender object {name!r} not found"
            )

        hits = by_name.get(name, [])

        if len(hits) != 1:
            raise RuntimeError(
                f"Template NODE_NAME {name!r}: expected 1 GEOMOBJECT, "
                f"found {len(hits)}"
            )

        if not is_identity_transform(obj):
            raise RuntimeError(
                f"{name}: object transform is not identity. "
                f"Edit mesh in Edit Mode; do not transform whole object."
            )

        start, end, block = hits[0]
        mesh_block = first_mesh_block(block)

        if mesh_block is None:
            raise RuntimeError(
                f"{name}: missing MESH block"
            )

        vertices = parse_vertices(mesh_block)
        faces = parse_faces(mesh_block)
        tverts = parse_tverts(mesh_block)
        tfaces = parse_tfaces(mesh_block)
        physique = parse_physique(block)

        validate_topology(
            obj,
            faces,
        )

        vchanges = collect_vertex_changes(
            obj,
            vertices,
            args.epsilon,
        )

        uvchanges = collect_uv_changes(
            obj,
            faces,
            tverts,
            tfaces,
            args.epsilon,
            flip_v=args.flip_v,
        )

        mchanges = collect_material_changes(
            obj,
            faces,
        )

        pchanges = collect_physique_changes(
            obj,
            physique,
        )

        totals["vertex"] += len(vchanges)
        totals["uv"] += len(uvchanges)
        totals["material"] += len(mchanges)
        totals["physique"] += len(pchanges)

        patched = block
        patched = patch_vertex_lines(
            patched,
            vchanges,
        )
        patched = patch_tvert_lines(
            patched,
            uvchanges,
        )
        patched = patch_face_material_lines(
            patched,
            mchanges,
        )
        patched = patch_physique_lines(
            patched,
            pchanges,
        )

        if patched != block:
            replacements.append(
                (start, end, patched)
            )

        results.append({
            "name": name,
            "vertices": len(vertices),
            "faces": len(faces),
            "tverts": len(tverts),
            "tfaces": len(tfaces),
            "physique": len(
                physique["assignments"]
            ),
            "vchanges": vchanges,
            "uvchanges": uvchanges,
            "mchanges": mchanges,
            "pchanges": pchanges,
        })

    caps = [
        (
            "vertex",
            totals["vertex"],
            args.max_vertex_changed,
        ),
        (
            "uv",
            totals["uv"],
            args.max_uv_changed,
        ),
        (
            "material",
            totals["material"],
            args.max_material_changed,
        ),
        (
            "physique",
            totals["physique"],
            args.max_physique_changed,
        ),
    ]

    for label, count, cap in caps:
        if count > cap:
            raise RuntimeError(
                f"Safety stop: {label} changes={count} exceeds cap={cap}"
            )

    out_text = text

    for start, end, patched in sorted(
        replacements,
        key=lambda x: x[0],
        reverse=True,
    ):
        out_text = (
            out_text[:start]
            + patched
            + out_text[end:]
        )

    out_bytes = out_text.encode("latin-1")

    if args.expect_identical:
        if any(totals.values()):
            raise RuntimeError(
                f"--expect-identical failed: changes={totals}"
            )

        if out_bytes != raw:
            raise RuntimeError(
                "--expect-identical failed: output differs despite "
                "zero detected changes"
            )

    out_path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )
    out_path.write_bytes(out_bytes)

    print("=" * 80)
    print("RESULT")
    print("=" * 80)

    for item in results:
        print(
            f"{item['name']}: "
            f"V={item['vertices']} "
            f"F={item['faces']} "
            f"TV={item['tverts']} "
            f"TF={item['tfaces']} "
            f"PHYS={item['physique']} "
            f"VCHG={len(item['vchanges'])} "
            f"UVCHG={len(item['uvchanges'])} "
            f"MTLCHG={len(item['mchanges'])} "
            f"PHYSCHG={len(item['pchanges'])}"
        )

    print()
    print("Total vertex changes  :", totals["vertex"])
    print("Total TVERT changes   :", totals["uv"])
    print("Total MTLID changes   :", totals["material"])
    print("Total PHYS changes    :", totals["physique"])
    print()
    print("Output bytes          :", len(out_bytes))
    print("Output SHA256         :", sha256_bytes(out_bytes))
    print("Byte-identical        :", out_bytes == raw)
    print("Written               :", out_path)

    if args.expect_identical:
        print()
        print("V5b NO-OP: BYTE-IDENTICAL")

    print("=" * 80)


if __name__ == "__main__":
    main()

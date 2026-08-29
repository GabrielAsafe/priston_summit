#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Priston Tale / Summit Tale - Blender -> ASE V5a

Purpose
-------
Conservative, template-preserving round-trip exporter.

This V5a DOES NOT rebuild an ASE from scratch.

It opens the current Blender scene (normally the .blend produced by V4),
uses the original working ASE as a byte-preserving template, validates that:

    - mesh vertex count is unchanged;
    - face topology/order is unchanged;
    - UVs are unchanged;
    - rigid PHYSIQUE assignment is unchanged;
    - mesh object transforms are identity;

and then patches ONLY MESH_VERTEX coordinates that actually changed.

If no vertex was edited, output should be byte-identical to the template.

This is intentionally narrower than a final exporter. It is designed to
prove the Blender -> ASE direction before faces/UV/normals/Physique are
rewritten in V5b.
"""

import argparse
import hashlib
import re
import sys
from pathlib import Path

import bpy


FLOAT = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


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
        i = brace
        quote = False
        escape = False

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
    out = {}

    rx = re.compile(
        r"\*MESH_FACE\s+(\d+):"
        r".*?\bA:\s*(\d+)"
        r".*?\bB:\s*(\d+)"
        r".*?\bC:\s*(\d+)",
        flags=re.IGNORECASE,
    )

    for line in mesh_block.splitlines():
        m = rx.search(line)
        if not m:
            continue

        out[int(m.group(1))] = (
            int(m.group(2)),
            int(m.group(3)),
            int(m.group(4)),
        )

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
            "unsupported": [],
        }

    block = blocks[0][2]

    declared = first_int(
        block,
        "*PHYSIQUE_NUMVERTEXASSIGNMENT",
        None,
    )

    assignments = [
        (int(m.group(1)), m.group(2))
        for m in PHYSIQUE_RIGID_RE.finditer(block)
    ]

    unsupported = []

    for line in block.splitlines():
        if "PHYSIQUE_VERTEXASSIGNMENT" not in line.upper():
            continue
        if PHYSIQUE_RIGID_RE.search(line):
            continue
        unsupported.append(line.strip())

    return {
        "present": True,
        "declared": declared,
        "assignments": assignments,
        "unsupported": unsupported,
    }


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


def close3(a, b, eps):
    return (
        abs(a[0] - b[0]) <= eps
        and abs(a[1] - b[1]) <= eps
        and abs(a[2] - b[2]) <= eps
    )


def close2(a, b, eps):
    return (
        abs(a[0] - b[0]) <= eps
        and abs(a[1] - b[1]) <= eps
    )


def validate_topology(obj, original_faces):
    mesh = obj.data
    face_indices = sorted(original_faces)

    if len(mesh.polygons) != len(face_indices):
        raise RuntimeError(
            f"{obj.name}: face count changed: "
            f"Blender={len(mesh.polygons)} ASE={len(face_indices)}"
        )

    for poly_index, face_index in enumerate(face_indices):
        poly = mesh.polygons[poly_index]

        if len(poly.vertices) != 3:
            raise RuntimeError(
                f"{obj.name}: polygon {poly_index} is not a triangle. "
                f"V5a does not export topology."
            )

        blender_tri = tuple(int(v) for v in poly.vertices)
        ase_tri = original_faces[face_index]

        if blender_tri != ase_tri:
            raise RuntimeError(
                f"{obj.name}: topology/order changed at face {face_index}: "
                f"Blender={blender_tri} ASE={ase_tri}. "
                f"V5a refuses topology changes."
            )


def validate_uv(obj, original_faces, tverts, tfaces, eps, flip_v=False):
    if not tverts and not tfaces:
        return

    mesh = obj.data
    uv_layer = mesh.uv_layers.get("ASE_UV") or mesh.uv_layers.active

    if uv_layer is None:
        raise RuntimeError(
            f"{obj.name}: original ASE has UV but Blender mesh has no UV layer."
        )

    face_indices = sorted(original_faces)

    for poly_index, face_index in enumerate(face_indices):
        tf = tfaces.get(face_index)

        if tf is None:
            raise RuntimeError(
                f"{obj.name}: missing original TFACE for face {face_index}"
            )

        poly = mesh.polygons[poly_index]

        if len(poly.loop_indices) != 3:
            raise RuntimeError(
                f"{obj.name}: non-triangle polygon while validating UV."
            )

        for corner, (loop_index, tvert_index) in enumerate(
            zip(poly.loop_indices, tf)
        ):
            tv = tverts.get(tvert_index)

            if tv is None:
                raise RuntimeError(
                    f"{obj.name}: TFACE {face_index} references "
                    f"missing TVERT {tvert_index}"
                )

            expected_v = 1.0 - tv[1] if flip_v else tv[1]
            expected = (tv[0], expected_v)

            got_uv = uv_layer.data[loop_index].uv
            got = (float(got_uv.x), float(got_uv.y))

            if not close2(got, expected, eps):
                raise RuntimeError(
                    f"{obj.name}: UV changed at face={face_index} "
                    f"corner={corner}: Blender={got} ASE={expected}. "
                    f"V5a does not export UV edits."
                )


def validate_physique(obj, parsed, eps):
    if not parsed["present"]:
        return

    if parsed["unsupported"]:
        raise RuntimeError(
            f"{obj.name}: unsupported PHYSIQUE syntax exists; "
            f"V5a cannot safely validate it: {parsed['unsupported'][:3]}"
        )

    if (
        parsed["declared"] is not None
        and parsed["declared"] != len(parsed["assignments"])
    ):
        raise RuntimeError(
            f"{obj.name}: PHYSIQUE declared={parsed['declared']} "
            f"but parsed rigid assignments={len(parsed['assignments'])}"
        )

    expected_by_vertex = {}

    for vertex_index, bone_name in parsed["assignments"]:
        if vertex_index in expected_by_vertex:
            raise RuntimeError(
                f"{obj.name}: duplicate PHYSIQUE assignment "
                f"for vertex {vertex_index}."
            )
        expected_by_vertex[vertex_index] = bone_name

    for vertex_index, bone_name in expected_by_vertex.items():
        if vertex_index < 0 or vertex_index >= len(obj.data.vertices):
            raise RuntimeError(
                f"{obj.name}: PHYSIQUE vertex {vertex_index} outside mesh."
            )

        group = obj.vertex_groups.get(bone_name)

        if group is None:
            raise RuntimeError(
                f"{obj.name}: expected vertex group {bone_name!r} missing."
            )

        try:
            weight = group.weight(vertex_index)
        except RuntimeError:
            raise RuntimeError(
                f"{obj.name}: vertex {vertex_index} is no longer "
                f"assigned to {bone_name!r}."
            )

        if abs(weight - 1.0) > eps:
            raise RuntimeError(
                f"{obj.name}: vertex {vertex_index} weight changed "
                f"for {bone_name!r}: {weight}; expected 1.0."
            )

        vertex = obj.data.vertices[vertex_index]

        for membership in vertex.groups:
            vg = obj.vertex_groups[membership.group]

            if not vg.name.lower().startswith("bip"):
                continue

            if membership.weight <= eps:
                continue

            if vg.name != bone_name:
                raise RuntimeError(
                    f"{obj.name}: vertex {vertex_index} also has "
                    f"Bip group {vg.name!r} weight={membership.weight}; "
                    f"expected only {bone_name!r}. "
                    f"V5a does not export rig edits."
                )


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


def format_float(v):
    if abs(v) < 0.0000005:
        v = 0.0

    s = f"{v:.6f}"
    s = s.rstrip("0").rstrip(".")

    if "." not in s and "e" not in s.lower():
        s += ".0"

    return s


def patch_vertex_lines(geom_block, obj, eps):
    mesh_block = first_mesh_block(geom_block)
    original_vertices = parse_vertices(mesh_block)

    if len(obj.data.vertices) != len(original_vertices):
        raise RuntimeError(
            f"{obj.name}: vertex count changed: "
            f"Blender={len(obj.data.vertices)} "
            f"ASE={len(original_vertices)}."
        )

    missing = [
        i
        for i in range(len(obj.data.vertices))
        if i not in original_vertices
    ]

    if missing:
        raise RuntimeError(
            f"{obj.name}: ASE vertex indices are not dense; "
            f"first missing={missing[:10]}"
        )

    changed = []

    for i, vertex in enumerate(obj.data.vertices):
        old = original_vertices[i]
        new = tuple(float(v) for v in vertex.co)

        if not close3(old, new, eps):
            changed.append((i, old, new))

    change_by_index = {
        idx: (old, new)
        for idx, old, new in changed
    }

    seen = set()

    def repl(m):
        idx = int(m.group("idx"))

        if idx not in change_by_index:
            return m.group(0)

        old, new = change_by_index[idx]
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

    patched = VERTEX_LINE_RE.sub(repl, geom_block)

    not_seen = sorted(set(change_by_index) - seen)

    if not_seen:
        raise RuntimeError(
            f"{obj.name}: changed vertices not found in ASE text: "
            f"{not_seen[:20]}"
        )

    return patched, changed


def parse_args():
    argv = sys.argv

    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    p = argparse.ArgumentParser(
        description=(
            "Summit Tale Blender -> ASE V5a "
            "template-preserving exporter"
        )
    )

    p.add_argument("--template", required=True)
    p.add_argument("--out", required=True)

    p.add_argument(
        "--objects",
        default="",
        help=(
            "Comma-separated ASE NODE_NAME values. "
            "Default: all V4-imported mesh objects."
        ),
    )

    p.add_argument(
        "--epsilon",
        type=float,
        default=1e-5,
    )

    p.add_argument(
        "--flip-v",
        action="store_true",
        help="Use only if the .blend was imported with --flip-v.",
    )

    p.add_argument(
        "--max-changed",
        type=int,
        default=1000000,
        help="Safety cap for total changed vertices.",
    )

    p.add_argument(
        "--expect-identical",
        action="store_true",
        help=(
            "No-op test: require zero changed vertices and "
            "byte-identical output."
        ),
    )

    return p.parse_args(argv)


def main():
    args = parse_args()

    template_path = Path(args.template).expanduser().resolve()
    out_path = Path(args.out).expanduser().resolve()

    if not template_path.is_file():
        raise FileNotFoundError(template_path)

    raw = template_path.read_bytes()

    # latin-1 gives a reversible byte-preserving text mapping.
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
            "No V4 imported mesh objects found. "
            "Expected pt_ase_node_name metadata."
        )

    for name in wanted:
        if name not in scene_objects:
            raise RuntimeError(
                f"Requested mesh {name!r} not found in current .blend."
            )

    geom_blocks = find_braced_blocks(text, "GEOMOBJECT")
    blocks_by_name = {}

    for start, end, block in geom_blocks:
        name = first_quoted(block, "*NODE_NAME")

        if not name:
            continue

        blocks_by_name.setdefault(name, []).append(
            (start, end, block)
        )

    for name in wanted:
        hits = blocks_by_name.get(name, [])

        if len(hits) != 1:
            raise RuntimeError(
                f"Template must contain exactly one GEOMOBJECT "
                f"NODE_NAME {name!r}; found {len(hits)}"
            )

    print("=" * 80)
    print("PT BLENDER -> ASE V5a")
    print("CONSERVATIVE TEMPLATE ROUND-TRIP")
    print("=" * 80)
    print("Template :", template_path)
    print("Output   :", out_path)
    print("Meshes   :", ", ".join(wanted))
    print("Epsilon  :", args.epsilon)
    print("Flip V   :", args.flip_v)
    print()
    print("Template bytes :", len(raw))
    print("Template SHA256:", sha256_bytes(raw))
    print()

    replacements = []
    total_changed = 0
    report = []

    for name in wanted:
        obj = scene_objects[name]
        start, end, block = blocks_by_name[name][0]

        if not is_identity_transform(obj):
            raise RuntimeError(
                f"{name}: object transform is not identity. "
                f"Use Edit Mode for V5a vertex edits."
            )

        mesh_block = first_mesh_block(block)

        if not mesh_block:
            raise RuntimeError(
                f"{name}: GEOMOBJECT has no MESH block."
            )

        original_vertices = parse_vertices(mesh_block)
        original_faces = parse_faces(mesh_block)
        tverts = parse_tverts(mesh_block)
        tfaces = parse_tfaces(mesh_block)
        physique = parse_physique(block)

        if len(obj.data.vertices) != len(original_vertices):
            raise RuntimeError(
                f"{name}: vertex count changed."
            )

        validate_topology(obj, original_faces)
        validate_uv(
            obj,
            original_faces,
            tverts,
            tfaces,
            eps=args.epsilon,
            flip_v=args.flip_v,
        )
        validate_physique(
            obj,
            physique,
            eps=args.epsilon,
        )

        patched_block, changed = patch_vertex_lines(
            block,
            obj,
            eps=args.epsilon,
        )

        total_changed += len(changed)

        report.append({
            "name": name,
            "vertices": len(original_vertices),
            "faces": len(original_faces),
            "tverts": len(tverts),
            "tfaces": len(tfaces),
            "physique": len(physique["assignments"]),
            "changed": changed,
        })

        if patched_block != block:
            replacements.append((start, end, patched_block))

    if total_changed > args.max_changed:
        raise RuntimeError(
            f"Safety stop: {total_changed} changed vertices exceeds "
            f"--max-changed {args.max_changed}"
        )

    out_text = text

    for start, end, replacement in sorted(
        replacements,
        key=lambda item: item[0],
        reverse=True,
    ):
        out_text = out_text[:start] + replacement + out_text[end:]

    out_bytes = out_text.encode("latin-1")

    if args.expect_identical:
        if total_changed != 0:
            raise RuntimeError(
                f"--expect-identical failed: "
                f"{total_changed} vertices changed."
            )

        if out_bytes != raw:
            raise RuntimeError(
                "--expect-identical failed: output differs from "
                "template with zero detected changes."
            )

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(out_bytes)

    print("=" * 80)
    print("VALIDATION / PATCH RESULT")
    print("=" * 80)

    for item in report:
        print(
            f"{item['name']}: "
            f"V={item['vertices']} "
            f"F={item['faces']} "
            f"TV={item['tverts']} "
            f"TF={item['tfaces']} "
            f"PHYS={item['physique']} "
            f"CHANGED={len(item['changed'])}"
        )

        for idx, old, new in item["changed"][:20]:
            print(
                f"  vertex {idx}: {old} -> {new}"
            )

        if len(item["changed"]) > 20:
            print(
                f"  ... {len(item['changed']) - 20} more"
            )

    print()
    print("Total changed vertices:", total_changed)
    print("Output bytes          :", len(out_bytes))
    print("Output SHA256         :", sha256_bytes(out_bytes))
    print("Byte-identical        :", out_bytes == raw)
    print("Written               :", out_path)

    if args.expect_identical:
        print()
        print("NO-OP ROUND-TRIP: BYTE-IDENTICAL")

    print("=" * 80)


if __name__ == "__main__":
    main()

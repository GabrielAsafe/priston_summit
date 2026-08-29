#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
V5c format gate probe

Purpose:
- audit MESH_TVERT W values per mesh;
- audit AB/BC/CA edge flags and smoothing/material distributions;
- optionally scan the Summit source tree for ASE loader references.

This script does not modify any file.
"""

import argparse
import collections
import re
from pathlib import Path


FLOAT = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


def find_braced_blocks(text, token):
    rx = re.compile(rf"(?im)^[ \t]*\*{re.escape(token)}[ \t]*\{{")
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


def first_mesh_block(geom_block):
    blocks = find_braced_blocks(geom_block, "MESH")
    return blocks[0][2] if blocks else None


TVERT_RE = re.compile(
    rf"\*MESH_TVERT\s+(\d+)\s+({FLOAT})\s+({FLOAT})\s+({FLOAT})",
    flags=re.IGNORECASE,
)

FACE_RE = re.compile(
    r"\*MESH_FACE\s+(\d+):"
    r".*?\bA:\s*(\d+)"
    r".*?\bB:\s*(\d+)"
    r".*?\bC:\s*(\d+)"
    r".*?\bAB:\s*(\d+)"
    r".*?\bBC:\s*(\d+)"
    r".*?\bCA:\s*(\d+)"
    r".*?\*MESH_SMOOTHING\s+([^\t\r\n]+?)"
    r"[ \t]+\*MESH_MTLID\s+(\d+)",
    flags=re.IGNORECASE,
)


def audit_mesh(name, mesh_block):
    tverts = []

    for m in TVERT_RE.finditer(mesh_block):
        tverts.append(
            (
                int(m.group(1)),
                float(m.group(2)),
                float(m.group(3)),
                float(m.group(4)),
            )
        )

    faces = []

    for line in mesh_block.splitlines():
        m = FACE_RE.search(line)
        if not m:
            continue

        faces.append({
            "face": int(m.group(1)),
            "a": int(m.group(2)),
            "b": int(m.group(3)),
            "c": int(m.group(4)),
            "ab": int(m.group(5)),
            "bc": int(m.group(6)),
            "ca": int(m.group(7)),
            "smoothing": m.group(8).strip(),
            "mtlid": int(m.group(9)),
        })

    print("=" * 80)
    print("MESH:", name)
    print("=" * 80)

    if tverts:
        ws = [x[3] for x in tverts]
        rounded = collections.Counter(round(w, 6) for w in ws)

        print("TVERT count :", len(tverts))
        print("W min       :", min(ws))
        print("W max       :", max(ws))
        print("W unique(6) :", len(rounded))
        print("Most common W values:")

        for value, count in rounded.most_common(12):
            print(f"  {value: .6f} -> {count}")
    else:
        print("TVERT count : 0")

    print()

    if faces:
        triples = collections.Counter(
            (f["ab"], f["bc"], f["ca"])
            for f in faces
        )
        smooth = collections.Counter(
            f["smoothing"]
            for f in faces
        )
        mtl = collections.Counter(
            f["mtlid"]
            for f in faces
        )

        print("Face count  :", len(faces))
        print("AB/BC/CA patterns:")

        for flags, count in triples.most_common():
            print(f"  {flags} -> {count}")

        print("Smoothing groups:")

        for value, count in smooth.most_common():
            print(f"  {value!r} -> {count}")

        print("MTLID distribution:")

        for value, count in sorted(mtl.items()):
            print(f"  {value} -> {count}")
    else:
        print("Face count  : 0")

    print()


def scan_source(source_root):
    needles = [
        "MESH_TVERT",
        "MESH_TFACE",
        "MESH_FACE",
        "MESH_SMOOTHING",
        "MESH_MTLID",
        "TVERT",
        "TFACE",
    ]

    allowed_ext = {
        ".cpp", ".c", ".cc", ".cxx",
        ".h", ".hpp", ".hh",
    }

    hits = []

    for path in source_root.rglob("*"):
        if not path.is_file():
            continue
        if path.suffix.lower() not in allowed_ext:
            continue

        try:
            lines = path.read_text(
                encoding="latin-1",
                errors="replace",
            ).splitlines()
        except Exception:
            continue

        for lineno, line in enumerate(lines, 1):
            upper = line.upper()

            if any(n.upper() in upper for n in needles):
                hits.append(
                    (path, lineno, line.rstrip())
                )

    print("=" * 80)
    print("SOURCE SCAN")
    print("=" * 80)
    print("Root:", source_root)
    print("Hits:", len(hits))
    print()

    for path, lineno, line in hits[:300]:
        try:
            rel = path.relative_to(source_root)
        except ValueError:
            rel = path

        print(f"{rel}:{lineno}: {line}")

    if len(hits) > 300:
        print()
        print(f"... {len(hits) - 300} additional hits omitted")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--ase", required=True)
    p.add_argument(
        "--objects",
        default="MshC19,MbhC19,MlhC20,MahC20",
    )
    p.add_argument("--source-root")
    args = p.parse_args()

    ase_path = Path(args.ase).expanduser().resolve()
    text = ase_path.read_bytes().decode("latin-1")

    wanted = {
        x.strip()
        for x in args.objects.split(",")
        if x.strip()
    }

    found = set()

    for _, _, geom in find_braced_blocks(text, "GEOMOBJECT"):
        name = first_quoted(geom, "*NODE_NAME")

        if name not in wanted:
            continue

        mesh_block = first_mesh_block(geom)

        if mesh_block is None:
            continue

        found.add(name)
        audit_mesh(name, mesh_block)

    missing = sorted(wanted - found)

    if missing:
        print("WARNING: requested mesh(es) not found:", missing)

    if args.source_root:
        source_root = Path(args.source_root).expanduser().resolve()

        if not source_root.is_dir():
            raise SystemExit(
                f"source root is not a directory: {source_root}"
            )

        scan_source(source_root)


if __name__ == "__main__":
    main()

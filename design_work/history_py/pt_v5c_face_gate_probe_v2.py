#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Summit Tale / Priston Tale
V5c FACE gate probe v2

Purpose:
- robustly audit every *MESH_FACE line;
- never require MESH_SMOOTHING or MESH_MTLID in order to count a face;
- report lines whose A/B/C or AB/BC/CA fields cannot be parsed;
- report missing / unusual smoothing and MTLID separately.

Does not modify any file.
"""

import argparse
import collections
import re
from pathlib import Path


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


FACE_INDEX_RE = re.compile(
    r"\*MESH_FACE\s+(\d+):",
    flags=re.IGNORECASE,
)

FIELD_RE = {
    key: re.compile(
        rf"\b{key}:\s*(-?\d+)",
        flags=re.IGNORECASE,
    )
    for key in ("A", "B", "C", "AB", "BC", "CA")
}

SMOOTH_RE = re.compile(
    r"\*MESH_SMOOTHING(?:[ \t]+([^*\r\n\t]+?))?(?=[ \t]*(?:\*|$))",
    flags=re.IGNORECASE,
)

MTL_RE = re.compile(
    r"\*MESH_MTLID\s+(-?\d+)",
    flags=re.IGNORECASE,
)


def audit_mesh(name, mesh_block):
    face_lines = [
        line
        for line in mesh_block.splitlines()
        if FACE_INDEX_RE.search(line)
    ]

    parsed = []
    malformed = []

    for line in face_lines:
        im = FACE_INDEX_RE.search(line)
        idx = int(im.group(1))

        values = {}

        for key, rx in FIELD_RE.items():
            m = rx.search(line)
            values[key] = int(m.group(1)) if m else None

        sm = SMOOTH_RE.search(line)
        mt = MTL_RE.search(line)

        smoothing = None
        if sm:
            value = sm.group(1)
            smoothing = value.strip() if value else ""

        mtlid = int(mt.group(1)) if mt else None

        missing_core = [
            key
            for key in ("A", "B", "C", "AB", "BC", "CA")
            if values[key] is None
        ]

        item = {
            "idx": idx,
            **values,
            "smoothing": smoothing,
            "mtlid": mtlid,
            "line": line,
        }

        parsed.append(item)

        if missing_core:
            malformed.append(
                (idx, f"missing {missing_core}", line)
            )

    indices = [x["idx"] for x in parsed]
    duplicate_indices = sorted(
        idx for idx, count in collections.Counter(indices).items()
        if count != 1
    )

    dense_missing = []
    if indices:
        dense_missing = sorted(
            set(range(min(indices), max(indices) + 1)) - set(indices)
        )

    complete_flags = [
        x for x in parsed
        if all(x[k] is not None for k in ("AB", "BC", "CA"))
    ]

    patterns = collections.Counter(
        (x["AB"], x["BC"], x["CA"])
        for x in complete_flags
    )

    smoothing = collections.Counter(
        "<MISSING>" if x["smoothing"] is None
        else "<EMPTY>" if x["smoothing"] == ""
        else x["smoothing"]
        for x in parsed
    )

    mtlids = collections.Counter(
        "<MISSING>" if x["mtlid"] is None
        else x["mtlid"]
        for x in parsed
    )

    print("=" * 80)
    print("MESH:", name)
    print("=" * 80)
    print("MESH_FACE physical lines :", len(face_lines))
    print("Parsed face records      :", len(parsed))
    print("Index min/max            :", (
        (min(indices), max(indices)) if indices else None
    ))
    print("Missing dense indices    :", dense_missing)
    print("Duplicate indices        :", duplicate_indices)
    print("Malformed core records   :", len(malformed))
    print()

    print("AB/BC/CA patterns:")
    for key, count in patterns.most_common():
        print(f"  {key} -> {count}")

    print()
    print("Smoothing distribution:")
    for key, count in smoothing.most_common():
        print(f"  {key!r} -> {count}")

    print()
    print("MTLID distribution:")
    for key, count in sorted(
        mtlids.items(),
        key=lambda kv: str(kv[0]),
    ):
        print(f"  {key!r} -> {count}")

    unusual = [
        x for x in parsed
        if x["smoothing"] in (None, "")
        or x["mtlid"] is None
        or any(x[k] is None for k in ("A", "B", "C", "AB", "BC", "CA"))
    ]

    if unusual:
        print()
        print("UNUSUAL / PREVIOUSLY SKIPPABLE FACE LINES:")
        for x in unusual:
            print(f"FACE {x['idx']}: {x['line']}")

    if malformed:
        print()
        print("MALFORMED CORE DETAILS:")
        for idx, why, line in malformed:
            print(f"FACE {idx}: {why}")
            print(line)

    print()


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--ase", required=True)
    p.add_argument(
        "--objects",
        default="MshC19,MbhC19,MlhC20,MahC20",
    )
    args = p.parse_args()

    ase_path = Path(args.ase).expanduser().resolve()
    text = ase_path.read_bytes().decode("latin-1")

    wanted = [
        x.strip()
        for x in args.objects.split(",")
        if x.strip()
    ]

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

    missing = [x for x in wanted if x not in found]
    if missing:
        print("MISSING OBJECTS:", missing)


if __name__ == "__main__":
    main()

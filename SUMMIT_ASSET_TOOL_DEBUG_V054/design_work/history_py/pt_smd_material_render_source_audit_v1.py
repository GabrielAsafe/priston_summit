#!/usr/bin/env python3
"""
PT SMD Material Render Source Audit V1

Read-only source scanner.

Locates declarations and runtime usages of the smMATERIAL fields we may expose
to Blender:
  SelfIllum
  Transparency
  BlendType
  Shade
  TwoSide
  MapOpacity
  TextureType
  TextureStageState
  TextureFormState
  TextureClip
  UseState
  MeshState

Also searches for:
  SMMAT_BLEND_*
  SMTEX_TYPE_*
  SetRenderState
  SetTextureStageState
  D3DRS_CULLMODE
  D3DRS_ALPHABLENDENABLE
  D3DRS_SRCBLEND
  D3DRS_DESTBLEND

No source or client file is modified.
"""

from __future__ import annotations

import argparse
import json
import pathlib


TERMS = [
    "SelfIllum",
    "Transparency",
    "BlendType",
    "Shade",
    "TwoSide",
    "MapOpacity",
    "TextureType",
    "TextureStageState",
    "TextureFormState",
    "TextureClip",
    "UseState",
    "MeshState",
    "SMMAT_BLEND_",
    "SMTEX_TYPE_",
    "SetRenderState",
    "SetTextureStageState",
    "D3DRS_CULLMODE",
    "D3DRS_ALPHABLENDENABLE",
    "D3DRS_SRCBLEND",
    "D3DRS_DESTBLEND",
]

TEXT_EXTS = {
    ".h", ".hpp", ".hh", ".c", ".cc", ".cpp", ".cxx", ".inc", ".inl"
}


def read_text(path):
    raw = path.read_bytes()
    for enc in ("utf-8", "cp1252", "latin1"):
        try:
            return raw.decode(enc)
        except UnicodeDecodeError:
            pass
    return raw.decode("latin1", "replace")


def contexts(lines, needle, before=8, after=12):
    out = []
    n = needle.lower()
    for i, line in enumerate(lines):
        if n not in line.lower():
            continue
        a = max(0, i-before)
        b = min(len(lines), i+after+1)
        out.append({
            "match_line": i+1,
            "start_line": a+1,
            "end_line": b,
            "text": "".join(
                f"{j+1:6d}: {lines[j]}" for j in range(a, b)
            ),
        })
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-root", required=True)
    ap.add_argument(
        "--report",
        default="logs/pt_smd_material_render_source_audit_v1.json",
    )
    args = ap.parse_args()

    root = pathlib.Path(args.source_root).expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(root)

    scanned = 0
    matched = []
    term_counts = {t: 0 for t in TERMS}

    for p in sorted(root.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in TEXT_EXTS:
            continue
        scanned += 1
        try:
            text = read_text(p)
        except OSError:
            continue

        low = text.lower()
        present = [t for t in TERMS if t.lower() in low]
        if not present:
            continue

        lines = text.splitlines(keepends=True)
        hits = {}
        for term in present:
            c = contexts(lines, term)
            if c:
                hits[term] = c
                term_counts[term] += len(c)

        matched.append({
            "path": str(p.relative_to(root)).replace("\\", "/"),
            "hits": hits,
        })

    result = {
        "tool": "PT_SMD_MATERIAL_RENDER_SOURCE_AUDIT_V1",
        "source_root": str(root),
        "text_files_scanned": scanned,
        "matched_file_count": len(matched),
        "term_counts": term_counts,
        "matched_files": matched,
        "source_modified": False,
        "client_modified": False,
        "material_render_source_audit_check": "PASS",
    }

    rp = pathlib.Path(args.report).expanduser()
    if not rp.is_absolute():
        rp = pathlib.Path.cwd() / rp
    rp = rp.resolve()
    rp.parent.mkdir(parents=True, exist_ok=True)
    rp.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n",
                  encoding="utf-8")

    print("PT_SMD_MATERIAL_RENDER_SOURCE_AUDIT_V1")
    print(f"source_root={root}")
    print(f"text_files_scanned={scanned}")
    print(f"matched_file_count={len(matched)}")
    for term in TERMS:
        print(f"TERM {term}: matches={term_counts[term]}")

    priority = [
        "SelfIllum", "Transparency", "BlendType", "TwoSide",
        "SMMAT_BLEND_", "D3DRS_CULLMODE",
        "D3DRS_ALPHABLENDENABLE", "D3DRS_SRCBLEND", "D3DRS_DESTBLEND",
    ]

    for item in matched:
        interesting = [t for t in priority if t in item["hits"]]
        if not interesting:
            continue
        print(f"SOURCE_FILE {item['path']}")
        for term in interesting:
            print(f"  TERM {term}: matches={len(item['hits'][term])}")
            for c in item["hits"][term][:6]:
                print(f"  CONTEXT lines={c['start_line']}..{c['end_line']}")
                print(c["text"], end="" if c["text"].endswith("\n") else "\n")

    print(f"report={rp}")
    print("source_modified=False")
    print("client_modified=False")
    print("material_render_source_audit_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(
            f"material_render_source_audit_check=FAIL: "
            f"{type(exc).__name__}: {exc}"
        )
        raise

#!/usr/bin/env python3
"""
PT SMD Diffuse Render Source Audit V1

Read-only audit for the exact semantics of smFCOLOR / smMATERIAL::Diffuse.

Goals:
  1) find the exact smFCOLOR declaration/member types;
  2) find ASE/material ingestion writes to Diffuse;
  3) distinguish obvious writes/copies from runtime reads/uses of Diffuse;
  4) find the character render color path around bCol / sDef_Color /
     SetD3DRendBuff* so a Diffuse -> final pixel link can be proven or rejected.

No source or client file is modified.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re

TEXT_EXTS = {
    ".h", ".hpp", ".hh", ".c", ".cc", ".cpp", ".cxx", ".inc", ".inl"
}

DEFAULT_SKIP_PARTS = {
    ".git", ".svn", ".hg",
    "dependency", "dependencies", "thirdparty", "third_party",
    "vendor", "vendors",
    "build", "builds", "bin", "obj",
    "node_modules",
}

TERMS = [
    "smFCOLOR",
    "Diffuse",
    "MATERIAL_DIFFUSE",
    "aseMaterial",
    "smMaterial",
    "smRendMaterial",
    "bCol",
    "sDef_Color",
    "SetD3DRendBuff",
    "SetD3DRendBuff2",
    "SetD3DRendBuff3",
    "D3DTA_DIFFUSE",
]

DIFFUSE_MEMBER_RE = re.compile(r"(?:\.|->)\s*Diffuse\b", re.I)
SMFCOLOR_DECL_RE = re.compile(
    r"\b(?:struct|class|typedef\s+struct)\s+smFCOLOR\b|\bsmFCOLOR\s*\{",
    re.I,
)

def read_text(path: pathlib.Path) -> str:
    raw = path.read_bytes()
    for enc in ("utf-8", "cp1252", "latin1"):
        try:
            return raw.decode(enc)
        except UnicodeDecodeError:
            pass
    return raw.decode("latin1", "replace")

def should_skip(rel: pathlib.Path, include_vendor: bool) -> bool:
    if include_vendor:
        return False
    return any(part.lower() in DEFAULT_SKIP_PARTS for part in rel.parts[:-1])

def make_context(lines, i, before=10, after=16):
    a = max(0, i - before)
    b = min(len(lines), i + after + 1)
    return {
        "match_line": i + 1,
        "start_line": a + 1,
        "end_line": b,
        "text": "".join(f"{j+1:6d}: {lines[j]}" for j in range(a, b)),
    }

def classify_diffuse_line(line: str) -> str:
    """
    Conservative lexical classification.
    Only labels obvious assignments with Diffuse on the LHS as WRITE_LHS.
    Everything else containing a Diffuse member is READ_OR_USE, except lines
    where the pattern cannot be located.
    """
    m = DIFFUSE_MEMBER_RE.search(line)
    if not m:
        return "NO_MEMBER_ACCESS"

    eq = line.find("=")
    if eq < 0:
        return "READ_OR_USE"

    # Ignore equality/comparison operators as assignment indicators where easy.
    if "==" in line or "!=" in line or ">=" in line or "<=" in line:
        return "READ_OR_USE"

    if m.start() < eq:
        return "WRITE_LHS"
    return "READ_OR_USE"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-root", required=True)
    ap.add_argument(
        "--report",
        default="logs/pt_smd_diffuse_render_source_audit_v1.json",
    )
    ap.add_argument(
        "--include-vendor",
        action="store_true",
        help="also scan Dependency/vendor/build trees (off by default)",
    )
    args = ap.parse_args()

    root = pathlib.Path(args.source_root).expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(root)

    term_counts = {t: 0 for t in TERMS}
    matched_files = []
    diffuse_member_hits = []
    smfcolor_decl_hits = []
    files_scanned = 0
    files_skipped_vendor = 0

    for p in sorted(root.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in TEXT_EXTS:
            continue

        rel = p.relative_to(root)
        if should_skip(rel, args.include_vendor):
            files_skipped_vendor += 1
            continue

        files_scanned += 1
        try:
            text = read_text(p)
        except OSError:
            continue

        low = text.lower()
        present = [t for t in TERMS if t.lower() in low]
        has_smf_decl = bool(SMFCOLOR_DECL_RE.search(text))
        has_diff_member = bool(DIFFUSE_MEMBER_RE.search(text))
        if not present and not has_smf_decl and not has_diff_member:
            continue

        lines = text.splitlines(keepends=True)
        hits = {}

        for term in present:
            arr = []
            needle = term.lower()
            for i, line in enumerate(lines):
                if needle in line.lower():
                    arr.append(make_context(lines, i))
            if arr:
                hits[term] = arr
                term_counts[term] += len(arr)

        for i, line in enumerate(lines):
            if SMFCOLOR_DECL_RE.search(line):
                smfcolor_decl_hits.append({
                    "path": str(rel).replace("\\", "/"),
                    **make_context(lines, i, before=14, after=28),
                })

            if DIFFUSE_MEMBER_RE.search(line):
                cls = classify_diffuse_line(line)
                diffuse_member_hits.append({
                    "path": str(rel).replace("\\", "/"),
                    "line": i + 1,
                    "classification": cls,
                    "source_line": line.rstrip("\r\n"),
                    **make_context(lines, i, before=8, after=12),
                })

        matched_files.append({
            "path": str(rel).replace("\\", "/"),
            "hits": hits,
        })

    write_hits = [x for x in diffuse_member_hits if x["classification"] == "WRITE_LHS"]
    read_hits = [x for x in diffuse_member_hits if x["classification"] == "READ_OR_USE"]

    result = {
        "tool": "PT_SMD_DIFFUSE_RENDER_SOURCE_AUDIT_V1",
        "source_root": str(root),
        "include_vendor": bool(args.include_vendor),
        "text_files_scanned": files_scanned,
        "vendor_or_build_files_skipped": files_skipped_vendor,
        "matched_file_count": len(matched_files),
        "term_counts": term_counts,
        "smfcolor_declaration_hit_count": len(smfcolor_decl_hits),
        "smfcolor_declaration_hits": smfcolor_decl_hits,
        "diffuse_member_hit_count": len(diffuse_member_hits),
        "diffuse_write_lhs_count": len(write_hits),
        "diffuse_read_or_use_count": len(read_hits),
        "diffuse_member_hits": diffuse_member_hits,
        "matched_files": matched_files,
        "source_modified": False,
        "client_modified": False,
        "diffuse_render_source_audit_check": "PASS",
    }

    rp = pathlib.Path(args.report).expanduser()
    if not rp.is_absolute():
        rp = pathlib.Path.cwd() / rp
    rp = rp.resolve()
    rp.parent.mkdir(parents=True, exist_ok=True)
    rp.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print("PT_SMD_DIFFUSE_RENDER_SOURCE_AUDIT_V1")
    print(f"source_root={root}")
    print(f"text_files_scanned={files_scanned}")
    print(f"vendor_or_build_files_skipped={files_skipped_vendor}")
    print(f"matched_file_count={len(matched_files)}")
    print(f"smfcolor_declaration_hit_count={len(smfcolor_decl_hits)}")
    print(f"diffuse_member_hit_count={len(diffuse_member_hits)}")
    print(f"diffuse_write_lhs_count={len(write_hits)}")
    print(f"diffuse_read_or_use_count={len(read_hits)}")

    print("\n=== SMFCOLOR DECLARATION CONTEXTS ===")
    if not smfcolor_decl_hits:
        print("NONE")
    for h in smfcolor_decl_hits[:12]:
        print(f"SOURCE_FILE {h['path']}")
        print(f"CONTEXT lines={h['start_line']}..{h['end_line']}")
        print(h["text"], end="" if h["text"].endswith("\n") else "\n")

    print("\n=== DIFFUSE MEMBER ACCESSES ===")
    if not diffuse_member_hits:
        print("NONE")
    for h in diffuse_member_hits[:80]:
        print(
            f"SOURCE_FILE {h['path']} line={h['line']} "
            f"class={h['classification']}"
        )
        print(f"  {h['source_line']}")

    priority_terms = [
        "Diffuse",
        "MATERIAL_DIFFUSE",
        "smMaterial",
        "smRendMaterial",
        "bCol",
        "sDef_Color",
        "SetD3DRendBuff",
        "D3DTA_DIFFUSE",
    ]

    print("\n=== RENDER/COLOR PRIORITY CONTEXTS ===")
    printed = 0
    for item in matched_files:
        interesting = [t for t in priority_terms if t in item["hits"]]
        if not interesting:
            continue
        print(f"SOURCE_FILE {item['path']}")
        for term in interesting:
            print(f"  TERM {term}: matches={len(item['hits'][term])}")
            for c in item["hits"][term][:5]:
                print(f"  CONTEXT lines={c['start_line']}..{c['end_line']}")
                print(c["text"], end="" if c["text"].endswith("\n") else "\n")
                printed += 1
                if printed >= 60:
                    break
            if printed >= 60:
                break
        if printed >= 60:
            break

    print(f"\nreport={rp}")
    print("source_modified=False")
    print("client_modified=False")
    print("diffuse_render_source_audit_check=PASS")

if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(
            f"diffuse_render_source_audit_check=FAIL: "
            f"{type(exc).__name__}: {exc}"
        )
        raise

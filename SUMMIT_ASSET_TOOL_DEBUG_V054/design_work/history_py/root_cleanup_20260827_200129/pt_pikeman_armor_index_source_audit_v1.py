#!/usr/bin/env python3
"""
PT Pikeman Armor Index Source Audit V1

Read-only source scanner.

Finds:
- playmodel.h / source files containing szArmorNumFile;
- the //PIKEMAN row and nearby Cxxx.ini entries;
- ArmorJobNum declarations/usages;
- SetPlayerArmor definition and calls;
- INVENTORY_POS_ARMOR armor-index calculation;
- sinITEM_MASK3 occurrences relevant to the calculation.

No source or client file is modified.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re


TEXT_EXTS = {
    ".h", ".hpp", ".hh", ".c", ".cc", ".cpp", ".cxx",
    ".txt", ".inc", ".ini"
}

TARGET_TERMS = [
    "szArmorNumFile",
    "//PIKEMAN",
    "ArmorJobNum",
    "SetPlayerArmor",
    "INVENTORY_POS_ARMOR",
    "sinITEM_MASK3",
]


def is_text_candidate(p: pathlib.Path) -> bool:
    return p.suffix.lower() in TEXT_EXTS


def read_text(p: pathlib.Path):
    raw = p.read_bytes()
    # Source tree is historical; try common encodings conservatively.
    for enc in ("utf-8", "cp1252", "latin1"):
        try:
            return raw.decode(enc)
        except UnicodeDecodeError:
            pass
    return raw.decode("latin1", "replace")


def contexts(lines, needle, before=12, after=36, case_sensitive=False):
    out = []
    n = needle if case_sensitive else needle.lower()
    for i, line in enumerate(lines):
        hay = line if case_sensitive else line.lower()
        if n in hay:
            a = max(0, i - before)
            b = min(len(lines), i + after + 1)
            out.append({
                "match_line": i + 1,
                "start_line": a + 1,
                "end_line": b,
                "text": "".join(
                    f"{j+1:6d}: {lines[j]}"
                    for j in range(a, b)
                ),
            })
    return out


def extract_pikeman_row_windows(lines):
    rows = []
    for i, line in enumerate(lines):
        if "pikeman" not in line.lower():
            continue
        # Prefer comment labels but retain nearby evidence either way.
        a = max(0, i - 4)
        b = min(len(lines), i + 45)
        block = "".join(lines[a:b])
        if "c001" in block.lower() or "szarmornumfile" in block.lower():
            rows.append({
                "match_line": i + 1,
                "start_line": a + 1,
                "end_line": b,
                "text": "".join(
                    f"{j+1:6d}: {lines[j]}"
                    for j in range(a, b)
                ),
            })
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-root", required=True)
    ap.add_argument(
        "--report",
        default="logs/pikeman_armor_index_source_audit_v1.json",
    )
    args = ap.parse_args()

    root = pathlib.Path(args.source_root).expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(root)

    scanned = 0
    matched_files = []
    all_pikeman_rows = []

    for p in sorted(root.rglob("*")):
        if not p.is_file() or not is_text_candidate(p):
            continue
        scanned += 1
        try:
            text = read_text(p)
        except OSError:
            continue

        low = text.lower()
        if not any(t.lower() in low for t in TARGET_TERMS):
            continue

        lines = text.splitlines(keepends=True)
        rel = str(p.relative_to(root)).replace("\\", "/")
        item = {
            "path": rel,
            "hits": {},
        }

        for term in TARGET_TERMS:
            c = contexts(lines, term)
            if c:
                item["hits"][term] = c

        pikeman_rows = extract_pikeman_row_windows(lines)
        if pikeman_rows:
            item["pikeman_row_windows"] = pikeman_rows
            for r in pikeman_rows:
                all_pikeman_rows.append({
                    "path": rel,
                    **r,
                })

        matched_files.append(item)

    # Derive only literal Cxxx tokens appearing in Pikeman windows.
    pikeman_tokens = []
    for row in all_pikeman_rows:
        toks = re.findall(r'(?i)\bC\d{3}(?:t)?\.ini\b', row["text"])
        for t in toks:
            normalized = t
            if normalized not in pikeman_tokens:
                pikeman_tokens.append(normalized)

    result = {
        "tool": "PT_PIKEMAN_ARMOR_INDEX_SOURCE_AUDIT_V1",
        "source_root": str(root),
        "text_files_scanned": scanned,
        "matched_file_count": len(matched_files),
        "pikeman_row_window_count": len(all_pikeman_rows),
        "pikeman_literal_config_tokens": pikeman_tokens,
        "pikeman_rows": all_pikeman_rows,
        "matched_files": matched_files,
        "source_modified": False,
        "client_modified": False,
        "armor_index_source_audit_check": "PASS",
    }

    report = pathlib.Path(args.report).expanduser()
    if not report.is_absolute():
        report = pathlib.Path.cwd() / report
    report = report.resolve()
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print("PT_PIKEMAN_ARMOR_INDEX_SOURCE_AUDIT_V1")
    print(f"source_root={root}")
    print(f"text_files_scanned={scanned}")
    print(f"matched_file_count={len(matched_files)}")
    print(f"pikeman_row_window_count={len(all_pikeman_rows)}")
    print(f"pikeman_literal_config_tokens={pikeman_tokens}")

    for row in all_pikeman_rows[:10]:
        print(f"PIKEMAN_ROW path={row['path']} lines={row['start_line']}..{row['end_line']}")
        print(row["text"], end="" if row["text"].endswith("\n") else "\n")

    for item in matched_files:
        interesting = [
            t for t in ("ArmorJobNum", "SetPlayerArmor", "INVENTORY_POS_ARMOR", "sinITEM_MASK3")
            if t in item["hits"]
        ]
        if not interesting:
            continue
        print(f"SOURCE_FILE {item['path']}")
        for term in interesting:
            print(f"  TERM {term}: matches={len(item['hits'][term])}")
            for c in item["hits"][term][:3]:
                print(f"  CONTEXT lines={c['start_line']}..{c['end_line']}")
                print(c["text"], end="" if c["text"].endswith("\n") else "\n")

    print(f"report={report}")
    print("source_modified=False")
    print("client_modified=False")
    print("armor_index_source_audit_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"armor_index_source_audit_check=FAIL: {type(exc).__name__}: {exc}")
        raise

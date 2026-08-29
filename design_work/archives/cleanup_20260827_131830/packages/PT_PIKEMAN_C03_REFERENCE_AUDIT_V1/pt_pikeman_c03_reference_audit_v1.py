#!/usr/bin/env python3
"""
PT Pikeman C03 Reference Audit V1

Read-only full-client reference scan.

Searches bytewise, case-insensitively, for:
  TbhC03
  TbmC03
  TblC03
  TmbC03.bmp

Purpose:
- distinguish SMD-embedded data from external runtime/config references;
- do not infer C03 meaning from filename alone;
- do not modify any file.

The scan is chunked and safe for binary files.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import re
from collections import defaultdict


DEFAULT_TOKENS = ["TbhC03", "TbmC03", "TblC03", "TmbC03.bmp"]


def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def printable_context(data: bytes, pos: int, token_len: int, radius=64):
    a = max(0, pos - radius)
    b = min(len(data), pos + token_len + radius)
    raw = data[a:b]
    return "".join(chr(c) if 32 <= c <= 126 else "." for c in raw)


def scan_file(path, needles, chunk_size=1024 * 1024):
    max_len = max(len(x) for x in needles)
    overlap = max_len - 1
    tail = b""
    absolute_base = 0
    hits = defaultdict(list)

    with path.open("rb") as f:
        while True:
            block = f.read(chunk_size)
            if not block:
                break
            data = tail + block
            low = data.lower()

            for token, needle in needles.items():
                start = 0
                while True:
                    pos = low.find(needle, start)
                    if pos < 0:
                        break

                    abs_pos = absolute_base - len(tail) + pos

                    # Avoid reporting an overlap hit twice.
                    if abs_pos >= 0:
                        hits[token].append({
                            "offset": abs_pos,
                            "context": printable_context(data, pos, len(needle)),
                        })
                    start = pos + 1

            absolute_base += len(block)
            tail = data[-overlap:] if overlap else b""

    # De-duplicate overlap positions.
    out = {}
    for token, rows in hits.items():
        seen = set()
        uniq = []
        for row in rows:
            if row["offset"] in seen:
                continue
            seen.add(row["offset"])
            uniq.append(row)
        out[token] = uniq
    return out


def category(path: pathlib.Path):
    ext = path.suffix.lower()
    if ext == ".inx":
        return "INX"
    if ext in {".ini", ".cfg", ".conf", ".txt", ".lst"}:
        return "CONFIG_TEXT"
    if ext == ".smd":
        return "SMD"
    if ext == ".smb":
        return "SMB"
    if ext in {".exe", ".dll"}:
        return "EXECUTABLE"
    if ext in {".bmp", ".tga", ".png", ".jpg", ".jpeg"}:
        return "IMAGE"
    return "OTHER"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--client-root", required=True)
    ap.add_argument("--token", action="append", default=[])
    ap.add_argument("--report")
    ap.add_argument(
        "--max-contexts-per-token-per-file",
        type=int,
        default=5,
    )
    args = ap.parse_args()

    root = pathlib.Path(args.client_root).expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(root)

    tokens = args.token or DEFAULT_TOKENS
    needles = {t: t.encode("ascii").lower() for t in tokens}

    file_count = 0
    scanned_bytes = 0
    matched_files = []

    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        file_count += 1
        try:
            size = path.stat().st_size
            scanned_bytes += size
            hits = scan_file(path, needles)
        except (OSError, PermissionError):
            continue

        if not hits:
            continue

        rel = str(path.relative_to(root)).replace("\\", "/")
        row = {
            "path": rel,
            "category": category(path),
            "size": size,
            "sha256": sha256_file(path),
            "hits": {},
        }
        for token, rows in hits.items():
            row["hits"][token] = {
                "count": len(rows),
                "samples": rows[:args.max_contexts_per_token_per_file],
            }
        matched_files.append(row)

    refs_by_token = {}
    for token in tokens:
        rows = []
        for f in matched_files:
            h = f["hits"].get(token)
            if h:
                rows.append({
                    "path": f["path"],
                    "category": f["category"],
                    "count": h["count"],
                })
        refs_by_token[token] = rows

    node_tokens = [x for x in tokens if x.lower() in {"tbhc03", "tbmc03", "tblc03"}]
    external_node_refs = []
    for token in node_tokens:
        for row in refs_by_token[token]:
            # The known containing model itself is not evidence of an external
            # selector/reference.
            if row["path"].lower() != "char/tmabcd/tmbc01.smd":
                external_node_refs.append({
                    "token": token,
                    **row,
                })

    texture_external_refs = [
        row for row in refs_by_token.get("TmbC03.bmp", [])
        if row["path"].lower() != "char/tmabcd/tmbc01.smd"
        and row["category"] != "IMAGE"
    ]

    if not external_node_refs:
        node_reference_status = "SMD_EMBEDDED_ONLY"
    else:
        node_reference_status = "EXTERNAL_REFERENCES_FOUND"

    result = {
        "tool": "PT_PIKEMAN_C03_REFERENCE_AUDIT_V1",
        "client_root": str(root),
        "tokens": tokens,
        "files_scanned": file_count,
        "bytes_scanned": scanned_bytes,
        "matched_file_count": len(matched_files),
        "references_by_token": refs_by_token,
        "external_node_references": external_node_refs,
        "external_texture_references": texture_external_refs,
        "node_reference_status": node_reference_status,
        "matched_files": matched_files,
        "client_modified": False,
        "c03_reference_audit_check": "PASS",
    }

    if args.report:
        report = pathlib.Path(args.report).expanduser()
        if not report.is_absolute():
            report = pathlib.Path.cwd() / report
        report = report.resolve()
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_text(
            json.dumps(result, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
    else:
        report = None

    print("PT_PIKEMAN_C03_REFERENCE_AUDIT_V1")
    print(f"files_scanned={file_count}")
    print(f"bytes_scanned={scanned_bytes}")
    print(f"matched_file_count={len(matched_files)}")

    for token in tokens:
        rows = refs_by_token[token]
        print(f"TOKEN {token!r}: file_refs={len(rows)}")
        for row in rows:
            print(
                f"  {row['category']} {row['path']} count={row['count']}"
            )

    print(f"external_node_reference_count={len(external_node_refs)}")
    for row in external_node_refs:
        print(
            f"EXTERNAL_NODE_REF token={row['token']} "
            f"category={row['category']} path={row['path']} count={row['count']}"
        )

    print(f"external_texture_reference_count={len(texture_external_refs)}")
    for row in texture_external_refs:
        print(
            f"EXTERNAL_TEXTURE_REF category={row['category']} "
            f"path={row['path']} count={row['count']}"
        )

    print(f"node_reference_status={node_reference_status}")
    if report:
        print(f"report={report}")
    print("client_modified=False")
    print("c03_reference_audit_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"c03_reference_audit_check=FAIL: {type(exc).__name__}: {exc}")
        raise

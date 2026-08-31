#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import os
import re
import sys
from collections import Counter
from pathlib import Path

IMAGE_EXTS = {".bmp", ".png", ".tga", ".jpg", ".jpeg", ".dds", ".gif", ".pcx"}
SOURCE_EXTS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".rc"}
RUNTIME_TEXT_EXTS = {".ini", ".txt", ".cfg", ".lst", ".lua", ".xml", ".json", ".csv", ".zhoon"}
DYNAMIC_RE = re.compile(r'%(?:[-+0 #]*\d*(?:\.\d+)?[hlLzjt]*[a-zA-Z])|[*?]|\{[^}]*\}')
IMAGE_PATH_RE = re.compile(
    r'(?i)(?:[A-Za-z0-9_. ~()@+\-\[\]{}%*?/\\:]+?\.(?:bmp|png|tga|jpe?g|dds|gif|pcx))'
)

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def norm_ref(s: str) -> str:
    s = s.strip().strip('"\'')
    s = s.replace("\\\\", "/").replace("\\", "/")
    while "//" in s:
        s = s.replace("//", "/")
    while s.startswith("./"):
        s = s[2:]
    return s.strip()

def safe_rel(ref: str) -> str | None:
    ref = norm_ref(ref)
    # Drive/URL-like references are not runtime-root-relative assets.
    if re.match(r'^[A-Za-z]:/', ref) or "://" in ref:
        return None
    parts = []
    for part in ref.split("/"):
        if part in ("", "."):
            continue
        if part == "..":
            if not parts:
                return None
            parts.pop()
        else:
            parts.append(part)
    return "/".join(parts)

def resolve_casefold(root: Path, rel: str):
    cur = root
    canonical = []
    for comp in rel.split("/"):
        if not cur.is_dir():
            return ("MISSING", None)
        exact = cur / comp
        if exact.exists():
            cur = exact
            canonical.append(comp)
            continue
        matches = []
        try:
            for ent in cur.iterdir():
                if ent.name.casefold() == comp.casefold():
                    matches.append(ent)
        except OSError:
            return ("MISSING", None)
        if len(matches) == 0:
            return ("MISSING", None)
        if len(matches) > 1:
            return ("AMBIGUOUS", None)
        cur = matches[0]
        canonical.append(matches[0].name)
    if not cur.is_file():
        return ("MISSING", None)
    canon = "/".join(canonical)
    return ("EXACT" if canon == rel else "CASE_MISMATCH", canon)

def strip_cpp_comments_keep_strings(data: bytes) -> bytes:
    out = bytearray()
    i = 0
    n = len(data)
    state = "code"
    quote = None
    while i < n:
        c = data[i]
        nxt = data[i+1] if i+1 < n else None
        if state == "code":
            if c == ord('/') and nxt == ord('/'):
                out.extend(b"  ")
                i += 2
                state = "line"
                continue
            if c == ord('/') and nxt == ord('*'):
                out.extend(b"  ")
                i += 2
                state = "block"
                continue
            if c in (ord('"'), ord("'")):
                quote = c
                out.append(c)
                i += 1
                state = "string"
                continue
            out.append(c)
            i += 1
        elif state == "line":
            if c in (10, 13):
                out.append(c)
                i += 1
                state = "code"
            else:
                out.append(32)
                i += 1
        elif state == "block":
            if c == ord('*') and nxt == ord('/'):
                out.extend(b"  ")
                i += 2
                state = "code"
            else:
                out.append(c if c in (10,13) else 32)
                i += 1
        else:  # string/char
            out.append(c)
            i += 1
            if c == ord('\\') and i < n:
                out.append(data[i])
                i += 1
            elif c == quote:
                state = "code"
    return bytes(out)

def cpp_string_refs(path: Path):
    raw = path.read_bytes()
    clean = strip_cpp_comments_keep_strings(raw)
    # latin1 preserves byte positions and legacy source without decoding failures.
    text = clean.decode("latin1", errors="ignore")
    for m in re.finditer(r'"((?:\\.|[^"\\])*)"', text):
        literal = m.group(1)
        # Keep slash escapes as separators; do not interpret arbitrary C escapes.
        candidate = literal.replace(r"\\", "\\")
        if Path(norm_ref(candidate)).suffix.lower() not in IMAGE_EXTS:
            continue
        line = text.count("\n", 0, m.start()) + 1
        yield line, candidate

def runtime_text_refs(path: Path):
    try:
        data = path.read_bytes()
    except OSError:
        return
    if b"\x00" in data[:8192]:
        return
    text = data.decode("latin1", errors="ignore")
    for ln, line in enumerate(text.splitlines(), 1):
        for m in IMAGE_PATH_RE.finditer(line):
            candidate = m.group(0).strip()
            # trim common config syntax prefixes accidentally captured
            candidate = re.sub(r'^[^"=:\t]*[=:\t]\s*', '', candidate)
            if Path(norm_ref(candidate)).suffix.lower() in IMAGE_EXTS:
                yield ln, candidate

def historical_candidates(history_roots, rel: str):
    out = []
    for label, root in history_roots:
        status, canon = resolve_casefold(root, rel)
        if status in ("EXACT", "CASE_MISMATCH") and canon:
            p = root / Path(canon)
            out.append((label, p, canon, sha256(p)))
    return out

def basename_candidates(history_roots, basename: str, limit=30):
    hits = []
    folded = basename.casefold()
    for label, root in history_roots:
        if not root.is_dir():
            continue
        for dirpath, _, filenames in os.walk(root):
            for fn in filenames:
                if fn.casefold() == folded:
                    p = Path(dirpath) / fn
                    hits.append((label, p, p.relative_to(root).as_posix(), sha256(p)))
                    if len(hits) >= limit:
                        return hits
    return hits

def main():
    ap = argparse.ArgumentParser(description="Audit current client image references and historical restore candidates.")
    ap.add_argument("repo", nargs="?", default=".", help="Summit repo root")
    ap.add_argument("--no-runtime-text", action="store_true", help="Do not scan client runtime text/config files")
    ap.add_argument("--basename-search", action="store_true", help="For missing static refs, also search historical roots by basename")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    if not client.is_dir():
        raise SystemExit(f"ERRO: client runtime não encontrado: {client}")

    outdir = repo / "bugfixes" / "058_asset_reconciliation" / "evidence" / "image_audit"
    outdir.mkdir(parents=True, exist_ok=True)

    default_history = [
        ("summit_archive", Path.home() / "armazem/priston/Source Summit Tale"),
        ("pristontale_master", Path.home() / "armazem/priston/pristontale-1-master/bin/Client"),
        ("priston_main", Path.home() / "armazem/priston/drive-download-20260818T103311Z-1-002/priston-main"),
        ("fpt_rpt", Path.home() / "armazem/priston/drive-download-20260818T103311Z-1-002/fPT-rPT-Project-main"),
        ("exmachina", Path.home() / "armazem/priston/ExMachina"),
        ("server_runtime", repo / "Servidor"),
    ]
    history = [(label, p.resolve()) for label, p in default_history if p.is_dir()]

    active_cpp = set()
    cpp_list = repo / "SourceCode" / "cpp_files_client_i686.txt"
    if cpp_list.is_file():
        for raw in cpp_list.read_text(encoding="utf-8", errors="ignore").splitlines():
            raw = raw.strip()
            if not raw:
                continue
            p = (repo / "SourceCode" / raw).resolve() if not Path(raw).is_absolute() else Path(raw).resolve()
            active_cpp.add(p)

    scan_roots = [
        repo / "SourceCode" / "SrcGame",
        repo / "SourceCode" / "Shared",
    ]

    refs = []
    seen = set()

    for root in scan_roots:
        if not root.is_dir():
            continue
        for p in root.rglob("*"):
            if not p.is_file() or p.suffix.lower() not in SOURCE_EXTS:
                continue
            source_class = "ACTIVE_CPP" if p.resolve() in active_cpp else ("HEADER_OR_OTHER_SOURCE" if p.suffix.lower() in {".h",".hh",".hpp"} else "OTHER_SOURCE")
            try:
                iterable = cpp_string_refs(p)
                for line, literal in iterable:
                    key = ("source", str(p), line, literal)
                    if key in seen:
                        continue
                    seen.add(key)
                    refs.append(("source", source_class, p, line, literal))
            except Exception as e:
                print(f"WARN source {p}: {e}", file=sys.stderr)

    if not args.no_runtime_text:
        for p in client.rglob("*"):
            if not p.is_file() or p.suffix.lower() not in RUNTIME_TEXT_EXTS:
                continue
            try:
                for line, literal in runtime_text_refs(p):
                    key = ("client_text", str(p), line, literal)
                    if key in seen:
                        continue
                    seen.add(key)
                    refs.append(("client_text", "RUNTIME_TEXT", p, line, literal))
            except Exception as e:
                print(f"WARN runtime text {p}: {e}", file=sys.stderr)

    rows = []
    counts = Counter()
    safe_restore_count = 0

    for origin, source_class, source_file, line, literal in refs:
        normalized = norm_ref(literal)
        rel = safe_rel(normalized)
        dynamic = bool(DYNAMIC_RE.search(normalized))

        if rel is None:
            status = "NON_RUNTIME_RELATIVE"
            canon = ""
        elif dynamic:
            status = "DYNAMIC"
            canon = ""
        else:
            status, canon0 = resolve_casefold(client, rel)
            canon = canon0 or ""

        candidates = []
        basename_hits = []
        safe_restore = "0"
        restore_reason = ""

        if status == "MISSING" and rel:
            candidates = historical_candidates(history, rel)
            if candidates:
                trusted = [x for x in candidates if x[0] != "server_runtime"]
                trusted_hashes = {x[3] for x in trusted}
                all_hashes = {x[3] for x in candidates}
                if trusted and len(trusted_hashes) == 1:
                    safe_restore = "1"
                    safe_restore_count += 1
                    if len(all_hashes) == 1:
                        restore_reason = "TRUSTED_SAME_RELATIVE_PATH_IDENTICAL"
                    else:
                        restore_reason = "TRUSTED_IDENTICAL_SERVER_DIFFERS"
                elif trusted and len(trusted_hashes) > 1:
                    restore_reason = "TRUSTED_HISTORICAL_CONTENT_CONFLICT"
                elif not trusted:
                    restore_reason = "SERVER_ONLY_EVIDENCE_NOT_SAFE"
            elif args.basename_search:
                basename_hits = basename_candidates(history, Path(rel).name)
                if basename_hits:
                    hashes = {x[3] for x in basename_hits}
                    rels = {x[2].casefold() for x in basename_hits}
                    restore_reason = "BASENAME_ONLY_IDENTICAL" if len(hashes) == 1 else "BASENAME_ONLY_CONFLICT"

        counts[status] += 1
        rows.append({
            "origin": origin,
            "source_class": source_class,
            "source_file": str(source_file.relative_to(repo)) if source_file.is_relative_to(repo) else str(source_file),
            "line": line,
            "literal": literal,
            "normalized": normalized,
            "status": status,
            "runtime_canonical": canon,
            "safe_restore": safe_restore,
            "restore_reason": restore_reason,
            "historical_same_rel_count": len(candidates),
            "historical_same_rel_sources": " | ".join(f"{lab}:{relp}:{h}" for lab, _, relp, h in candidates),
            "basename_hit_count": len(basename_hits),
            "basename_hits": " | ".join(f"{lab}:{relp}:{h}" for lab, _, relp, h in basename_hits),
        })

    report = outdir / "image_references.tsv"
    fields = list(rows[0].keys()) if rows else [
        "origin","source_class","source_file","line","literal","normalized","status","runtime_canonical",
        "safe_restore","restore_reason","historical_same_rel_count","historical_same_rel_sources",
        "basename_hit_count","basename_hits"
    ]
    with report.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)

    missing_report = outdir / "missing_static_images.tsv"
    with missing_report.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(r for r in rows if r["status"] == "MISSING")

    safe_report = outdir / "safe_restore_candidates.tsv"
    with safe_report.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(r for r in rows if r["safe_restore"] == "1")

    case_report = outdir / "case_mismatches.tsv"
    with case_report.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(r for r in rows if r["status"] == "CASE_MISMATCH")

    summary = outdir / "summary.txt"
    with summary.open("w", encoding="utf-8") as f:
        f.write(f"repo={repo}\nclient={client}\n")
        f.write("history_roots:\n")
        for label, root in history:
            f.write(f"  {label}={root}\n")
        f.write("\nSTATUS\n")
        for k in sorted(counts):
            f.write(f"{k}={counts[k]}\n")
        f.write(f"\nSAFE_RESTORE_CANDIDATES={safe_restore_count}\n")
        f.write(f"TOTAL_REFERENCES={len(rows)}\n")

    print("=== IMAGE ASSET AUDIT 058 ===")
    print(f"client: {client}")
    print(f"references: {len(rows)}")
    for k in sorted(counts):
        print(f"{k}={counts[k]}")
    print(f"SAFE_RESTORE_CANDIDATES={safe_restore_count}")
    print(f"report: {report}")
    print(f"missing: {missing_report}")
    print(f"safe: {safe_report}")
    print(f"summary: {summary}")

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import re
from collections import Counter
from pathlib import Path

IMAGE_EXTS = {".bmp", ".png", ".tga", ".jpg", ".jpeg", ".dds", ".gif", ".pcx"}
SOURCE_EXTS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".rc"}
RUNTIME_TEXT_EXTS = {".ini", ".txt", ".cfg", ".lst", ".lua", ".xml", ".json", ".csv", ".zhoon"}

# These prefixes are known client-root-relative namespaces.
ROOT_PREFIXES = {
    "image", "game", "effect", "field", "char", "sound", "music",
    "startimage", "save", "data", "userdata", "clan", "patch"
}

DYNAMIC_RE = re.compile(r'%(?:[-+0 #]*\d*(?:\.\d+)?[hlLzjt]*[a-zA-Z])|[*?]|\{[^}]*\}')
IMAGE_PATH_RE = re.compile(
    r'(?i)(?:[A-Za-z0-9_. ~()@+\-\[\]{}%*?/\\:]+?\.(?:bmp|png|tga|jpe?g|dds|gif|pcx))'
)

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
                out.extend(b"  "); i += 2; state = "line"; continue
            if c == ord('/') and nxt == ord('*'):
                out.extend(b"  "); i += 2; state = "block"; continue
            if c in (ord('"'), ord("'")):
                quote = c; out.append(c); i += 1; state = "string"; continue
            out.append(c); i += 1
        elif state == "line":
            if c in (10, 13):
                out.append(c); i += 1; state = "code"
            else:
                out.append(32); i += 1
        elif state == "block":
            if c == ord('*') and nxt == ord('/'):
                out.extend(b"  "); i += 2; state = "code"
            else:
                out.append(c if c in (10,13) else 32); i += 1
        else:
            out.append(c); i += 1
            if c == ord('\\') and i < n:
                out.append(data[i]); i += 1
            elif c == quote:
                state = "code"
    return bytes(out)

def cpp_string_refs(path: Path):
    raw = path.read_bytes()
    clean = strip_cpp_comments_keep_strings(raw)
    text = clean.decode("latin1", errors="ignore")
    for m in re.finditer(r'"((?:\\.|[^"\\])*)"', text):
        literal = m.group(1)
        candidate = literal.replace(r"\\", "\\")
        if Path(norm_ref(candidate)).suffix.lower() not in IMAGE_EXTS:
            continue
        line = text.count("\n", 0, m.start()) + 1
        yield line, candidate

def runtime_text_refs(path: Path):
    data = path.read_bytes()
    if b"\x00" in data[:8192]:
        return
    text = data.decode("latin1", errors="ignore")
    for ln, line in enumerate(text.splitlines(), 1):
        for m in IMAGE_PATH_RE.finditer(line):
            candidate = m.group(0).strip()
            candidate = re.sub(r'^[^"=:\t]*[=:\t]\s*', '', candidate)
            if Path(norm_ref(candidate)).suffix.lower() in IMAGE_EXTS:
                yield ln, candidate

def first_component(rel: str) -> str:
    return rel.split("/", 1)[0].casefold() if rel else ""

def runtime_relative_resolution(client: Path, source_file: Path, rel: str):
    """
    Returns:
      status, resolved_client_rel, canonical, resolution

    Absolute-ish/root namespace refs resolve against client root.
    Other refs are tried relative to the containing runtime text file first,
    then client root. If neither exists, they are RELATIVE_UNRESOLVED instead
    of falsely becoming root-level MISSING.
    """
    if first_component(rel) in ROOT_PREFIXES:
        st, canon = resolve_casefold(client, rel)
        return st, rel, canon or "", "CLIENT_ROOT"

    parent_rel = source_file.parent.relative_to(client).as_posix()
    parent_target = f"{parent_rel}/{rel}" if parent_rel != "." else rel

    pst, pcanon = resolve_casefold(client, parent_target)
    rst, rcanon = resolve_casefold(client, rel)

    parent_exists = pst in ("EXACT", "CASE_MISMATCH")
    root_exists = rst in ("EXACT", "CASE_MISMATCH")

    if parent_exists and not root_exists:
        return pst, parent_target, pcanon or "", "CONTAINING_DIR"
    if root_exists and not parent_exists:
        return rst, rel, rcanon or "", "CLIENT_ROOT_FALLBACK"
    if parent_exists and root_exists:
        # Prefer containing-directory semantics for relative runtime text.
        # Record ambiguity so it can be reviewed if the two paths differ.
        if (pcanon or parent_target).casefold() != (rcanon or rel).casefold():
            return "AMBIGUOUS_RESOLUTION", parent_target, pcanon or "", "BOTH_EXIST"
        return pst, parent_target, pcanon or "", "CONTAINING_DIR"

    return "RELATIVE_UNRESOLVED", parent_target, "", "CONTAINING_DIR_OR_ROOT_NOT_FOUND"

def main():
    ap = argparse.ArgumentParser(description="Audit image references with runtime relative-path awareness.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("--no-runtime-text", action="store_true")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    if not client.is_dir():
        raise SystemExit(f"ERRO: client runtime não encontrado: {client}")

    outdir = repo / "bugfixes" / "058_asset_reconciliation" / "evidence" / "image_audit_v3"
    outdir.mkdir(parents=True, exist_ok=True)

    active_cpp = set()
    cpp_list = repo / "SourceCode" / "cpp_files_client_i686.txt"
    if cpp_list.is_file():
        for raw in cpp_list.read_text(encoding="utf-8", errors="ignore").splitlines():
            raw = raw.strip()
            if not raw:
                continue
            p = (repo / "SourceCode" / raw).resolve() if not Path(raw).is_absolute() else Path(raw).resolve()
            active_cpp.add(p)

    refs = []
    seen = set()
    for root in [repo / "SourceCode" / "SrcGame", repo / "SourceCode" / "Shared"]:
        if not root.is_dir():
            continue
        for p in root.rglob("*"):
            if not p.is_file() or p.suffix.lower() not in SOURCE_EXTS:
                continue
            source_class = "ACTIVE_CPP" if p.resolve() in active_cpp else (
                "HEADER_OR_OTHER_SOURCE" if p.suffix.lower() in {".h",".hh",".hpp"} else "OTHER_SOURCE"
            )
            for line, literal in cpp_string_refs(p):
                key = ("source", str(p), line, literal)
                if key not in seen:
                    seen.add(key)
                    refs.append(("source", source_class, p, line, literal))

    if not args.no_runtime_text:
        for p in client.rglob("*"):
            if not p.is_file() or p.suffix.lower() not in RUNTIME_TEXT_EXTS:
                continue

            # Diagnostic output is evidence about loads, not an input that causes loads.
            # Exclude only explicit log/debug/trace artifacts; keep real .ini/.lua/.txt configs.
            rel_low = p.relative_to(client).as_posix().casefold()
            name_low = p.name.casefold()
            parts_low = [x.casefold() for x in p.relative_to(client).parts]
            is_diagnostic = (
                "logs" in parts_low
                or "log" in parts_low
                or name_low.endswith(".log")
                or "debug" in name_low
                or "trace" in name_low
            )
            if is_diagnostic:
                continue

            try:
                for line, literal in runtime_text_refs(p):
                    key = ("client_text", str(p), line, literal)
                    if key not in seen:
                        seen.add(key)
                        refs.append(("client_text", "RUNTIME_TEXT", p, line, literal))
            except Exception as e:
                print(f"WARN runtime text {p}: {e}")

    rows = []
    counts = Counter()

    for origin, source_class, source_file, line, literal in refs:
        normalized = norm_ref(literal)
        rel = safe_rel(normalized)
        dynamic = bool(DYNAMIC_RE.search(normalized))

        resolved_rel = rel or ""
        canonical = ""
        resolution = ""

        if rel is None:
            status = "NON_RUNTIME_RELATIVE"
            resolution = "INVALID_OR_EXTERNAL"
        elif dynamic:
            status = "DYNAMIC"
            resolution = "DYNAMIC"
        elif origin == "client_text":
            status, resolved_rel, canonical, resolution = runtime_relative_resolution(
                client, source_file, rel
            )
        else:
            status, canon = resolve_casefold(client, rel)
            canonical = canon or ""
            resolution = "CLIENT_ROOT"

        counts[status] += 1
        rows.append({
            "origin": origin,
            "source_class": source_class,
            "source_file": str(source_file.relative_to(repo)) if source_file.is_relative_to(repo) else str(source_file),
            "line": line,
            "literal": literal,
            "normalized": normalized,
            "resolved_client_rel": resolved_rel,
            "resolution": resolution,
            "status": status,
            "runtime_canonical": canonical,
        })

    fields = [
        "origin","source_class","source_file","line","literal","normalized",
        "resolved_client_rel","resolution","status","runtime_canonical"
    ]

    def write(name, pred):
        p = outdir / name
        with p.open("w", encoding="utf-8", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
            w.writeheader()
            w.writerows(r for r in rows if pred(r))
        return p

    report = write("image_references.tsv", lambda r: True)
    missing = write("missing_static_images.tsv", lambda r: r["status"] == "MISSING")
    rel_unresolved = write("runtime_relative_unresolved.tsv", lambda r: r["status"] == "RELATIVE_UNRESOLVED")
    case = write("case_mismatches.tsv", lambda r: r["status"] == "CASE_MISMATCH")
    amb = write("ambiguous_resolution.tsv", lambda r: r["status"] == "AMBIGUOUS_RESOLUTION")

    summary = outdir / "summary.txt"
    with summary.open("w", encoding="utf-8") as f:
        for k in sorted(counts):
            f.write(f"{k}={counts[k]}\n")
        f.write(f"TOTAL_REFERENCES={len(rows)}\n")

    print("=== IMAGE ASSET AUDIT 058 V3 ===")
    print(f"client: {client}")
    print(f"references: {len(rows)}")
    for k in sorted(counts):
        print(f"{k}={counts[k]}")
    print(f"report: {report}")
    print(f"missing: {missing}")
    print(f"relative_unresolved: {rel_unresolved}")
    print(f"case: {case}")
    print(f"ambiguous: {amb}")
    print(f"summary: {summary}")

if __name__ == "__main__":
    main()

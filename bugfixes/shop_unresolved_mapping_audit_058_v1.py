#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import re
from collections import defaultdict, Counter
from pathlib import Path

MATCH_RE = re.compile(r'\[MATCH\]\s+wanted=(0x[0-9A-Fa-f]+).*?Path="([^"]+)".*?Cat="([^"]+)"')
FAIL_RE = re.compile(r'\[IMAGE_FAIL\].*?path="([^"]+)"', re.I)

SEARCH_ROOTS = [
    ("SourceCode", "SourceCode"),
    ("Servidor", "Servidor"),
    ("ClientRuntime", "Summit Tale Client Full/cliente"),
]
EXTERNAL_ROOTS = [
    ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
    ("priston-official", Path("/home/gabriel/Área de Trabalho/priston-official")),
]

TEXT_EXTS = {
    ".c",".cc",".cpp",".cxx",".h",".hh",".hpp",".hxx",
    ".ini",".cfg",".conf",".txt",".csv",".tsv",".xml",".json",
    ".lua",".sql",".lst",".log",".md",".dat",".def",".rc"
}
SKIP_DIRS = {
    ".git","ccache","dist_mingw_client_i686","dist_mingw_server_i686",
    "logs","log","__pycache__"
}
MAX_FILE_SIZE = 8 * 1024 * 1024

def clean(s: str) -> str:
    s = s.replace("\\", "/")
    while "//" in s:
        s = s.replace("//", "/")
    return s.lstrip("/")

def parse_failures(log: Path, client: Path):
    last = ("","","")
    out = []
    seen = set()
    for line in log.read_text(errors="replace").splitlines():
        m = MATCH_RE.search(line)
        if m:
            last = (m.group(1).upper(), m.group(2), m.group(3))
        f = FAIL_RE.search(line)
        if not f:
            continue
        rel = clean(f.group(1))
        if (client / rel).is_file():
            continue
        key = rel.casefold()
        if key in seen:
            continue
        seen.add(key)
        code, catalog_path, cat = last
        out.append({
            "code_hex": code,
            "code_decimal": str(int(code, 16)) if code else "",
            "catalog_path": catalog_path,
            "cat": cat,
            "target_rel": rel,
            "target_stem": Path(rel).stem,
        })
    return out

def iter_text_files(root: Path):
    if not root.is_dir():
        return
    for p in root.rglob("*"):
        if not p.is_file() or p.is_symlink():
            continue
        if any(part.casefold() in SKIP_DIRS for part in p.parts):
            continue
        if p.suffix.casefold() not in TEXT_EXTS:
            continue
        try:
            if p.stat().st_size > MAX_FILE_SIZE:
                continue
        except OSError:
            continue
        yield p

def safe_lines(p: Path):
    try:
        data = p.read_bytes()
    except OSError:
        return []
    if b"\x00" in data[:4096]:
        return []
    text = data.decode("utf-8", errors="replace")
    return text.splitlines()

def scan_root(label: str, root: Path, targets, rows, file_name_rows):
    print(f"SCAN {label}: {root}")
    token_map = defaultdict(list)

    for t in targets:
        cat = t["cat"]
        stem = t["target_stem"]
        code = t["code_hex"]
        dec = t["code_decimal"]

        toks = []
        if cat:
            toks.append(("CAT", cat.casefold()))
        if stem:
            toks.append(("STEM", stem.casefold()))
        if code:
            toks.append(("HEX", code.casefold()))
            toks.append(("HEX_NOPREFIX", code[2:].lstrip("0").casefold() or "0"))
        if dec:
            toks.append(("DECIMAL", dec.casefold()))
        token_map[t["target_rel"]].extend(toks)

    for p in iter_text_files(root):
        lines = safe_lines(p)
        if not lines:
            continue
        rel = p.relative_to(root).as_posix()
        basename_cf = p.name.casefold()

        # filename evidence
        for t in targets:
            names = [t["cat"], t["target_stem"]]
            for tok in names:
                if tok and tok.casefold() in basename_cf:
                    file_name_rows.append({
                        "target_rel": t["target_rel"],
                        "code_hex": t["code_hex"],
                        "cat": t["cat"],
                        "root": label,
                        "file": rel,
                        "match_kind": "FILENAME",
                    })
                    break

        for lineno, line in enumerate(lines, 1):
            cf = line.casefold()
            for t in targets:
                matched = []
                for kind, tok in token_map[t["target_rel"]]:
                    if not tok:
                        continue
                    if kind == "DECIMAL":
                        # Avoid accidental substring matches in arbitrary numbers.
                        if re.search(rf'(?<!\d){re.escape(tok)}(?!\d)', cf):
                            matched.append(kind)
                    elif kind == "HEX_NOPREFIX":
                        # Only useful if at least 5 hex chars and bounded.
                        if len(tok) >= 5 and re.search(rf'(?<![0-9a-f]){re.escape(tok)}(?![0-9a-f])', cf):
                            matched.append(kind)
                    else:
                        if tok in cf:
                            matched.append(kind)

                if matched:
                    rows.append({
                        "target_rel": t["target_rel"],
                        "code_hex": t["code_hex"],
                        "cat": t["cat"],
                        "catalog_path": t["catalog_path"],
                        "root": label,
                        "file": rel,
                        "line": lineno,
                        "match_kind": ",".join(sorted(set(matched))),
                        "text": line.strip()[:800],
                    })

def main():
    ap = argparse.ArgumentParser(description="Cross-reference unresolved ItemShop/TimeShop runtime item codes.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("--log", default="")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    log = Path(args.log).expanduser().resolve() if args.log else client / "LoadItemImage_debug.txt"
    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/shop_unresolved_mapping"
    outdir.mkdir(parents=True, exist_ok=True)

    if not log.is_file():
        raise SystemExit(f"ERRO: log ausente: {log}")

    targets = parse_failures(log, client)
    print("=== SHOP UNRESOLVED MAPPING AUDIT 058 ===")
    print(f"runtime_unresolved_targets={len(targets)}")

    # Save target list.
    target_fields = ["code_hex","code_decimal","catalog_path","cat","target_rel","target_stem"]
    with (outdir / "targets.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=target_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(targets)

    rows = []
    filename_rows = []

    for label, rel in SEARCH_ROOTS:
        scan_root(label, repo / rel, targets, rows, filename_rows)

    for label, root in EXTERNAL_ROOTS:
        scan_root(label, root, targets, rows, filename_rows)

    hit_fields = [
        "target_rel","code_hex","cat","catalog_path","root","file",
        "line","match_kind","text"
    ]
    with (outdir / "text_hits.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=hit_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)

    fn_fields = ["target_rel","code_hex","cat","root","file","match_kind"]
    with (outdir / "filename_hits.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fn_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(filename_rows)

    by_target = defaultdict(list)
    for r in rows:
        by_target[r["target_rel"]].append(r)

    summary_rows = []
    for t in targets:
        hits = by_target.get(t["target_rel"], [])
        roots = sorted({h["root"] for h in hits})
        files = sorted({f"{h['root']}:{h['file']}" for h in hits})
        summary_rows.append({
            **t,
            "text_hit_count": len(hits),
            "hit_roots": ";".join(roots),
            "hit_files": ";".join(files[:20]),
        })

    sum_fields = target_fields + ["text_hit_count","hit_roots","hit_files"]
    with (outdir / "target_summary.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=sum_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(summary_rows)

    counts = Counter()
    for r in summary_rows:
        if r["text_hit_count"] == 0:
            counts["NO_TEXT_EVIDENCE"] += 1
        elif "SourceCode" in r["hit_roots"] or "Servidor" in r["hit_roots"]:
            counts["SOURCE_OR_SERVER_EVIDENCE"] += 1
        else:
            counts["RUNTIME_OR_HISTORICAL_TEXT_ONLY"] += 1

    print(f"TEXT_HITS={len(rows)}")
    print(f"FILENAME_HITS={len(filename_rows)}")
    for k in sorted(counts):
        print(f"{k}={counts[k]}")
    print(f"OUT={outdir}")

    print("\n=== TARGETS WITH SOURCE/SERVER EVIDENCE ===")
    shown = 0
    for r in summary_rows:
        if "SourceCode" in r["hit_roots"] or "Servidor" in r["hit_roots"]:
            print(f"{r['code_hex']:10s} {r['cat']:8s} hits={r['text_hit_count']:3d} {r['target_rel']}")
            print(f"  roots={r['hit_roots']}")
            shown += 1
    if not shown:
        print("(none)")

if __name__ == "__main__":
    main()

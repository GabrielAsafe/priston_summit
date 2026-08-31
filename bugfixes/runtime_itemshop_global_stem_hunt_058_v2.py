#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import re
from collections import Counter, defaultdict
from pathlib import Path

ROOTS = [
    ("Summit-current", None),
    ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
    ("priston-official", Path("/home/gabriel/Área de Trabalho/priston-official")),
]
EXTS = {".bmp", ".tga", ".dds", ".png", ".jpg", ".jpeg"}
FAIL_RE = re.compile(r'\[IMAGE_FAIL\].*?path="([^"]+)"', re.I)
MATCH_RE = re.compile(r'\[MATCH\]\s+wanted=(0x[0-9A-Fa-f]+).*?Path="([^"]+)".*?Cat="([^"]+)"')

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def clean(s: str) -> str:
    s = s.replace("\\", "/")
    while "//" in s:
        s = s.replace("//", "/")
    return s.lstrip("/")

def parse_failures(log: Path, client: Path):
    last = ("", "", "")
    out = []
    seen = set()
    for line in log.read_text(errors="replace").splitlines():
        m = MATCH_RE.search(line)
        if m:
            last = (m.group(1), m.group(2), m.group(3))
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
        out.append({
            "wanted_code": last[0],
            "catalog_path": last[1],
            "cat": last[2],
            "target_rel": rel,
            "target_stem": Path(rel).stem,
            "target_ext": Path(rel).suffix,
        })
    return out

def build_stem_index(client: Path):
    idx = defaultdict(list)
    roots = []
    for name, root in ROOTS:
        if name == "Summit-current":
            root = client
        if not root or not root.is_dir():
            continue
        roots.append((name, root))
        print(f"INDEX {name}: {root}")
        for p in root.rglob("*"):
            if not p.is_file() or p.is_symlink():
                continue
            if p.suffix.casefold() not in EXTS:
                continue
            idx[p.stem.casefold()].append((name, root, p))
    return idx

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("--log", default="")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    log = Path(args.log).expanduser().resolve() if args.log else client / "LoadItemImage_debug.txt"
    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/runtime_itemshop_global_stem"
    outdir.mkdir(parents=True, exist_ok=True)

    if not log.is_file():
        raise SystemExit(f"ERRO: log ausente: {log}")

    failures = parse_failures(log, client)
    idx = build_stem_index(client)

    rows = []
    counts = Counter()

    for f in failures:
        cands = idx.get(f["target_stem"].casefold(), [])
        # Exclude the requested target itself if a weird case-resolved duplicate appears.
        filtered = []
        for root_name, root, p in cands:
            try:
                rel = p.relative_to(root).as_posix()
            except ValueError:
                rel = str(p)
            if root_name == "Summit-current" and rel.casefold() == f["target_rel"].casefold():
                continue
            filtered.append((root_name, root, p, rel))

        if not filtered:
            cls = "UNRESOLVED_STEM"
            counts[cls] += 1
            rows.append({
                **f,
                "classification": cls,
                "source_root": "",
                "source_rel": "",
                "source_path": "",
                "source_ext": "",
                "source_size": "",
                "source_sha256": "",
            })
            continue

        enriched = []
        hashes = set()
        for root_name, root, p, rel in filtered:
            h = sha256(p)
            hashes.add(h)
            enriched.append((root_name, p, rel, h))

        if len(enriched) == 1:
            cls = "GLOBAL_STEM_UNIQUE"
        elif len(hashes) == 1:
            cls = "GLOBAL_STEM_IDENTICAL_MULTIPLE"
        else:
            cls = "GLOBAL_STEM_DIVERGENT"
        counts[cls] += 1

        for root_name, p, rel, h in enriched:
            rows.append({
                **f,
                "classification": cls,
                "source_root": root_name,
                "source_rel": rel,
                "source_path": str(p),
                "source_ext": p.suffix,
                "source_size": p.stat().st_size,
                "source_sha256": h,
            })

    fields = [
        "wanted_code","catalog_path","cat","target_rel","target_stem","target_ext",
        "classification","source_root","source_rel","source_path","source_ext",
        "source_size","source_sha256"
    ]
    report = outdir / "global_stem_hunt.tsv"
    with report.open("w", encoding="utf-8", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)

    print("=== GLOBAL STEM HUNT 058 ===")
    print(f"runtime_missing_targets={len(failures)}")
    for k in sorted(counts):
        print(f"{k}={counts[k]}")
    print(f"REPORT={report}")

    # Compact per-target candidate summary.
    print("\n=== RESOLVED STEM TARGETS ===")
    grouped = defaultdict(list)
    for r in rows:
        if r["classification"] != "UNRESOLVED_STEM":
            grouped[r["target_rel"]].append(r)
    for target in sorted(grouped):
        rr = grouped[target]
        print(f"{rr[0]['cat']:8s} {rr[0]['wanted_code']:10s} {target}")
        for r in rr[:12]:
            print(f"  {r['classification']:30s} {r['source_root']:20s} {r['source_rel']}")
        if len(rr) > 12:
            print(f"  ... +{len(rr)-12}")

if __name__ == "__main__":
    main()

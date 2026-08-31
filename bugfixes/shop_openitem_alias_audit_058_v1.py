#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import re
from collections import Counter, defaultdict
from pathlib import Path

IMAGE_EXTS = {".bmp", ".tga", ".dds", ".png", ".jpg", ".jpeg"}

ROOT_SPECS = [
    ("Summit-current", None),
    ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
    ("priston-official", Path("/home/gabriel/Área de Trabalho/priston-official")),
]

ITEM_TOKEN_RE = re.compile(r"(?<![A-Za-z0-9])([A-Za-z]{2,4}\d{2,4})(?![A-Za-z0-9])")
HEX_RE = re.compile(r"0x[0-9A-Fa-f]{6,8}")
QUOTED_RE = re.compile(r"[\"']([^\"'\r\n]{2,160})[\"']")

TEXT_EXTS = {
    ".c",".cc",".cpp",".cxx",".h",".hpp",".hxx",
    ".txt",".ini",".cfg",".conf",".csv",".tsv",".xml",".json",
    ".lua",".sql",".lst",".dat",".def",".rc"
}
SKIP_DIRS = {
    ".git","ccache","dist_mingw_client_i686","dist_mingw_server_i686",
    "logs","log","coinshop_log","timeshop_log","logfile","__pycache__"
}
MAX_TEXT = 8 * 1024 * 1024

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def read_text(p: Path) -> str:
    data = p.read_bytes()
    for enc in ("utf-8", "cp1252", "latin1"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            pass
    return data.decode("latin1", errors="replace")

def find_case_insensitive_file(parent: Path, wanted: str) -> Path | None:
    if not parent.is_dir():
        return None
    exact = parent / wanted
    if exact.is_file():
        return exact
    matches = [p for p in parent.iterdir()
               if p.is_file() and p.name.casefold() == wanted.casefold()]
    return matches[0] if len(matches) == 1 else None

def build_asset_index(client: Path):
    idx = defaultdict(list)
    total = 0
    for label, root in ROOT_SPECS:
        if label == "Summit-current":
            root = client
        if not root or not root.is_dir():
            continue
        print(f"INDEX {label}: {root}")
        for p in root.rglob("*"):
            if not p.is_file() or p.is_symlink():
                continue
            if p.suffix.casefold() not in IMAGE_EXTS:
                continue
            total += 1
            idx[p.stem.casefold()].append((label, root, p))
    return idx, total

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
            if p.stat().st_size > MAX_TEXT:
                continue
        except OSError:
            continue
        yield p

def compact_text_refs(repo: Path, cats: set[str]):
    roots = [
        ("SourceCode", repo / "SourceCode"),
        ("Servidor", repo / "Servidor"),
        ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
        ("priston-official", Path("/home/gabriel/Área de Trabalho/priston-official")),
        ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ]
    wanted = {c.casefold() for c in cats}
    rows = []
    for label, root in roots:
        if not root.is_dir():
            continue
        print(f"TEXT {label}: {root}")
        for p in iter_text_files(root):
            try:
                text = read_text(p)
            except OSError:
                continue
            rel = p.relative_to(root).as_posix()
            for n, line in enumerate(text.splitlines(), 1):
                cf = line.casefold()
                hits = sorted(c for c in wanted if c in cf)
                if not hits:
                    continue
                rows.append({
                    "root": label,
                    "file": rel,
                    "line": n,
                    "cats": ",".join(hits),
                    "text": line.strip()[:1000],
                })
    return rows

def main():
    ap = argparse.ArgumentParser(description="Resolve unresolved shop icons via OpenItem aliases.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("--targets", default="")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    openitem = repo / "Servidor" / "GameServer" / "OpenItem"
    targets_path = (
        Path(args.targets).expanduser().resolve()
        if args.targets
        else repo / "bugfixes/058_asset_reconciliation/evidence/shop_unresolved_mapping/targets.tsv"
    )
    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/shop_alias_resolution"
    outdir.mkdir(parents=True, exist_ok=True)

    if not targets_path.is_file():
        raise SystemExit(f"ERRO: targets ausente: {targets_path}")

    with targets_path.open(encoding="utf-8", newline="") as f:
        targets = list(csv.DictReader(f, delimiter="\t"))

    asset_idx, indexed = build_asset_index(client)

    openitem_rows = []
    alias_rows = []
    unresolved_openitem = []
    all_cats = {r["cat"] for r in targets if r.get("cat")}

    for t in targets:
        cat = t["cat"]
        fp = find_case_insensitive_file(openitem, f"{cat}.txt")
        if not fp:
            unresolved_openitem.append(cat)
            openitem_rows.append({
                "cat": cat,
                "code_hex": t["code_hex"],
                "target_rel": t["target_rel"],
                "openitem_file": "",
                "aliases": "",
                "hex_tokens": "",
                "quoted_strings": "",
                "text_preview": "",
            })
            continue

        text = read_text(fp)
        aliases = []
        seen = set()
        for m in ITEM_TOKEN_RE.finditer(text):
            tok = m.group(1)
            if tok.casefold() in {
                cat.casefold(),
                t["target_stem"].removeprefix("it").casefold()
            }:
                continue
            if tok.casefold() not in seen:
                seen.add(tok.casefold())
                aliases.append(tok)

        quoted = []
        for m in QUOTED_RE.finditer(text):
            s = " ".join(m.group(1).split())
            if s and s not in quoted:
                quoted.append(s)

        hexes = []
        for m in HEX_RE.finditer(text):
            v = m.group(0).upper()
            if v not in hexes:
                hexes.append(v)

        preview = " | ".join(line.strip() for line in text.splitlines() if line.strip())
        openitem_rows.append({
            "cat": cat,
            "code_hex": t["code_hex"],
            "target_rel": t["target_rel"],
            "openitem_file": str(fp.relative_to(repo)),
            "aliases": ";".join(aliases),
            "hex_tokens": ";".join(hexes),
            "quoted_strings": ";".join(quoted[:20]),
            "text_preview": preview[:3000],
        })

        for alias in aliases:
            candidate_stems = [alias.casefold(), ("it" + alias).casefold()]
            cands = []
            for stem in candidate_stems:
                cands.extend(asset_idx.get(stem, []))

            uniq = {}
            for root_label, root, p in cands:
                uniq[(root_label, str(p))] = (root_label, root, p)

            if not uniq:
                alias_rows.append({
                    "cat": cat,
                    "code_hex": t["code_hex"],
                    "target_rel": t["target_rel"],
                    "alias": alias,
                    "classification": "ALIAS_WITHOUT_ASSET",
                    "source_root": "",
                    "source_rel": "",
                    "source_path": "",
                    "source_ext": "",
                    "source_sha256": "",
                })
                continue

            enriched = []
            hashes = set()
            for root_label, root, p in uniq.values():
                h = sha256(p)
                hashes.add(h)
                try:
                    rel = p.relative_to(root).as_posix()
                except ValueError:
                    rel = str(p)
                enriched.append((root_label, rel, p, h))

            if len(enriched) == 1:
                cls = "ALIAS_ASSET_UNIQUE"
            elif len(hashes) == 1:
                cls = "ALIAS_ASSET_IDENTICAL_MULTIPLE"
            else:
                cls = "ALIAS_ASSET_DIVERGENT"

            for root_label, rel, p, h in enriched:
                alias_rows.append({
                    "cat": cat,
                    "code_hex": t["code_hex"],
                    "target_rel": t["target_rel"],
                    "alias": alias,
                    "classification": cls,
                    "source_root": root_label,
                    "source_rel": rel,
                    "source_path": str(p),
                    "source_ext": p.suffix,
                    "source_sha256": h,
                })

    oi_fields = [
        "cat","code_hex","target_rel","openitem_file","aliases",
        "hex_tokens","quoted_strings","text_preview"
    ]
    with (outdir / "openitem_semantics.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=oi_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(openitem_rows)

    alias_fields = [
        "cat","code_hex","target_rel","alias","classification",
        "source_root","source_rel","source_path","source_ext","source_sha256"
    ]
    with (outdir / "alias_asset_candidates.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=alias_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(alias_rows)

    text_rows = compact_text_refs(repo, all_cats)
    tr_fields = ["root","file","line","cats","text"]
    with (outdir / "source_semantic_refs.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=tr_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(text_rows)

    counts = Counter(r["classification"] for r in alias_rows)
    targets_with_asset_alias = {
        r["cat"] for r in alias_rows
        if r["classification"] in {
            "ALIAS_ASSET_UNIQUE",
            "ALIAS_ASSET_IDENTICAL_MULTIPLE",
            "ALIAS_ASSET_DIVERGENT"
        }
    }

    print("=== SHOP ALIAS RESOLUTION 058 ===")
    print(f"targets={len(targets)}")
    print(f"indexed_images={indexed}")
    print(f"openitem_missing={len(unresolved_openitem)}")
    print(f"alias_rows={len(alias_rows)}")
    for k in sorted(counts):
        print(f"{k}={counts[k]}")
    print(f"TARGETS_WITH_ASSET_ALIAS={len(targets_with_asset_alias)}")
    print(f"SOURCE_SEMANTIC_REFS={len(text_rows)}")
    print(f"OUT={outdir}")

    print("\n=== TARGETS WITH ASSET-BEARING ALIASES ===")
    bycat = defaultdict(list)
    for r in alias_rows:
        if r["classification"].startswith("ALIAS_ASSET_"):
            bycat[r["cat"]].append(r)
    if not bycat:
        print("(none)")
    else:
        for cat in sorted(bycat):
            print(cat)
            for r in bycat[cat][:20]:
                print(
                    f"  alias={r['alias']:8s} {r['classification']:30s} "
                    f"{r['source_root']:20s} {r['source_rel']}"
                )

if __name__ == "__main__":
    main()

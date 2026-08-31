#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import re
from collections import defaultdict, Counter
from pathlib import Path

TEXT_EXTS = {
    ".c",".cc",".cpp",".cxx",".h",".hh",".hpp",".hxx",
    ".txt",".ini",".cfg",".conf",".csv",".tsv",".xml",".json",
    ".lua",".sql",".lst",".dat",".def",".rc"
}
SKIP_DIRS = {
    ".git","logs","log","logfile","coinshop_log","timeshop_log",
    "ccache","dist_mingw_client_i686","dist_mingw_server_i686",
    "__pycache__"
}
MAX_SIZE = 8 * 1024 * 1024

QUOTED_RE = re.compile(r'["\']([^"\']*)["\']')
CODE_FIELD_RE = re.compile(r'(?:\*[^"\r\n]*?["\'])([A-Za-z]{2,4}\d{2,4})(?:["\'])')
NAME_FILE_RE = re.compile(r'(?i)(?:name\s*file|file)[^"\r\n]*["\']([^"\']+\.zhoon)["\']')
HEX_TOKEN_RE = re.compile(r'0x[0-9A-Fa-f]{6,8}')

def read_text(p: Path) -> str:
    data = p.read_bytes()
    for enc in ("utf-8", "cp1252", "latin1"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            pass
    return data.decode("latin1", errors="replace")

def iter_text_files(root: Path, exclude_openitem=False):
    if not root.is_dir():
        return
    for p in root.rglob("*"):
        if not p.is_file() or p.is_symlink():
            continue
        cparts = {x.casefold() for x in p.parts}
        if cparts & SKIP_DIRS:
            continue
        if exclude_openitem and "openitem" in cparts:
            continue
        if p.suffix.casefold() not in TEXT_EXTS:
            continue
        try:
            if p.stat().st_size > MAX_SIZE:
                continue
        except OSError:
            continue
        yield p

def extract_openitem_semantics(text: str, filename_cat: str):
    quoted = [x.strip() for x in QUOTED_RE.findall(text) if x.strip()]

    # The first quoted token equal to an item-like code is the internal item code.
    internal = ""
    for q in quoted:
        if re.fullmatch(r"[A-Za-z]{2,4}\d{2,4}", q):
            internal = q
            break

    # Prefer the first human-readable quoted string that isn't code/path.
    human = ""
    for q in quoted:
        if q.casefold() == filename_cat.casefold():
            continue
        if re.fullmatch(r"[A-Za-z]{2,4}\d{2,4}", q):
            continue
        if "\\" in q or "/" in q or q.casefold().endswith(".zhoon"):
            continue
        if any(ch.isalpha() for ch in q):
            human = q
            break

    name_file = ""
    for q in quoted:
        if q.casefold().endswith(".zhoon"):
            name_file = q
            break

    return internal, human, name_file, quoted

def classify_ref(root_label: str, rel: str):
    cf = rel.casefold()
    if "timeshop" in cf or "time_shop" in cf:
        return "TIMESHOP_CONFIG_OR_CODE"
    if "coinshop" in cf or "coin_shop" in cf or "itemshop" in cf or "item_shop" in cf:
        return "COINSHOP_ITEMSHOP_CONFIG_OR_CODE"
    if root_label == "SourceCode":
        return "SOURCE_CODE"
    if root_label == "Servidor":
        return "SERVER_OTHER"
    return "OTHER"

def main():
    ap = argparse.ArgumentParser(
        description="Audit unresolved shop item semantics and real shop/config references; read-only."
    )
    ap.add_argument("repo", nargs="?", default=".")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    targets_path = repo / "bugfixes/058_asset_reconciliation/evidence/shop_unresolved_mapping/targets.tsv"
    openitem_dir = repo / "Servidor/GameServer/OpenItem"
    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/shop_catalog_mapping"
    outdir.mkdir(parents=True, exist_ok=True)

    if not targets_path.is_file():
        raise SystemExit(f"ERRO: targets ausente: {targets_path}")

    with targets_path.open(encoding="utf-8", newline="") as f:
        targets = list(csv.DictReader(f, delimiter="\t"))

    semantics = []
    by_cat = {}

    for t in targets:
        cat = t["cat"]
        fp = openitem_dir / f"{cat}.txt"
        if not fp.is_file():
            # case-insensitive fallback
            mm = [x for x in openitem_dir.glob("*.txt") if x.stem.casefold() == cat.casefold()]
            fp = mm[0] if len(mm) == 1 else fp

        if fp.is_file():
            text = read_text(fp)
            internal, human, name_file, quoted = extract_openitem_semantics(text, cat)
        else:
            internal = human = name_file = ""
            quoted = []

        row = {
            "catalog_cat": cat,
            "catalog_code_hex": t["code_hex"],
            "catalog_path": t["catalog_path"],
            "target_rel": t["target_rel"],
            "openitem_file": str(fp.relative_to(repo)) if fp.is_file() else "",
            "openitem_internal_code": internal,
            "openitem_name": human,
            "openitem_name_file": name_file,
            "filename_equals_internal": (
                "YES" if internal and internal.casefold() == cat.casefold()
                else "NO" if internal else ""
            ),
            "quoted_values": ";".join(quoted[:20]),
        }
        semantics.append(row)
        by_cat[cat.casefold()] = row

    sem_fields = [
        "catalog_cat","catalog_code_hex","catalog_path","target_rel",
        "openitem_file","openitem_internal_code","openitem_name",
        "openitem_name_file","filename_equals_internal","quoted_values"
    ]
    with (outdir / "openitem_catalog_semantics.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=sem_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(semantics)

    scan_roots = [
        ("SourceCode", repo / "SourceCode"),
        ("Servidor", repo / "Servidor"),
    ]
    ref_rows = []

    # Search actual source/configs, excluding logs and OpenItem themselves.
    for root_label, root in scan_roots:
        print(f"SCAN {root_label}: {root}")
        for p in iter_text_files(root, exclude_openitem=True):
            try:
                text = read_text(p)
            except OSError:
                continue
            rel = p.relative_to(root).as_posix()

            for lineno, line in enumerate(text.splitlines(), 1):
                cf = line.casefold()

                for sem in semantics:
                    cat = sem["catalog_cat"]
                    internal = sem["openitem_internal_code"]
                    code_hex = sem["catalog_code_hex"]

                    hits = []
                    if cat and re.search(
                        rf'(?<![A-Za-z0-9]){re.escape(cat)}(?![A-Za-z0-9])',
                        line, re.I
                    ):
                        hits.append("CAT")
                    if internal and internal.casefold() != cat.casefold() and re.search(
                        rf'(?<![A-Za-z0-9]){re.escape(internal)}(?![A-Za-z0-9])',
                        line, re.I
                    ):
                        hits.append("OPENITEM_INTERNAL_CODE")
                    if code_hex and code_hex.casefold() in cf:
                        hits.append("HEX")

                    if not hits:
                        continue

                    ref_rows.append({
                        "catalog_cat": cat,
                        "catalog_code_hex": code_hex,
                        "openitem_internal_code": internal,
                        "openitem_name": sem["openitem_name"],
                        "root": root_label,
                        "file": rel,
                        "line": lineno,
                        "classification": classify_ref(root_label, rel),
                        "match_kind": ",".join(hits),
                        "text": line.strip()[:1200],
                    })

    ref_fields = [
        "catalog_cat","catalog_code_hex","openitem_internal_code","openitem_name",
        "root","file","line","classification","match_kind","text"
    ]
    with (outdir / "source_config_refs.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=ref_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(ref_rows)

    grouped = defaultdict(list)
    for r in ref_rows:
        grouped[r["catalog_cat"]].append(r)

    summary = []
    for sem in semantics:
        rr = grouped.get(sem["catalog_cat"], [])
        classes = Counter(x["classification"] for x in rr)
        summary.append({
            "catalog_cat": sem["catalog_cat"],
            "catalog_code_hex": sem["catalog_code_hex"],
            "openitem_internal_code": sem["openitem_internal_code"],
            "openitem_name": sem["openitem_name"],
            "filename_equals_internal": sem["filename_equals_internal"],
            "timeshop_refs": classes["TIMESHOP_CONFIG_OR_CODE"],
            "coinshop_itemshop_refs": classes["COINSHOP_ITEMSHOP_CONFIG_OR_CODE"],
            "source_code_refs": classes["SOURCE_CODE"],
            "server_other_refs": classes["SERVER_OTHER"],
            "ref_files": ";".join(sorted({f"{x['root']}:{x['file']}" for x in rr})[:30]),
        })

    sum_fields = [
        "catalog_cat","catalog_code_hex","openitem_internal_code","openitem_name",
        "filename_equals_internal","timeshop_refs","coinshop_itemshop_refs",
        "source_code_refs","server_other_refs","ref_files"
    ]
    with (outdir / "target_summary.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=sum_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(summary)

    mismatch = [x for x in semantics if x["filename_equals_internal"] == "NO"]
    time_targets = [x for x in summary if int(x["timeshop_refs"]) > 0]
    coin_targets = [x for x in summary if int(x["coinshop_itemshop_refs"]) > 0]

    print("=== SHOP CATALOG MAPPING AUDIT 058 ===")
    print("policy=READ_ONLY; NO_IMAGE_GENERATION; NO_CONVERSION; NO_COPY")
    print(f"targets={len(targets)}")
    print(f"OPENITEM_FILENAME_INTERNAL_MISMATCH={len(mismatch)}")
    print(f"TARGETS_WITH_TIMESHOP_CONFIG_OR_CODE_REFS={len(time_targets)}")
    print(f"TARGETS_WITH_COINSHOP_ITEMSHOP_CONFIG_OR_CODE_REFS={len(coin_targets)}")
    print(f"REF_ROWS={len(ref_rows)}")
    print(f"OUT={outdir}")

    print("\n=== OPENITEM FILENAME != INTERNAL CODE ===")
    for x in mismatch:
        print(
            f"{x['catalog_cat']:8s} -> internal={x['openitem_internal_code']:8s} "
            f"name={x['openitem_name']}"
        )

    print("\n=== TARGETS WITH SHOP-SPECIFIC SOURCE/CONFIG REFS ===")
    any_shown = False
    for x in summary:
        if int(x["timeshop_refs"]) or int(x["coinshop_itemshop_refs"]):
            any_shown = True
            print(
                f"{x['catalog_cat']:8s} internal={x['openitem_internal_code']:8s} "
                f"time={x['timeshop_refs']:>3} coin/item={x['coinshop_itemshop_refs']:>3} "
                f"name={x['openitem_name']}"
            )
    if not any_shown:
        print("(none)")

if __name__ == "__main__":
    main()

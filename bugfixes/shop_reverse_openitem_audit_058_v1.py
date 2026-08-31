#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import re
from collections import Counter, defaultdict
from pathlib import Path

IMAGE_EXTS = {".bmp", ".tga", ".dds", ".png", ".jpg", ".jpeg"}
TOKEN_RE = re.compile(r"(?<![A-Za-z0-9])([A-Za-z]{2,4}\d{2,4})(?![A-Za-z0-9])")

ROOTS = [
    ("Summit-current", None),
    ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
    ("priston-official", Path("/home/gabriel/Área de Trabalho/priston-official")),
]

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

def build_asset_index(client: Path):
    idx = defaultdict(list)
    total = 0
    for label, root in ROOTS:
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

def main():
    ap = argparse.ArgumentParser(
        description="Find reverse OpenItem references to unresolved shop item codes and asset-bearing wrappers."
    )
    ap.add_argument("repo", nargs="?", default=".")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    openitem = repo / "Servidor" / "GameServer" / "OpenItem"
    targets_path = repo / "bugfixes/058_asset_reconciliation/evidence/shop_unresolved_mapping/targets.tsv"
    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/shop_reverse_openitem"
    outdir.mkdir(parents=True, exist_ok=True)

    if not targets_path.is_file():
        raise SystemExit(f"ERRO: targets ausente: {targets_path}")
    if not openitem.is_dir():
        raise SystemExit(f"ERRO: OpenItem ausente: {openitem}")

    with targets_path.open(encoding="utf-8", newline="") as f:
        targets = list(csv.DictReader(f, delimiter="\t"))

    by_cat = {t["cat"].casefold(): t for t in targets}

    # Parse all OpenItem files once.
    docs = []
    for p in sorted(openitem.glob("*.txt"), key=lambda x: x.name.casefold()):
        try:
            text = read_text(p)
        except OSError:
            continue
        docs.append((p, text, text.casefold()))

    asset_idx, indexed = build_asset_index(client)

    refs = []
    candidate_rows = []

    for target_cf, t in by_cat.items():
        target_cat = t["cat"]
        token_pat = re.compile(
            rf"(?<![A-Za-z0-9]){re.escape(target_cat)}(?![A-Za-z0-9])",
            re.I
        )

        for p, text, _cf in docs:
            owner = p.stem
            if owner.casefold() == target_cf:
                continue

            lines = text.splitlines()
            matched_lines = []
            for n, line in enumerate(lines, 1):
                if token_pat.search(line):
                    matched_lines.append((n, line.strip()))

            if not matched_lines:
                continue

            refs.append({
                "target_cat": target_cat,
                "target_code_hex": t["code_hex"],
                "target_rel": t["target_rel"],
                "ref_owner_cat": owner,
                "ref_file": str(p.relative_to(repo)),
                "match_lines": "; ".join(f"{n}:{line}" for n, line in matched_lines[:20]),
            })

            # The referencing OpenItem's own CAT is the reverse-alias/wrapper candidate.
            stems = [owner.casefold(), ("it" + owner).casefold()]
            cands = []
            for stem in stems:
                cands.extend(asset_idx.get(stem, []))

            uniq = {}
            for root_label, root, img in cands:
                uniq[(root_label, str(img))] = (root_label, root, img)

            if not uniq:
                candidate_rows.append({
                    "target_cat": target_cat,
                    "target_code_hex": t["code_hex"],
                    "target_rel": t["target_rel"],
                    "wrapper_cat": owner,
                    "classification": "REVERSE_REF_NO_ASSET",
                    "source_root": "",
                    "source_rel": "",
                    "source_path": "",
                    "source_ext": "",
                    "source_sha256": "",
                })
                continue

            enriched = []
            hashes = set()
            for root_label, root, img in uniq.values():
                h = sha256(img)
                hashes.add(h)
                try:
                    rel = img.relative_to(root).as_posix()
                except ValueError:
                    rel = str(img)
                enriched.append((root_label, rel, img, h))

            if len(enriched) == 1:
                cls = "REVERSE_REF_ASSET_UNIQUE"
            elif len(hashes) == 1:
                cls = "REVERSE_REF_ASSET_IDENTICAL_MULTIPLE"
            else:
                cls = "REVERSE_REF_ASSET_DIVERGENT"

            for root_label, rel, img, h in enriched:
                candidate_rows.append({
                    "target_cat": target_cat,
                    "target_code_hex": t["code_hex"],
                    "target_rel": t["target_rel"],
                    "wrapper_cat": owner,
                    "classification": cls,
                    "source_root": root_label,
                    "source_rel": rel,
                    "source_path": str(img),
                    "source_ext": img.suffix,
                    "source_sha256": h,
                })

    ref_fields = [
        "target_cat","target_code_hex","target_rel",
        "ref_owner_cat","ref_file","match_lines"
    ]
    with (outdir / "reverse_refs.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=ref_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(refs)

    cand_fields = [
        "target_cat","target_code_hex","target_rel","wrapper_cat",
        "classification","source_root","source_rel","source_path",
        "source_ext","source_sha256"
    ]
    with (outdir / "reverse_asset_candidates.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=cand_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(candidate_rows)

    counts = Counter(r["classification"] for r in candidate_rows)
    targets_with_reverse = {r["target_cat"] for r in refs}
    targets_with_assets = {
        r["target_cat"] for r in candidate_rows
        if r["classification"].startswith("REVERSE_REF_ASSET_")
    }

    print("=== SHOP REVERSE OPENITEM AUDIT 058 ===")
    print(f"targets={len(targets)}")
    print(f"openitem_files={len(docs)}")
    print(f"indexed_images={indexed}")
    print(f"REVERSE_REF_TARGETS={len(targets_with_reverse)}")
    print(f"REVERSE_REF_ROWS={len(refs)}")
    for k in sorted(counts):
        print(f"{k}={counts[k]}")
    print(f"TARGETS_WITH_REVERSE_ASSET={len(targets_with_assets)}")
    print(f"OUT={outdir}")

    print("\n=== REVERSE REFERENCES ===")
    for r in refs:
        print(
            f"{r['target_cat']:8s} <- {r['ref_owner_cat']:8s} "
            f"{r['ref_file']}"
        )

    print("\n=== ASSET-BEARING REVERSE REFERENCES ===")
    shown = False
    for r in candidate_rows:
        if r["classification"].startswith("REVERSE_REF_ASSET_"):
            shown = True
            print(
                f"{r['target_cat']:8s} <- {r['wrapper_cat']:8s} "
                f"{r['classification']:34s} "
                f"{r['source_root']:20s} {r['source_rel']}"
            )
    if not shown:
        print("(none)")

if __name__ == "__main__":
    main()

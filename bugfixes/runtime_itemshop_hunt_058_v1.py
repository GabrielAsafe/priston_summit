#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import os
import re
import shutil
from collections import defaultdict, Counter
from pathlib import Path

TRUSTED_ROOTS = [
    ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
    ("priston-official", Path("/home/gabriel/Área de Trabalho/priston-official")),
]

IMAGE_FAIL_RE = re.compile(r'\[IMAGE_FAIL\].*?path="([^"]+)"', re.I)
IMAGE_EXTS = {".bmp", ".tga", ".dds", ".png", ".jpg", ".jpeg"}

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def norm_rel(s: str) -> str:
    s = s.replace("\\", "/")
    while "//" in s:
        s = s.replace("//", "/")
    return s.lstrip("/")

def suffix_match(path_parts_cf, target_parts_cf) -> bool:
    if len(path_parts_cf) < len(target_parts_cf):
        return False
    return tuple(path_parts_cf[-len(target_parts_cf):]) == tuple(target_parts_cf)

def parse_targets(log: Path, client: Path):
    wanted = []
    seen = set()
    for line in log.read_text(errors="replace").splitlines():
        m = IMAGE_FAIL_RE.search(line)
        if not m:
            continue
        rel = norm_rel(m.group(1))
        key = rel.casefold()
        if key in seen:
            continue
        seen.add(key)
        dst = client / rel
        if dst.is_file():
            continue
        wanted.append(rel)
    return wanted

def build_index():
    rows = []
    for root_name, root in TRUSTED_ROOTS:
        if not root.is_dir():
            print(f"WARN missing trusted root: {root}")
            continue
        print(f"INDEX {root_name}: {root}")
        for p in root.rglob("*"):
            if not p.is_file() or p.is_symlink():
                continue
            if p.suffix.casefold() not in IMAGE_EXTS:
                continue
            try:
                rel = p.relative_to(root)
            except ValueError:
                continue
            rows.append({
                "root_name": root_name,
                "root": root,
                "path": p,
                "parts_cf": tuple(x.casefold() for x in rel.parts),
                "stem_cf": p.stem.casefold(),
                "name_cf": p.name.casefold(),
                "ext_cf": p.suffix.casefold(),
            })
    return rows

def classify(target: str, index):
    t = Path(target)
    tparts_cf = tuple(x.casefold() for x in t.parts)
    tstem_cf = t.stem.casefold()
    text_cf = t.suffix.casefold()
    tname_cf = t.name.casefold()

    same_path = []
    alt_ext_path = []
    basename = []

    for r in index:
        if r["ext_cf"] == text_cf and suffix_match(r["parts_cf"], tparts_cf):
            same_path.append(r)
            continue

        if r["stem_cf"] == tstem_cf:
            # same directory suffix, but alternate extension
            if len(tparts_cf) >= 2:
                target_dir_cf = tparts_cf[:-1]
                src_parent_cf = r["parts_cf"][:-1]
                if len(src_parent_cf) >= len(target_dir_cf) and tuple(src_parent_cf[-len(target_dir_cf):]) == tuple(target_dir_cf):
                    alt_ext_path.append(r)
                    continue

        if r["name_cf"] == tname_cf:
            basename.append(r)

    def enrich(rows):
        out = []
        for r in rows:
            out.append((r, sha256(r["path"])))
        return out

    same_path_h = enrich(same_path)
    alt_ext_h = enrich(alt_ext_path)
    basename_h = enrich(basename)

    if same_path_h:
        hashes = {h for _, h in same_path_h}
        if len(hashes) == 1:
            return "SAFE_EXACT_SUFFIX_COPY", same_path_h, alt_ext_h, basename_h
        return "AMBIGUOUS_EXACT_SUFFIX_CONTENT", same_path_h, alt_ext_h, basename_h

    if alt_ext_h:
        return "ALT_EXTENSION_REVIEW", same_path_h, alt_ext_h, basename_h

    if basename_h:
        return "BASENAME_ONLY_REVIEW", same_path_h, alt_ext_h, basename_h

    return "UNRESOLVED", [], [], []

def main():
    ap = argparse.ArgumentParser(description="Hunt runtime IMAGE_FAIL item icons across trusted roots.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    ap.add_argument("--log", default="")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    log = Path(args.log).expanduser().resolve() if args.log else client / "LoadItemImage_debug.txt"
    evid = repo / "bugfixes/058_asset_reconciliation/evidence/runtime_itemshop_hunt"
    evid.mkdir(parents=True, exist_ok=True)
    manifest = evid / "apply_manifest.tsv"

    if args.action == "rollback":
        if not manifest.is_file():
            raise SystemExit(f"ERRO: manifest ausente: {manifest}")
        with manifest.open(encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f, delimiter="\t"))

        for r in rows:
            p = client / r["target_rel"]
            if not p.exists():
                continue
            if not p.is_file() or sha256(p) != r["target_sha256"]:
                raise SystemExit(f"ERRO: rollback recusado; arquivo mudou: {p}")

        removed = 0
        for r in reversed(rows):
            p = client / r["target_rel"]
            if p.is_file():
                p.unlink()
                removed += 1

        for r in reversed(rows):
            p = (client / r["target_rel"]).parent
            while p != client:
                try:
                    p.rmdir()
                except OSError:
                    break
                p = p.parent

        print(f"OK: rollback removeu {removed} arquivos.")
        return

    if not log.is_file():
        raise SystemExit(f"ERRO: runtime log ausente: {log}")

    targets = parse_targets(log, client)
    index = build_index()

    print("=== RUNTIME ITEMSHOP/TIMESHOP IMAGE HUNT 058 ===")
    print(f"log={log}")
    print(f"runtime_missing_targets={len(targets)}")
    print(f"indexed_images={len(index)}")

    report_rows = []
    safe_targets = []
    counts = Counter()

    for target in targets:
        cls, same, alt, basename = classify(target, index)
        counts[cls] += 1

        candidates = same if same else alt if alt else basename
        if not candidates:
            report_rows.append({
                "classification": cls,
                "target_rel": target,
                "source_root": "",
                "source_path": "",
                "source_sha256": "",
                "source_ext": "",
            })
        else:
            for r, h in candidates:
                report_rows.append({
                    "classification": cls,
                    "target_rel": target,
                    "source_root": r["root_name"],
                    "source_path": str(r["path"]),
                    "source_sha256": h,
                    "source_ext": r["path"].suffix,
                })

        if cls == "SAFE_EXACT_SUFFIX_COPY":
            safe_targets.append((target, same))

    fields = ["classification","target_rel","source_root","source_path","source_sha256","source_ext"]
    report = evid / "hunt.tsv"
    with report.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(report_rows)

    for cls in sorted(counts):
        print(f"{cls}={counts[cls]}")
    print(f"REPORT={report}")

    if args.action == "audit":
        print("APPLY=NOT_RUN")
        return

    ambiguous = counts["AMBIGUOUS_EXACT_SUFFIX_CONTENT"]
    if ambiguous:
        raise SystemExit("ERRO: há candidatos exact-path com conteúdo divergente; nada aplicado.")

    created = []
    manifest_rows = []
    try:
        for target, candidates in safe_targets:
            dst = client / target
            if dst.exists():
                continue

            # All exact-suffix candidates have same hash by classification.
            src_row, src_hash = candidates[0]
            src = src_row["path"]

            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)

            dst_hash = sha256(dst)
            if dst_hash != src_hash:
                raise RuntimeError(f"SHA divergiu após copy: {dst}")

            created.append(dst)
            manifest_rows.append({
                "target_rel": target,
                "source_root": src_row["root_name"],
                "source_path": str(src),
                "source_sha256": src_hash,
                "target_sha256": dst_hash,
            })
    except Exception:
        for p in reversed(created):
            try:
                p.unlink()
            except OSError:
                pass
        raise

    mf_fields = ["target_rel","source_root","source_path","source_sha256","target_sha256"]
    with manifest.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=mf_fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(manifest_rows)

    print(f"OK: copiados {len(manifest_rows)} runtime assets exact-path.")
    print(f"MANIFEST={manifest}")
    print("RUNTIME_TEST=READY")

if __name__ == "__main__":
    main()

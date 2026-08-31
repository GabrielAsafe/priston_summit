#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import os
from collections import defaultdict
from pathlib import Path

IMAGE_EXTS = {".bmp", ".tga", ".png", ".jpg", ".jpeg", ".dds", ".gif", ".pcx"}
TRUSTED_ROOTS = [
    ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
]

def norm(s: str) -> str:
    s = (s or "").strip().strip('"\'').replace("\\", "/")
    while "//" in s:
        s = s.replace("//", "/")
    while s.startswith("./"):
        s = s[2:]
    return s.strip("/")

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def endswith_parts(candidate: tuple[str, ...], wanted: tuple[str, ...]) -> bool:
    if len(candidate) < len(wanted):
        return False
    tail = candidate[-len(wanted):]
    return all(a.casefold() == b.casefold() for a, b in zip(tail, wanted))

def parent_endswith_parts(candidate: tuple[str, ...], wanted: tuple[str, ...]) -> bool:
    if not wanted:
        return True
    return endswith_parts(candidate, wanted)

def resolve_existing_child_case(parent: Path, wanted: str) -> str | None:
    exact = parent / wanted
    if exact.exists():
        return wanted
    if not parent.is_dir():
        return None
    matches = [p.name for p in parent.iterdir() if p.name.casefold() == wanted.casefold()]
    if len(matches) == 1:
        return matches[0]
    return None

def runtime_destination(client: Path, target: str, source_tail: tuple[str, ...]) -> str:
    """
    Existing runtime directory case wins.
    For missing components, use trusted source casing.
    """
    target_parts = list(Path(target).parts)
    src_parts = list(source_tail)
    out = []
    cur = client
    for i, wanted in enumerate(target_parts):
        src_case = src_parts[i] if i < len(src_parts) else wanted
        if i < len(target_parts) - 1:
            existing = resolve_existing_child_case(cur, wanted)
            chosen = existing if existing is not None else src_case
            out.append(chosen)
            cur = cur / chosen
        else:
            # File does not exist by definition; preserve trusted-source filename case.
            out.append(src_case)
    return "/".join(out)

def risk_for(target: str) -> str:
    cf = target.casefold()
    if target.lower().endswith(".bmp") and (
        cf.startswith("image/sinimage/inter/")
        or cf.startswith("startimage/")
        or cf.startswith("game/maps/chrselect/")
    ):
        return "PROTECTED_BMP_REVIEW"
    if target.lower().endswith(".bmp"):
        return "BMP_REVIEW"
    return "NORMAL"

def main():
    ap = argparse.ArgumentParser(description="Strict full-path trusted asset reconciliation audit.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("--missing-tsv",
                    default="bugfixes/058_asset_reconciliation/evidence/image_audit/missing_static_images.tsv")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    missing = (repo / args.missing_tsv).resolve()
    if not client.is_dir():
        raise SystemExit(f"ERRO: runtime cliente ausente: {client}")
    if not missing.is_file():
        raise SystemExit(f"ERRO: TSV ausente: {missing}")

    roots = [(lab, p) for lab, p in TRUSTED_ROOTS if p.is_dir()]
    if not roots:
        raise SystemExit("ERRO: nenhum trusted root encontrado.")

    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/trusted_asset_hunt_v3"
    outdir.mkdir(parents=True, exist_ok=True)

    grouped = {}
    with missing.open("r", encoding="utf-8", errors="replace", newline="") as f:
        for r in csv.DictReader(f, delimiter="\t"):
            target = norm(r.get("normalized") or r.get("literal") or "")
            if not target:
                continue
            k = target.casefold()
            g = grouped.setdefault(k, {"target": target, "refs": []})
            g["refs"].append(f'{r.get("source_file","")}:{r.get("line","")}')

    # One recursive index over exactly the two roots requested.
    by_name = defaultdict(list)
    by_stem = defaultdict(list)
    total_images = 0
    for lab, root in roots:
        print(f"INDEX {lab}: {root}", flush=True)
        for dp, _, fns in os.walk(root):
            d = Path(dp)
            for fn in fns:
                p = d / fn
                if p.suffix.casefold() not in IMAGE_EXTS:
                    continue
                total_images += 1
                try:
                    rel_parts = p.relative_to(root).parts
                except ValueError:
                    continue
                rec = {
                    "root_label": lab,
                    "root": root,
                    "path": p,
                    "rel_parts": tuple(rel_parts),
                    "rel": Path(*rel_parts).as_posix(),
                }
                by_name[fn.casefold()].append(rec)
                by_stem[p.stem.casefold()].append(rec)

    digest_cache = {}
    def digest(p: Path):
        k = str(p)
        if k not in digest_cache:
            digest_cache[k] = sha256(p)
        return digest_cache[k]

    plan = []
    context_required = []
    unresolved = []
    ambiguous = []
    conversion = []
    safe_copy = []

    for g in grouped.values():
        target = g["target"]
        tp = Path(target)
        parts = tuple(tp.parts)
        refs = " | ".join(g["refs"])
        risk = risk_for(target)

        # A basename-only literal does not identify a runtime destination.
        if len(parts) < 2:
            same_name = by_name.get(tp.name.casefold(), [])
            same_stem = by_stem.get(tp.stem.casefold(), [])
            candidates = same_name + [x for x in same_stem if x["path"].suffix.casefold() != tp.suffix.casefold()]
            context_required.append({
                "target_rel": target,
                "ref_count": len(g["refs"]),
                "references": refs,
                "risk": risk,
                "candidate_count": len({str(x["path"]) for x in candidates}),
                "candidate_examples": " | ".join(str(x["path"]) for x in candidates[:8]),
                "reason": "BASENAME_ONLY_LITERAL_NO_RUNTIME_DIRECTORY",
            })
            continue

        # Same-format requires the ENTIRE target path as a suffix under the trusted root.
        same_format = []
        for rec in by_name.get(tp.name.casefold(), []):
            if endswith_parts(rec["rel_parts"], parts):
                same_format.append(rec)

        # Alternate-format requires the ENTIRE PARENT path plus same stem.
        alt_format = []
        parent_parts = parts[:-1]
        for rec in by_stem.get(tp.stem.casefold(), []):
            if rec["path"].suffix.casefold() == tp.suffix.casefold():
                continue
            if parent_endswith_parts(rec["rel_parts"][:-1], parent_parts):
                alt_format.append(rec)

        if same_format:
            hashes = defaultdict(list)
            for rec in same_format:
                hashes[digest(rec["path"])].append(rec)
            if len(hashes) == 1:
                # Identical content across roots is safe to source from the first deterministic candidate.
                chosen = sorted(same_format, key=lambda r: (r["root_label"], r["rel"].casefold()))[0]
                source_tail = chosen["rel_parts"][-len(parts):]
                dest = runtime_destination(client, target, source_tail)
                row = {
                    "classification": "SAFE_FULL_PATH_COPY",
                    "target_rel": target,
                    "runtime_destination": dest,
                    "source_case_update_required": "1" if target != dest else "0",
                    "ref_count": len(g["refs"]),
                    "references": refs,
                    "risk": risk,
                    "source_root": chosen["root_label"],
                    "source_path": str(chosen["path"]),
                    "source_sha256": digest(chosen["path"]),
                    "trusted_match_count": len(same_format),
                    "trusted_hash_count": 1,
                    "note": "full target path matches trusted path suffix; all copies byte-identical",
                }
                safe_copy.append(row)
                plan.append(row)
            else:
                for h, recs in hashes.items():
                    for rec in recs:
                        ambiguous.append({
                            "classification": "AMBIGUOUS_FULL_PATH_COPY",
                            "target_rel": target,
                            "runtime_destination": "",
                            "source_case_update_required": "",
                            "ref_count": len(g["refs"]),
                            "references": refs,
                            "risk": risk,
                            "source_root": rec["root_label"],
                            "source_path": str(rec["path"]),
                            "source_sha256": h,
                            "trusted_match_count": len(same_format),
                            "trusted_hash_count": len(hashes),
                            "note": "same full relative path exists with conflicting content",
                        })
            continue

        if alt_format:
            hashes = defaultdict(list)
            for rec in alt_format:
                hashes[digest(rec["path"])].append(rec)
            # Conversion is never auto-safe, but full parent path is strong evidence.
            for h, recs in hashes.items():
                for rec in recs:
                    source_tail = list(rec["rel_parts"][-len(parts):])
                    # Keep trusted source directory case and source basename stem, but desired target extension.
                    source_tail[-1] = rec["path"].stem + tp.suffix
                    dest = runtime_destination(client, target, tuple(source_tail))
                    conversion.append({
                        "classification": "FULL_PATH_CONVERSION_REVIEW",
                        "target_rel": target,
                        "runtime_destination": dest,
                        "source_case_update_required": "1" if target != dest else "0",
                        "ref_count": len(g["refs"]),
                        "references": refs,
                        "risk": risk,
                        "source_root": rec["root_label"],
                        "source_path": str(rec["path"]),
                        "source_sha256": h,
                        "trusted_match_count": len(alt_format),
                        "trusted_hash_count": len(hashes),
                        "note": f"full parent path + stem match; convert {rec['path'].suffix} -> {tp.suffix}",
                    })
            continue

        # No full-path match. Preserve weaker basename evidence only for manual context reconstruction.
        weak = by_name.get(tp.name.casefold(), [])
        weak_alt = [x for x in by_stem.get(tp.stem.casefold(), [])
                    if x["path"].suffix.casefold() != tp.suffix.casefold()]
        weak_all = weak + weak_alt
        if weak_all:
            context_required.append({
                "target_rel": target,
                "ref_count": len(g["refs"]),
                "references": refs,
                "risk": risk,
                "candidate_count": len({str(x["path"]) for x in weak_all}),
                "candidate_examples": " | ".join(str(x["path"]) for x in weak_all[:8]),
                "reason": "NAME_OR_STEM_FOUND_BUT_DIRECTORY_CONTEXT_DOES_NOT_MATCH",
            })
        else:
            unresolved.append({
                "target_rel": target,
                "ref_count": len(g["refs"]),
                "references": refs,
                "risk": risk,
                "reason": "NO_NAME_OR_STEM_IN_TRUSTED_ROOTS",
            })

    common_fields = [
        "classification","target_rel","runtime_destination","source_case_update_required",
        "ref_count","references","risk","source_root","source_path","source_sha256",
        "trusted_match_count","trusted_hash_count","note"
    ]
    def write(path, rows, fields):
        with path.open("w", encoding="utf-8", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
            w.writeheader()
            w.writerows(rows)

    write(outdir/"safe_full_path_copy.tsv", safe_copy, common_fields)
    write(outdir/"full_path_conversion_review.tsv", conversion, common_fields)
    write(outdir/"ambiguous_full_path.tsv", ambiguous, common_fields)

    ctx_fields = ["target_rel","ref_count","references","risk","candidate_count","candidate_examples","reason"]
    write(outdir/"context_required.tsv", context_required, ctx_fields)

    unr_fields = ["target_rel","ref_count","references","risk","reason"]
    write(outdir/"unresolved.tsv", unresolved, unr_fields)

    with (outdir/"summary.txt").open("w", encoding="utf-8") as f:
        f.write(f"UNIQUE_MISSING_TARGETS={len(grouped)}\n")
        f.write(f"INDEXED_IMAGES={total_images}\n")
        f.write(f"SAFE_FULL_PATH_COPY={len(safe_copy)}\n")
        f.write(f"FULL_PATH_CONVERSION_REVIEW_TARGETS={len({r['target_rel'] for r in conversion})}\n")
        f.write(f"FULL_PATH_CONVERSION_REVIEW_ROWS={len(conversion)}\n")
        f.write(f"AMBIGUOUS_FULL_PATH_TARGETS={len({r['target_rel'] for r in ambiguous})}\n")
        f.write(f"CONTEXT_REQUIRED={len(context_required)}\n")
        f.write(f"UNRESOLVED={len(unresolved)}\n")
        f.write("\nTRUSTED_ROOTS\n")
        for lab, root in roots:
            f.write(f"{lab}={root}\n")

    print("=== STRICT TRUSTED ASSET HUNT 058 V3 ===")
    print(f"UNIQUE_MISSING_TARGETS={len(grouped)}")
    print(f"INDEXED_IMAGES={total_images}")
    print(f"SAFE_FULL_PATH_COPY={len(safe_copy)}")
    print(f"FULL_PATH_CONVERSION_REVIEW_TARGETS={len({r['target_rel'] for r in conversion})}")
    print(f"AMBIGUOUS_FULL_PATH_TARGETS={len({r['target_rel'] for r in ambiguous})}")
    print(f"CONTEXT_REQUIRED={len(context_required)}")
    print(f"UNRESOLVED={len(unresolved)}")
    print(f"OUT={outdir}")

if __name__ == "__main__":
    main()

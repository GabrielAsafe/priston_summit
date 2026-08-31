#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import os
import shutil
import subprocess
from collections import defaultdict
from pathlib import Path

IMAGE_EXTS = {".bmp", ".tga", ".png", ".jpg", ".jpeg", ".dds", ".gif", ".pcx"}

TRUSTED_ROOTS = [
    ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
]

def norm_rel(s: str) -> str:
    s = (s or "").strip().strip('"\'').replace("\\", "/")
    while "//" in s:
        s = s.replace("//", "/")
    while s.startswith("./"):
        s = s[2:]
    return s.strip("/")

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def suffix_score(candidate_parts, target_parts) -> int:
    score = 0
    for a, b in zip(reversed(candidate_parts), reversed(target_parts)):
        if a.casefold() != b.casefold():
            break
        score += 1
    return score

def target_risk(target_rel: str) -> str:
    cf = target_rel.casefold()
    ext = Path(target_rel).suffix.casefold()
    if ext == ".bmp" and (
        cf.startswith("image/sinimage/inter/")
        or cf.startswith("game/maps/chrselect/")
        or cf.startswith("startimage/")
    ):
        return "PROTECTED_OR_LEGACY_BMP_REVIEW"
    if ext == ".bmp":
        return "BMP_HEADER_REVIEW"
    return "NORMAL"

def decoder_probe(path: Path) -> tuple[str, str]:
    # No conversion. Only asks installed Linux tools whether they can decode/read the source.
    magick = shutil.which("magick")
    identify = shutil.which("identify")
    ffprobe = shutil.which("ffprobe")

    commands = []
    if magick:
        commands.append(("magick-identify", [magick, "identify", "-quiet", str(path)]))
    elif identify:
        commands.append(("identify", [identify, "-quiet", str(path)]))
    if ffprobe:
        commands.append(("ffprobe", [ffprobe, "-v", "error", "-show_entries", "stream=codec_name,width,height", "-of", "default=nw=1", str(path)]))

    if not commands:
        return ("NO_DECODER_TOOL", "")

    errors = []
    for label, cmd in commands:
        try:
            p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=8)
        except Exception as e:
            errors.append(f"{label}:{type(e).__name__}")
            continue
        if p.returncode == 0:
            detail = (p.stdout or "").strip().replace("\t", " ").replace("\n", " | ")
            return (f"READABLE:{label}", detail[:500])
        err = (p.stderr or "").strip().replace("\t", " ").replace("\n", " | ")
        errors.append(f"{label}:{err[:200]}")
    return ("UNREADABLE", " || ".join(errors)[:500])

def choose_conversion_hint(source: Path, target_rel: str, risk: str, decoder_status: str) -> str:
    if source.suffix.casefold() == Path(target_rel).suffix.casefold():
        return ""
    if risk.startswith("PROTECTED"):
        return "DO_NOT_GENERIC_CONVERT; inspect target BMP header family first"
    if not decoder_status.startswith("READABLE:"):
        return "source not proven decodable; inspect before conversion"
    dst_ext = Path(target_rel).suffix.casefold()
    src_ext = source.suffix.casefold()
    if dst_ext in {".bmp", ".png", ".tga", ".jpg", ".jpeg"} and src_ext in IMAGE_EXTS:
        return "conversion plausible with ImageMagick/ffmpeg; validate dimensions/alpha/header before deploy"
    return "manual conversion review"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument(
        "--missing-tsv",
        default="bugfixes/058_asset_reconciliation/evidence/image_audit/missing_static_images.tsv",
    )
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    missing_tsv = (repo / args.missing_tsv).resolve() if not Path(args.missing_tsv).is_absolute() else Path(args.missing_tsv)
    if not missing_tsv.is_file():
        raise SystemExit(f"ERRO: missing TSV não encontrado: {missing_tsv}")

    roots = [(label, root) for label, root in TRUSTED_ROOTS if root.is_dir()]
    if not roots:
        raise SystemExit("ERRO: nenhum dos dois roots confiáveis existe.")

    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/trusted_asset_hunt"
    outdir.mkdir(parents=True, exist_ok=True)

    # Group duplicate source references into unique missing assets.
    grouped = {}
    with missing_tsv.open("r", encoding="utf-8", errors="replace", newline="") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            target = norm_rel(row.get("normalized") or row.get("literal") or "")
            if not target:
                continue
            g = grouped.setdefault(target.casefold(), {
                "target_rel": target,
                "references": [],
            })
            g["references"].append(
                f'{row.get("source_file","")}:{row.get("line","")}'
            )

    # Index only image files once.
    by_basename = defaultdict(list)
    by_stem = defaultdict(list)
    indexed = []
    for label, root in roots:
        print(f"INDEX {label}: {root}", flush=True)
        for dirpath, _, filenames in os.walk(root):
            d = Path(dirpath)
            for fn in filenames:
                p = d / fn
                if p.suffix.casefold() not in IMAGE_EXTS:
                    continue
                try:
                    rel = p.relative_to(root).as_posix()
                except Exception:
                    continue
                rec = {
                    "root_label": label,
                    "root": root,
                    "path": p,
                    "root_rel": rel,
                    "parts": Path(rel).parts,
                }
                indexed.append(rec)
                by_basename[fn.casefold()].append(rec)
                by_stem[p.stem.casefold()].append(rec)

    print(f"INDEXED_IMAGES={len(indexed)}", flush=True)

    rows = []
    unresolved = []
    exact_copy = []
    conversion = []
    ambiguous = []

    digest_cache = {}

    def digest(p: Path):
        key = str(p)
        if key not in digest_cache:
            digest_cache[key] = sha256(p)
        return digest_cache[key]

    for g in grouped.values():
        target = g["target_rel"]
        tp = Path(target)
        tparts = tp.parts
        tbase = tp.name.casefold()
        tstem = tp.stem.casefold()
        text = tp.suffix.casefold()
        risk = target_risk(target)

        # Same extension + basename, then same stem with alternate extensions.
        candidates = []
        for rec in by_basename.get(tbase, []):
            score = suffix_score(rec["parts"], tparts)
            kind = "SAME_NAME_SAME_EXT"
            if score == len(tparts):
                kind = "EXACT_RELATIVE_SUFFIX"
            elif score >= 2:
                kind = "STRONG_SUFFIX_SAME_NAME"
            candidates.append((kind, score, rec))

        for rec in by_stem.get(tstem, []):
            if rec["path"].suffix.casefold() == text:
                continue
            score = suffix_score(rec["parts"][:-1], tparts[:-1])
            kind = "SAME_STEM_ALT_EXT"
            if score >= max(1, len(tparts) - 1):
                kind = "EXACT_DIR_SAME_STEM_ALT_EXT"
            elif score >= 2:
                kind = "STRONG_SUFFIX_ALT_EXT"
            candidates.append((kind, score, rec))

        if not candidates:
            unresolved.append({
                "target_rel": target,
                "ref_count": len(g["references"]),
                "references": " | ".join(g["references"]),
                "risk": risk,
            })
            continue

        # De-duplicate physical candidates and sort by strongest path evidence first.
        best_by_path = {}
        priority = {
            "EXACT_RELATIVE_SUFFIX": 50,
            "EXACT_DIR_SAME_STEM_ALT_EXT": 45,
            "STRONG_SUFFIX_SAME_NAME": 40,
            "STRONG_SUFFIX_ALT_EXT": 35,
            "SAME_NAME_SAME_EXT": 20,
            "SAME_STEM_ALT_EXT": 10,
        }
        for kind, score, rec in candidates:
            key = str(rec["path"])
            rank = (priority[kind], score)
            if key not in best_by_path or rank > best_by_path[key][0]:
                best_by_path[key] = (rank, kind, score, rec)

        ranked = sorted(best_by_path.values(), key=lambda x: (x[0][0], x[0][1], x[3]["root_label"], x[3]["root_rel"]), reverse=True)
        top_rank = ranked[0][0]
        top = [x for x in ranked if x[0] == top_rank]

        # Multiple top candidates are okay only when byte-identical; still report all.
        top_hashes = {digest(x[3]["path"]) for x in top}
        top_unique_content = len(top_hashes) == 1

        for rank, kind, score, rec in ranked:
            p = rec["path"]
            src_ext = p.suffix.casefold()
            needs_conversion = src_ext != text
            decoder_status, decoder_detail = decoder_probe(p) if needs_conversion else ("NOT_NEEDED", "")
            hint = choose_conversion_hint(p, target, risk, decoder_status)

            row = {
                "target_rel": target,
                "wanted_ext": text,
                "ref_count": len(g["references"]),
                "references": " | ".join(g["references"]),
                "risk": risk,
                "match_kind": kind,
                "suffix_score": score,
                "source_root": rec["root_label"],
                "source_path": str(p),
                "source_root_rel": rec["root_rel"],
                "source_ext": src_ext,
                "source_sha256": digest(p),
                "needs_conversion": "1" if needs_conversion else "0",
                "decoder_status": decoder_status,
                "decoder_detail": decoder_detail,
                "conversion_hint": hint,
                "is_top_rank": "1" if rank == top_rank else "0",
                "top_rank_content_identical": "1" if top_unique_content else "0",
            }
            rows.append(row)

        top_rows = [r for r in rows if r["target_rel"] == target and r["is_top_rank"] == "1"]
        if len(top_rows) == 1 or top_unique_content:
            chosen = top_rows[0]
            if chosen["needs_conversion"] == "0":
                exact_copy.append(chosen)
            else:
                conversion.append(chosen)
        else:
            ambiguous.extend(top_rows)

    fields = [
        "target_rel","wanted_ext","ref_count","references","risk","match_kind","suffix_score",
        "source_root","source_path","source_root_rel","source_ext","source_sha256",
        "needs_conversion","decoder_status","decoder_detail","conversion_hint",
        "is_top_rank","top_rank_content_identical"
    ]

    def write_tsv(name, data, fieldnames=fields):
        p = outdir / name
        with p.open("w", encoding="utf-8", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t", lineterminator="\n")
            w.writeheader()
            w.writerows(data)
        return p

    all_p = write_tsv("all_candidates.tsv", rows)
    copy_p = write_tsv("exact_copy_candidates.tsv", exact_copy)
    conv_p = write_tsv("conversion_candidates.tsv", conversion)
    amb_p = write_tsv("ambiguous_top_candidates.tsv", ambiguous)
    unr_fields = ["target_rel","ref_count","references","risk"]
    unr_p = write_tsv("unresolved.tsv", unresolved, unr_fields)

    summary_p = outdir / "summary.txt"
    with summary_p.open("w", encoding="utf-8") as f:
        f.write(f"missing_input={missing_tsv}\n")
        f.write("trusted_roots:\n")
        for label, root in roots:
            f.write(f"  {label}={root}\n")
        f.write(f"\nUNIQUE_MISSING_TARGETS={len(grouped)}\n")
        f.write(f"INDEXED_IMAGES={len(indexed)}\n")
        f.write(f"EXACT_COPY_CANDIDATES={len(exact_copy)}\n")
        f.write(f"CONVERSION_CANDIDATES={len(conversion)}\n")
        f.write(f"AMBIGUOUS_TOP_TARGETS={len({x['target_rel'] for x in ambiguous})}\n")
        f.write(f"UNRESOLVED={len(unresolved)}\n")

    print("=== TRUSTED ASSET HUNT 058 ===")
    print(f"UNIQUE_MISSING_TARGETS={len(grouped)}")
    print(f"INDEXED_IMAGES={len(indexed)}")
    print(f"EXACT_COPY_CANDIDATES={len(exact_copy)}")
    print(f"CONVERSION_CANDIDATES={len(conversion)}")
    print(f"AMBIGUOUS_TOP_TARGETS={len({x['target_rel'] for x in ambiguous})}")
    print(f"UNRESOLVED={len(unresolved)}")
    print(f"summary={summary_p}")
    print(f"copy={copy_p}")
    print(f"conversion={conv_p}")
    print(f"ambiguous={amb_p}")
    print(f"unresolved={unr_p}")

if __name__ == "__main__":
    main()

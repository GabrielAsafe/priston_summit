#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import shutil
import struct
import subprocess
from collections import Counter, defaultdict
from pathlib import Path

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def identify(path: Path):
    magick = shutil.which("magick")
    ident = shutil.which("identify")
    if magick:
        cmd = [magick, "identify", "-format", "%w\t%h\t%[channels]\t%m", str(path)]
    elif ident:
        cmd = [ident, "-format", "%w\t%h\t%[channels]\t%m", str(path)]
    else:
        return ("", "", "", "NO_IMAGEMAGICK")
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode != 0:
        return ("", "", "", "UNREADABLE")
    parts = p.stdout.strip().split("\t")
    if len(parts) < 4:
        return ("", "", "", "UNREADABLE")
    return tuple(parts[:4])

def parse_bmp(path: Path):
    data = path.read_bytes()
    out = {
        "file_size": len(data),
        "magic_hex": data[:2].hex() if len(data) >= 2 else "",
        "magic_ascii": data[:2].decode("latin1", errors="replace") if len(data) >= 2 else "",
        "bf_size": "",
        "pixel_offset": "",
        "dib_size": "",
        "width": "",
        "height": "",
        "planes": "",
        "bpp": "",
        "compression": "",
        "image_size": "",
        "palette_colors": "",
        "header_class": "TOO_SHORT",
        "header_sha64": hashlib.sha256(data[:64]).hexdigest() if data else "",
    }
    if len(data) < 14:
        return out
    try:
        out["bf_size"] = struct.unpack_from("<I", data, 2)[0]
        out["pixel_offset"] = struct.unpack_from("<I", data, 10)[0]
    except struct.error:
        return out

    if len(data) < 18:
        return out
    try:
        dib = struct.unpack_from("<I", data, 14)[0]
    except struct.error:
        return out
    out["dib_size"] = dib

    if data[:2] != b"BM":
        out["header_class"] = "NON_BM_SIGNATURE"
        return out

    if dib == 12 and len(data) >= 26:
        try:
            out["width"], out["height"], out["planes"], out["bpp"] = struct.unpack_from("<HHHH", data, 18)
            out["compression"] = 0
            out["header_class"] = "OS2_CORE_STANDARD"
        except struct.error:
            pass
        return out

    if dib >= 40 and len(data) >= 54:
        try:
            out["width"] = struct.unpack_from("<i", data, 18)[0]
            out["height"] = struct.unpack_from("<i", data, 22)[0]
            out["planes"] = struct.unpack_from("<H", data, 26)[0]
            out["bpp"] = struct.unpack_from("<H", data, 28)[0]
            out["compression"] = struct.unpack_from("<I", data, 30)[0]
            out["image_size"] = struct.unpack_from("<I", data, 34)[0]
            out["palette_colors"] = struct.unpack_from("<I", data, 46)[0]
        except struct.error:
            return out

        standard_dibs = {40: "INFOHEADER", 52: "V2", 56: "V3", 108: "V4", 124: "V5"}
        if dib in standard_dibs:
            if out["planes"] == 1 and out["compression"] in (0, 1, 2, 3, 4, 5, 6):
                out["header_class"] = f"STANDARD_{standard_dibs[dib]}"
            else:
                out["header_class"] = f"ODD_FIELDS_{standard_dibs[dib]}"
        else:
            out["header_class"] = "UNKNOWN_DIB_SIZE"
        return out

    out["header_class"] = "UNKNOWN_DIB_SIZE"
    return out

def stem_prefix(stem: str) -> str:
    # Prefer icon family prefix such as itDA / itFO.
    s = stem
    for i, c in enumerate(s):
        if c.isdigit():
            return s[:i].casefold()
    # Otherwise use leading alphabetic/underscore/hyphen chunk.
    return s.casefold()

def main():
    ap = argparse.ArgumentParser(description="Audit physical BMP family before DDS/TGA->BMP conversion.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument(
        "--plan",
        default="bugfixes/058_asset_reconciliation/evidence/trusted_asset_hunt_v3/full_path_conversion_review.tsv",
    )
    ap.add_argument("--sample-limit", type=int, default=40)
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    plan = (repo / args.plan).resolve()
    if not client.is_dir():
        raise SystemExit(f"ERRO: cliente ausente: {client}")
    if not plan.is_file():
        raise SystemExit(f"ERRO: plano ausente: {plan}")

    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/bmp_family_audit"
    outdir.mkdir(parents=True, exist_ok=True)

    with plan.open("r", encoding="utf-8", newline="") as f:
        plan_rows = list(csv.DictReader(f, delimiter="\t"))

    grouped = defaultdict(list)
    for r in plan_rows:
        target = r["target_rel"]
        if Path(target).suffix.casefold() == ".bmp":
            grouped[target.casefold()].append(r)

    target_rows = []
    sample_rows = []

    for rows in grouped.values():
        r0 = rows[0]
        target_rel = r0["target_rel"]
        target = client / r0["runtime_destination"]
        target_dir = target.parent
        target_prefix = stem_prefix(target.stem)

        # Source metadata; preserve all alternate source candidates.
        source_meta = []
        for r in rows:
            src = Path(r["source_path"])
            sw, sh, sch, sfmt = identify(src) if src.is_file() else ("", "", "", "MISSING")
            source_meta.append({
                "path": str(src),
                "sha": r["source_sha256"],
                "ext": src.suffix.casefold(),
                "width": sw,
                "height": sh,
                "channels": sch,
                "format": sfmt,
            })

        siblings = []
        if target_dir.is_dir():
            for p in sorted(target_dir.glob("*")):
                if not p.is_file() or p.suffix.casefold() != ".bmp":
                    continue
                siblings.append(p)

        # Prefer same basename-family, then same dimensions, then remaining directory samples.
        same_prefix = [p for p in siblings if stem_prefix(p.stem) == target_prefix]
        source_dims = {(m["width"], m["height"]) for m in source_meta if m["width"] and m["height"]}
        same_dims = []
        other = []
        seen = set()

        def add_group(dest, items):
            for p in items:
                if str(p) not in seen:
                    seen.add(str(p))
                    dest.append(p)

        ordered = []
        add_group(ordered, same_prefix)
        for p in siblings:
            if str(p) in seen:
                continue
            w, h, _, _ = identify(p)
            if (w, h) in source_dims:
                same_dims.append(p)
        add_group(ordered, same_dims)
        add_group(ordered, siblings)
        ordered = ordered[:args.sample_limit]

        profiles = []
        for p in ordered:
            hdr = parse_bmp(p)
            iw, ih, channels, fmt = identify(p)
            prof = {
                "target_rel": target_rel,
                "sample_path": str(p),
                "sample_rel": p.relative_to(client).as_posix(),
                "same_stem_prefix": int(stem_prefix(p.stem) == target_prefix),
                "identify_width": iw,
                "identify_height": ih,
                "identify_channels": channels,
                "identify_format": fmt,
                **hdr,
                "sha256": sha256(p),
            }
            profiles.append(prof)
            sample_rows.append(prof)

        header_classes = Counter(p["header_class"] for p in profiles)
        bpps = Counter(str(p["bpp"]) for p in profiles if p["bpp"] != "")
        dibs = Counter(str(p["dib_size"]) for p in profiles if p["dib_size"] != "")
        compressions = Counter(str(p["compression"]) for p in profiles if p["compression"] != "")
        magics = Counter(p["magic_hex"] for p in profiles if p["magic_hex"])

        standard_count = sum(
            n for k, n in header_classes.items()
            if k.startswith("STANDARD_") or k == "OS2_CORE_STANDARD"
        )
        nonstandard_count = len(profiles) - standard_count

        same_prefix_profiles = [p for p in profiles if p["same_stem_prefix"]]
        sp_header = Counter(p["header_class"] for p in same_prefix_profiles)
        sp_bpp = Counter(str(p["bpp"]) for p in same_prefix_profiles if p["bpp"] != "")
        sp_dib = Counter(str(p["dib_size"]) for p in same_prefix_profiles if p["dib_size"] != "")
        sp_comp = Counter(str(p["compression"]) for p in same_prefix_profiles if p["compression"] != "")

        # Conservative recommendation.
        if not profiles:
            recommendation = "NO_LOCAL_BMP_FAMILY"
        elif same_prefix_profiles and all(
            p["header_class"].startswith("STANDARD_") or p["header_class"] == "OS2_CORE_STANDARD"
            for p in same_prefix_profiles
        ) and len(sp_header) == 1 and len(sp_bpp) == 1 and len(sp_dib) == 1 and len(sp_comp) == 1:
            recommendation = "STANDARD_UNIFORM_SAME_PREFIX_CANDIDATE"
        elif standard_count == len(profiles) and len(header_classes) == 1 and len(bpps) == 1 and len(dibs) == 1 and len(compressions) == 1:
            recommendation = "STANDARD_UNIFORM_DIRECTORY_CANDIDATE"
        elif nonstandard_count:
            recommendation = "PROTECTED_OR_MIXED_REVIEW"
        else:
            recommendation = "MIXED_STANDARD_REVIEW"

        target_rows.append({
            "target_rel": target_rel,
            "runtime_destination": r0["runtime_destination"],
            "source_candidates": " | ".join(m["path"] for m in source_meta),
            "source_exts": " | ".join(m["ext"] for m in source_meta),
            "source_dimensions": " | ".join(f'{m["width"]}x{m["height"]}' for m in source_meta),
            "source_channels": " | ".join(m["channels"] for m in source_meta),
            "target_dir_exists": int(target_dir.is_dir()),
            "sibling_bmp_count": len(siblings),
            "sample_count": len(profiles),
            "same_prefix_sample_count": len(same_prefix_profiles),
            "header_classes": ";".join(f"{k}:{v}" for k,v in header_classes.items()),
            "bpps": ";".join(f"{k}:{v}" for k,v in bpps.items()),
            "dib_sizes": ";".join(f"{k}:{v}" for k,v in dibs.items()),
            "compressions": ";".join(f"{k}:{v}" for k,v in compressions.items()),
            "same_prefix_headers": ";".join(f"{k}:{v}" for k,v in sp_header.items()),
            "same_prefix_bpps": ";".join(f"{k}:{v}" for k,v in sp_bpp.items()),
            "same_prefix_dibs": ";".join(f"{k}:{v}" for k,v in sp_dib.items()),
            "same_prefix_compressions": ";".join(f"{k}:{v}" for k,v in sp_comp.items()),
            "recommendation": recommendation,
        })

    target_fields = [
        "target_rel","runtime_destination","source_candidates","source_exts",
        "source_dimensions","source_channels","target_dir_exists","sibling_bmp_count",
        "sample_count","same_prefix_sample_count","header_classes","bpps","dib_sizes",
        "compressions","same_prefix_headers","same_prefix_bpps","same_prefix_dibs",
        "same_prefix_compressions","recommendation",
    ]
    sample_fields = [
        "target_rel","sample_path","sample_rel","same_stem_prefix",
        "identify_width","identify_height","identify_channels","identify_format",
        "file_size","magic_hex","magic_ascii","bf_size","pixel_offset","dib_size",
        "width","height","planes","bpp","compression","image_size","palette_colors",
        "header_class","header_sha64","sha256",
    ]

    with (outdir / "targets.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=target_fields, delimiter="\t", lineterminator="\n")
        w.writeheader(); w.writerows(target_rows)

    with (outdir / "samples.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=sample_fields, delimiter="\t", lineterminator="\n")
        w.writeheader(); w.writerows(sample_rows)

    counts = Counter(r["recommendation"] for r in target_rows)
    with (outdir / "summary.txt").open("w", encoding="utf-8") as f:
        f.write(f"BMP_TARGETS={len(target_rows)}\n")
        for k in sorted(counts):
            f.write(f"{k}={counts[k]}\n")

    print("=== BMP FAMILY AUDIT 058 ===")
    print(f"BMP_TARGETS={len(target_rows)}")
    for k in sorted(counts):
        print(f"{k}={counts[k]}")
    print()
    for r in target_rows:
        print(
            f"{r['target_rel']}\n"
            f"  src={r['source_exts']} {r['source_dimensions']}\n"
            f"  siblings={r['sibling_bmp_count']} samples={r['sample_count']} same_prefix={r['same_prefix_sample_count']}\n"
            f"  same_prefix_headers={r['same_prefix_headers'] or '-'} "
            f"bpp={r['same_prefix_bpps'] or '-'} dib={r['same_prefix_dibs'] or '-'} "
            f"compression={r['same_prefix_compressions'] or '-'}\n"
            f"  => {r['recommendation']}\n"
        )
    print(f"OUT={outdir}")

if __name__ == "__main__":
    main()

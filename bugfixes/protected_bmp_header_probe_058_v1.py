#!/usr/bin/env python3
from __future__ import annotations
import os
import argparse
import csv
import hashlib
import shutil
import struct
import subprocess
import tempfile
from collections import Counter, defaultdict
from pathlib import Path

def identify(path: Path):
    magick = shutil.which("magick")
    ident = shutil.which("identify")
    if magick:
        cmd = [magick, "identify", "-format", "%w\t%h\t%[channels]\t%m", str(path)]
    elif ident:
        cmd = [ident, "-format", "%w\t%h\t%[channels]\t%m", str(path)]
    else:
        return False, "", "", "", "NO_IMAGEMAGICK"
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode:
        return False, "", "", "", p.stderr.strip().replace("\n", " | ")
    parts = p.stdout.strip().split("\t")
    if len(parts) < 4:
        return False, "", "", "", p.stdout.strip()
    return True, parts[0], parts[1], parts[2], parts[3]

def u16(b, off):
    return struct.unpack_from("<H", b, off)[0] if len(b) >= off+2 else None

def u32(b, off):
    return struct.unpack_from("<I", b, off)[0] if len(b) >= off+4 else None

def i32(b, off):
    return struct.unpack_from("<i", b, off)[0] if len(b) >= off+4 else None

def parse_header(path: Path):
    data = path.read_bytes()
    return {
        "actual_size": len(data),
        "sig_hex": data[:2].hex() if len(data) >= 2 else "",
        "sig_ascii": data[:2].decode("latin1", errors="replace") if len(data) >= 2 else "",
        "bfSize": u32(data, 2),
        "reserved1": u16(data, 6),
        "reserved2": u16(data, 8),
        "bfOffBits": u32(data, 10),
        "dibSize": u32(data, 14),
        "width": i32(data, 18),
        "height": i32(data, 22),
        "planes": u16(data, 26),
        "bpp": u16(data, 28),
        "compression": u32(data, 30),
        "imageSize": u32(data, 34),
        "xppm": i32(data, 38),
        "yppm": i32(data, 42),
        "clrUsed": u32(data, 46),
        "clrImportant": u32(data, 50),
        "header_hex_64": data[:64].hex(),
    }

def plausible(h):
    if h["dibSize"] not in (12, 40, 52, 56, 108, 124):
        return False
    if not h["width"] or not h["height"]:
        return False
    if h["planes"] != 1:
        return False
    if h["bpp"] not in (1, 4, 8, 16, 24, 32):
        return False
    if h["compression"] not in (0, 1, 2, 3, 4, 5, 6):
        return False
    if h["bfOffBits"] is None or h["bfOffBits"] < 14:
        return False
    if h["bfOffBits"] > h["actual_size"]:
        return False
    return True

def bm_signature_probe(path: Path):
    raw = bytearray(path.read_bytes())
    if len(raw) < 54:
        return False, "", "", "", "TOO_SHORT"
    raw[0:2] = b"BM"
    fd, name = tempfile.mkstemp(prefix="summit_bmprobe_", suffix=".bmp")
    os.close(fd)
    tmp = Path(name)
    try:
        tmp.write_bytes(raw)
        return identify(tmp)
    finally:
        try: tmp.unlink()
        except OSError: pass

def family_prefix(stem: str) -> str:
    for i, c in enumerate(stem):
        if c.isdigit():
            return stem[:i].casefold()
    return stem.casefold()

def main():
    ap = argparse.ArgumentParser(description="Probe Summit protected BMP headers without modifying runtime.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument(
        "--plan",
        default="bugfixes/058_asset_reconciliation/evidence/trusted_asset_hunt_v3/full_path_conversion_review.tsv"
    )
    ap.add_argument("--sample-limit", type=int, default=80)
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    plan = (repo / args.plan).resolve()
    if not client.is_dir():
        raise SystemExit(f"ERRO: cliente ausente: {client}")
    if not plan.is_file():
        raise SystemExit(f"ERRO: plano ausente: {plan}")

    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/protected_bmp_probe"
    outdir.mkdir(parents=True, exist_ok=True)

    with plan.open("r", encoding="utf-8", newline="") as f:
        plan_rows = list(csv.DictReader(f, delimiter="\t"))

    targets = {}
    for r in plan_rows:
        if Path(r["target_rel"]).suffix.casefold() != ".bmp":
            continue
        targets.setdefault(r["target_rel"].casefold(), r)

    detail_rows = []
    summary_rows = []

    for r in targets.values():
        target_rel = r["target_rel"]
        target = client / r["runtime_destination"]
        d = target.parent
        prefix = family_prefix(target.stem)

        siblings = []
        if d.is_dir():
            siblings = sorted(
                p for p in d.iterdir()
                if p.is_file() and p.suffix.casefold() == ".bmp"
            )

        same = [p for p in siblings if family_prefix(p.stem) == prefix]
        if same:
            samples = same[:args.sample_limit]
            sample_scope = "SAME_PREFIX"
        else:
            samples = siblings[:args.sample_limit]
            sample_scope = "DIRECTORY"

        sigs = Counter()
        dibs = Counter()
        bpps = Counter()
        comps = Counter()
        offsets = Counter()
        plaus = Counter()
        bmprobe = Counter()
        size_match = Counter()

        for p in samples:
            h = parse_header(p)
            raw_ok, rw, rh, rch, rfmt_or_err = identify(p)
            bm_ok, bw, bh, bch, bfmt_or_err = bm_signature_probe(p)

            is_plaus = plausible(h)
            bfsize_matches = (h["bfSize"] == h["actual_size"])

            sigs[h["sig_hex"]] += 1
            dibs[str(h["dibSize"])] += 1
            bpps[str(h["bpp"])] += 1
            comps[str(h["compression"])] += 1
            offsets[str(h["bfOffBits"])] += 1
            plaus[str(int(is_plaus))] += 1
            bmprobe[str(int(bm_ok))] += 1
            size_match[str(int(bfsize_matches))] += 1

            detail_rows.append({
                "target_rel": target_rel,
                "scope": sample_scope,
                "sample_rel": p.relative_to(client).as_posix(),
                "actual_size": h["actual_size"],
                "sig_hex": h["sig_hex"],
                "sig_ascii": h["sig_ascii"],
                "bfSize": h["bfSize"],
                "bfSize_matches_actual": int(bfsize_matches),
                "reserved1": h["reserved1"],
                "reserved2": h["reserved2"],
                "bfOffBits": h["bfOffBits"],
                "dibSize": h["dibSize"],
                "width": h["width"],
                "height": h["height"],
                "planes": h["planes"],
                "bpp": h["bpp"],
                "compression": h["compression"],
                "imageSize": h["imageSize"],
                "clrUsed": h["clrUsed"],
                "plausible_standard_fields": int(is_plaus),
                "raw_identify_ok": int(raw_ok),
                "raw_identify": f"{rw}x{rh} {rch} {rfmt_or_err}" if raw_ok else rfmt_or_err,
                "bm_signature_probe_ok": int(bm_ok),
                "bm_signature_probe": f"{bw}x{bh} {bch} {bfmt_or_err}" if bm_ok else bfmt_or_err,
                "header_hex_64": h["header_hex_64"],
            })

        if not samples:
            verdict = "NO_SAMPLES"
        elif (
            len(sigs) == 1
            and list(plaus.keys()) == ["1"]
            and list(bmprobe.keys()) == ["1"]
            and list(size_match.keys()) == ["1"]
        ):
            verdict = "SIGNATURE_ONLY_PROTECTION_PROVED"
        elif all(k == "1" for k in plaus) and all(k == "1" for k in bmprobe):
            verdict = "FIELDS_STANDARD_BM_PATCH_READABLE_MIXED_HEADER"
        elif any(k == "1" for k in bmprobe):
            verdict = "PARTIAL_BM_SIGNATURE_RECOVERY"
        else:
            verdict = "MORE_THAN_SIGNATURE_PROTECTION"

        summary_rows.append({
            "target_rel": target_rel,
            "sample_scope": sample_scope,
            "sample_count": len(samples),
            "signatures": ";".join(f"{k}:{v}" for k,v in sigs.items()),
            "dib_sizes": ";".join(f"{k}:{v}" for k,v in dibs.items()),
            "bpps": ";".join(f"{k}:{v}" for k,v in bpps.items()),
            "compressions": ";".join(f"{k}:{v}" for k,v in comps.items()),
            "pixel_offsets": ";".join(f"{k}:{v}" for k,v in offsets.items()),
            "plausible_fields": ";".join(f"{k}:{v}" for k,v in plaus.items()),
            "bfSize_matches_actual": ";".join(f"{k}:{v}" for k,v in size_match.items()),
            "bm_signature_probe": ";".join(f"{k}:{v}" for k,v in bmprobe.items()),
            "verdict": verdict,
        })

    detail_fields = [
        "target_rel","scope","sample_rel","actual_size","sig_hex","sig_ascii",
        "bfSize","bfSize_matches_actual","reserved1","reserved2","bfOffBits",
        "dibSize","width","height","planes","bpp","compression","imageSize",
        "clrUsed","plausible_standard_fields","raw_identify_ok","raw_identify",
        "bm_signature_probe_ok","bm_signature_probe","header_hex_64"
    ]
    summary_fields = [
        "target_rel","sample_scope","sample_count","signatures","dib_sizes",
        "bpps","compressions","pixel_offsets","plausible_fields",
        "bfSize_matches_actual","bm_signature_probe","verdict"
    ]

    with (outdir / "samples.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=detail_fields, delimiter="\t", lineterminator="\n")
        w.writeheader(); w.writerows(detail_rows)

    with (outdir / "targets.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=summary_fields, delimiter="\t", lineterminator="\n")
        w.writeheader(); w.writerows(summary_rows)

    vc = Counter(r["verdict"] for r in summary_rows)
    with (outdir / "summary.txt").open("w", encoding="utf-8") as f:
        f.write(f"TARGETS={len(summary_rows)}\n")
        for k in sorted(vc):
            f.write(f"{k}={vc[k]}\n")

    print("=== PROTECTED BMP HEADER PROBE 058 ===")
    print(f"TARGETS={len(summary_rows)}")
    for k in sorted(vc):
        print(f"{k}={vc[k]}")
    print()
    for r in summary_rows:
        print(r["target_rel"])
        print(f"  samples={r['sample_count']} scope={r['sample_scope']}")
        print(f"  signatures={r['signatures']}")
        print(f"  dib={r['dib_sizes']} bpp={r['bpps']} compression={r['compressions']}")
        print(f"  offbits={r['pixel_offsets']}")
        print(f"  plausible={r['plausible_fields']} size_match={r['bfSize_matches_actual']}")
        print(f"  BM-only probe={r['bm_signature_probe']}")
        print(f"  => {r['verdict']}")
        print()
    print(f"OUT={outdir}")

if __name__ == "__main__":
    main()

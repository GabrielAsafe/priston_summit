#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import shutil
import struct
import subprocess
import tempfile
from collections import defaultdict
from pathlib import Path

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def identify(path: Path):
    magick = shutil.which("magick")
    identify_bin = shutil.which("identify")
    if magick:
        cmd = [magick, "identify", "-format", "%w\t%h\t%[channels]\t%m", str(path)]
    elif identify_bin:
        cmd = [identify_bin, "-format", "%w\t%h\t%[channels]\t%m", str(path)]
    else:
        raise SystemExit("ERRO: ImageMagick (magick/identify) não encontrado.")
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode:
        raise RuntimeError(f"identify falhou: {path}: {p.stderr.strip()}")
    parts = p.stdout.strip().split("\t")
    return int(parts[0]), int(parts[1]), parts[2], parts[3]

def tga_header(path: Path):
    b = path.read_bytes()[:18]
    if len(b) < 18:
        raise RuntimeError(f"TGA curto demais: {path}")
    idlen, cmap, image_type = b[0], b[1], b[2]
    width = struct.unpack_from("<H", b, 12)[0]
    height = struct.unpack_from("<H", b, 14)[0]
    bpp = b[16]
    desc = b[17]
    return {
        "idlen": idlen, "cmap": cmap, "image_type": image_type,
        "width": width, "height": height, "bpp": bpp, "descriptor": desc,
    }

def convert_dds_to_tga(src: Path, dst: Path):
    magick = shutil.which("magick")
    convert_bin = shutil.which("convert")
    if magick:
        cmd = [
            magick, str(src),
            "-alpha", "on",
            "-depth", "8",
            "-type", "TrueColorAlpha",
            "-define", "tga:compression=none",
            f"TGA:{dst}",
        ]
    elif convert_bin:
        cmd = [
            convert_bin, str(src),
            "-alpha", "on",
            "-depth", "8",
            "-type", "TrueColorAlpha",
            "-define", "tga:compression=none",
            f"TGA:{dst}",
        ]
    else:
        raise SystemExit("ERRO: ImageMagick não encontrado.")
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode:
        raise RuntimeError(f"conversão falhou: {src}: {p.stderr.strip()}")

def main():
    ap = argparse.ArgumentParser(description="Convert only unambiguous NORMAL DDS->TGA full-path candidates.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    ap.add_argument(
        "--plan",
        default="bugfixes/058_asset_reconciliation/evidence/trusted_asset_hunt_v3/full_path_conversion_review.tsv"
    )
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    plan = (repo / args.plan).resolve()
    evid = repo / "bugfixes/058_asset_reconciliation/evidence/tga_conversion"
    manifest = evid / "manifest.tsv"

    if args.action == "rollback":
        if not manifest.is_file():
            raise SystemExit("ERRO: manifest ausente.")
        removed = 0
        with manifest.open("r", encoding="utf-8", newline="") as f:
            for r in csv.DictReader(f, delimiter="\t"):
                if r["action"] != "CONVERTED":
                    continue
                dst = client / r["runtime_destination"]
                if dst.is_file() and sha256(dst) == r["output_sha256"]:
                    dst.unlink()
                    removed += 1
        print(f"OK: rollback removeu {removed} TGA convertidos.")
        return

    if not client.is_dir():
        raise SystemExit(f"ERRO: cliente ausente: {client}")
    if not plan.is_file():
        raise SystemExit(f"ERRO: plano ausente: {plan}")

    grouped = defaultdict(list)
    with plan.open("r", encoding="utf-8", newline="") as f:
        for r in csv.DictReader(f, delimiter="\t"):
            target = r["target_rel"]
            if (
                r.get("classification") == "FULL_PATH_CONVERSION_REVIEW"
                and r.get("risk") == "NORMAL"
                and Path(target).suffix.casefold() == ".tga"
                and Path(r["source_path"]).suffix.casefold() == ".dds"
            ):
                grouped[target.casefold()].append(r)

    audit_rows = []
    eligible = []

    for rows in grouped.values():
        target = rows[0]["target_rel"]
        # Unique physical source/content only.
        paths = {r["source_path"] for r in rows}
        hashes = {r["source_sha256"] for r in rows}
        if len(paths) != 1 or len(hashes) != 1:
            audit_rows.append({
                "action": "AMBIGUOUS",
                "target_rel": target,
                "runtime_destination": rows[0]["runtime_destination"],
                "source_path": " | ".join(sorted(paths)),
                "source_sha256": " | ".join(sorted(hashes)),
                "source_width": "", "source_height": "",
                "output_width": "", "output_height": "",
                "tga_image_type": "", "tga_bpp": "", "tga_descriptor": "",
                "output_sha256": "",
            })
            continue

        r = rows[0]
        src = Path(r["source_path"])
        dst = client / r["runtime_destination"]

        if not src.is_file():
            action = "SOURCE_MISSING"
        elif sha256(src) != r["source_sha256"]:
            action = "SOURCE_HASH_CHANGED"
        elif dst.exists():
            action = "DEST_EXISTS"
        else:
            action = "CONVERT"

        sw = sh = ""
        try:
            if src.is_file():
                sw, sh, _, _ = identify(src)
        except Exception:
            action = "SOURCE_UNREADABLE"

        row = {
            "action": action,
            "target_rel": target,
            "runtime_destination": r["runtime_destination"],
            "source_path": r["source_path"],
            "source_sha256": r["source_sha256"],
            "source_width": sw, "source_height": sh,
            "output_width": "", "output_height": "",
            "tga_image_type": "", "tga_bpp": "", "tga_descriptor": "",
            "output_sha256": "",
        }
        audit_rows.append(row)
        if action == "CONVERT":
            eligible.append(row)

    evid.mkdir(parents=True, exist_ok=True)
    fields = [
        "action","target_rel","runtime_destination","source_path","source_sha256",
        "source_width","source_height","output_width","output_height",
        "tga_image_type","tga_bpp","tga_descriptor","output_sha256"
    ]
    with (evid / "audit.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader(); w.writerows(audit_rows)

    print("=== DDS -> TGA SAFE CONVERSION 058 ===")
    for key in ["CONVERT","DEST_EXISTS","SOURCE_MISSING","SOURCE_HASH_CHANGED","SOURCE_UNREADABLE","AMBIGUOUS"]:
        print(f"{key}={sum(r['action']==key for r in audit_rows)}")

    if args.action == "audit":
        return

    blockers = [r for r in audit_rows if r["action"] != "CONVERT"]
    if blockers:
        for r in blockers:
            print(f"BLOCKER: {r['target_rel']}: {r['action']}")
        raise SystemExit("ERRO: há blocker; nada convertido.")

    applied = []
    created = []
    try:
        for r in eligible:
            src = Path(r["source_path"])
            dst = client / r["runtime_destination"]
            if dst.exists():
                raise RuntimeError(f"destino apareceu durante apply: {dst}")
            dst.parent.mkdir(parents=True, exist_ok=True)

            with tempfile.NamedTemporaryFile(
                prefix="summit_tga_", suffix=".tga", dir=str(dst.parent), delete=False
            ) as tf:
                tmp = Path(tf.name)

            try:
                convert_dds_to_tga(src, tmp)
                sw, sh, _, _ = identify(src)
                ow, oh, _, fmt = identify(tmp)
                hdr = tga_header(tmp)

                if (sw, sh) != (ow, oh):
                    raise RuntimeError(f"dimensão mudou {sw}x{sh} -> {ow}x{oh}")
                if (hdr["width"], hdr["height"]) != (sw, sh):
                    raise RuntimeError("header TGA diverge das dimensões")
                if hdr["cmap"] != 0:
                    raise RuntimeError(f"TGA color-mapped inesperado: cmap={hdr['cmap']}")
                if hdr["image_type"] != 2:
                    raise RuntimeError(f"TGA não é true-color uncompressed: type={hdr['image_type']}")
                if hdr["bpp"] != 32:
                    raise RuntimeError(f"TGA não é 32-bit: bpp={hdr['bpp']}")

                shutil.move(str(tmp), str(dst))
                created.append(dst)

                out_hash = sha256(dst)
                outrow = dict(r)
                outrow.update({
                    "action": "CONVERTED",
                    "output_width": ow,
                    "output_height": oh,
                    "tga_image_type": hdr["image_type"],
                    "tga_bpp": hdr["bpp"],
                    "tga_descriptor": f"0x{hdr['descriptor']:02x}",
                    "output_sha256": out_hash,
                })
                applied.append(outrow)
            finally:
                if tmp.exists():
                    tmp.unlink()
    except Exception:
        # Transaction-like cleanup: remove outputs produced in this apply.
        for p in created:
            try:
                p.unlink()
            except OSError:
                pass
        raise

    with manifest.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader(); w.writerows(applied)

    print(f"OK: convertidos {len(applied)} DDS -> TGA.")
    for r in applied:
        print(
            f"{r['runtime_destination']} "
            f"{r['output_width']}x{r['output_height']} "
            f"type={r['tga_image_type']} bpp={r['tga_bpp']} "
            f"sha256={r['output_sha256']}"
        )

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import shutil
import struct
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

SOURCE = Path("/home/gabriel/armazem/priston/ExMachina/client/image/Sinimage/Items/Premium/itBI283.dds")
TARGET_REL = Path("image/Sinimage/Items/Premium/itBI283.bmp")
EXPECTED_SOURCE_SHA256 = "20c64fb21c28943223010a98ca4b12c1a70614f036b59d800cf0f22ea6fa8fd1"

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def u16(b, off): return struct.unpack_from("<H", b, off)[0]
def u32(b, off): return struct.unpack_from("<I", b, off)[0]
def i32(b, off): return struct.unpack_from("<i", b, off)[0]

def identify(p: Path):
    magick = shutil.which("magick")
    ident = shutil.which("identify")
    if magick:
        cmd = [magick, "identify", "-format", "%w\t%h\t%[channels]\t%m", str(p)]
    elif ident:
        cmd = [ident, "-format", "%w\t%h\t%[channels]\t%m", str(p)]
    else:
        raise RuntimeError("ImageMagick não encontrado")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if r.returncode:
        raise RuntimeError(r.stderr.strip())
    parts = r.stdout.strip().split("\t")
    if len(parts) < 4:
        raise RuntimeError(f"identify inesperado: {r.stdout!r}")
    return int(parts[0]), int(parts[1]), parts[2], parts[3]

def convert_bmp3(src: Path, dst: Path):
    magick = shutil.which("magick")
    convert = shutil.which("convert")
    if magick:
        cmd = [magick, str(src), "-alpha", "off", "-depth", "8", "-type", "TrueColor", f"BMP3:{dst}"]
    elif convert:
        cmd = [convert, str(src), "-alpha", "off", "-depth", "8", "-type", "TrueColor", f"BMP3:{dst}"]
    else:
        raise RuntimeError("ImageMagick não encontrado")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if r.returncode:
        raise RuntimeError(r.stderr.strip())

def parse_bmp(p: Path):
    b = p.read_bytes()
    if len(b) < 54:
        return None
    return {
        "path": p,
        "actual": len(b),
        "sig": bytes(b[:2]),
        "bfSize": u32(b, 2),
        "reserved1": u16(b, 6),
        "reserved2": u16(b, 8),
        "offbits": u32(b, 10),
        "dib": u32(b, 14),
        "width": i32(b, 18),
        "height": i32(b, 22),
        "planes": u16(b, 26),
        "bpp": u16(b, 28),
        "compression": u32(b, 30),
    }

def family_key(r):
    if not r:
        return ("INVALID",)
    if (
        r["sig"] == b"BM"
        and r["bfSize"] == r["actual"]
        and r["reserved1"] == 0
        and r["reserved2"] == 0
        and r["offbits"] == 54
        and r["dib"] == 40
        and r["planes"] == 1
        and r["bpp"] == 24
        and r["compression"] == 0
    ):
        return ("BMP_STANDARD_24BPP",)

    if (
        r["sig"] != b"BM"
        and r["dib"] == 40
        and r["planes"] == 1
        and r["bpp"] == 24
        and r["compression"] == 0
    ):
        return (
            "BMP_PROTECTED_HEADER14",
            r["sig"],
            r["bfSize"] - r["actual"],
            r["reserved1"],
            r["reserved2"],
            r["offbits"],
        )

    return (
        "OTHER",
        r["sig"], r["dib"], r["bpp"], r["compression"], r["offbits"]
    )

def validate_standard(p: Path, w: int, h: int):
    r = parse_bmp(p)
    if not r:
        raise RuntimeError("BMP3 inválido")
    checks = [
        r["sig"] == b"BM",
        r["bfSize"] == r["actual"],
        r["reserved1"] == 0,
        r["reserved2"] == 0,
        r["offbits"] == 54,
        r["dib"] == 40,
        abs(r["width"]) == w,
        abs(r["height"]) == h,
        r["planes"] == 1,
        r["bpp"] == 24,
        r["compression"] == 0,
    ]
    if not all(checks):
        raise RuntimeError(f"BMP3 estruturalmente incompatível: {r}")
    return r

def protect(std_bytes: bytes, fam_key):
    _, sig, delta, reserved1, reserved2, offbits = fam_key
    b = bytearray(std_bytes)
    struct.pack_into("<2s", b, 0, sig)
    struct.pack_into("<I", b, 2, len(b) + delta)
    struct.pack_into("<H", b, 6, reserved1)
    struct.pack_into("<H", b, 8, reserved2)
    struct.pack_into("<I", b, 10, offbits)
    return bytes(b)

def normalize(prot: bytes):
    b = bytearray(prot)
    dib = u32(b, 14)
    b[0:2] = b"BM"
    struct.pack_into("<I", b, 2, len(b))
    struct.pack_into("<H", b, 6, 0)
    struct.pack_into("<H", b, 8, 0)
    struct.pack_into("<I", b, 10, 14 + dib)
    return bytes(b)

def main():
    ap = argparse.ArgumentParser(description="Experimental BI283 restore from trusted DDS.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    premium_dir = client / "image/Sinimage/Items/Premium"
    target = client / TARGET_REL
    evid = repo / "bugfixes/058_asset_reconciliation/evidence/bi283_experimental"
    evid.mkdir(parents=True, exist_ok=True)
    manifest = evid / "manifest.tsv"

    if args.action == "rollback":
        if not manifest.is_file():
            raise SystemExit("ERRO: manifest ausente")
        with manifest.open(encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f, delimiter="\t"))
        if len(rows) != 1:
            raise SystemExit("ERRO: manifest inesperado")
        r = rows[0]
        if not target.exists():
            print("OK: target já ausente")
            return
        cur = sha256(target)
        if cur != r["target_sha256"]:
            raise SystemExit("ERRO: rollback recusado; target mudou")
        target.unlink()
        print(f"OK: rollback removeu somente {target}")
        return

    if not SOURCE.is_file():
        raise SystemExit(f"ERRO: source ausente: {SOURCE}")
    if target.exists():
        raise SystemExit(f"ERRO: target já existe: {target}")
    if not premium_dir.is_dir():
        raise SystemExit(f"ERRO: diretório Premium ausente: {premium_dir}")

    src_sha = sha256(SOURCE)
    if src_sha != EXPECTED_SOURCE_SHA256:
        raise SystemExit(f"ERRO: source SHA mudou: {src_sha}")

    sw, sh, sch, sfmt = identify(SOURCE)
    print("=== BI283 BMP FAMILY EXPERIMENT 058 ===")
    print(f"source={SOURCE}")
    print(f"source_sha256={src_sha}")
    print(f"source_identify={sw}x{sh} {sch} {sfmt}")
    print(f"target={target}")

    siblings = []
    for p in sorted(premium_dir.glob("*.bmp")):
        r = parse_bmp(p)
        if not r:
            continue
        if abs(r["width"]) == sw and abs(r["height"]) == sh:
            siblings.append(r)

    print(f"same_dimension_siblings={len(siblings)}")
    if not siblings:
        raise SystemExit("ERRO: nenhum BMP sibling Premium com mesma dimensão")

    fam_counts = Counter(family_key(r) for r in siblings)
    print("family_counts:")
    for k, n in fam_counts.most_common():
        print(f"  {k[0]}={n}")

    if len(fam_counts) != 1:
        raise SystemExit("ERRO: siblings de mesma dimensão pertencem a famílias BMP diferentes")

    fam = next(iter(fam_counts))
    family_name = fam[0]
    if family_name not in {"BMP_STANDARD_24BPP", "BMP_PROTECTED_HEADER14"}:
        raise SystemExit(f"ERRO: família não suportada: {fam}")

    print(f"selected_family={family_name}")
    if family_name == "BMP_PROTECTED_HEADER14":
        _, sig, delta, r1, r2, off = fam
        print(f"family_signature={sig.hex()} ({sig.decode('latin1', errors='replace')})")
        print(f"family_bfSize_delta=0x{delta:08x}")
        print(f"family_reserved1=0x{r1:04x}")
        print(f"family_reserved2=0x{r2:04x}")
        print(f"family_offbits=0x{off:08x}")

    with tempfile.TemporaryDirectory(prefix="summit_bi283_") as td:
        td = Path(td)
        standard = td / "itBI283.standard.bmp"
        final = td / "itBI283.final.bmp"
        normalized = td / "itBI283.normalized.bmp"

        convert_bmp3(SOURCE, standard)
        validate_standard(standard, sw, sh)
        std_bytes = standard.read_bytes()

        if family_name == "BMP_STANDARD_24BPP":
            final_bytes = std_bytes
            normalized_bytes = std_bytes
        else:
            final_bytes = protect(std_bytes, fam)
            if final_bytes[14:] != std_bytes[14:]:
                raise SystemExit("ERRO: proteção alterou DIB/pixels")
            normalized_bytes = normalize(final_bytes)
            if normalized_bytes[14:] != std_bytes[14:]:
                raise SystemExit("ERRO: normalização alterou DIB/pixels")

        final.write_bytes(final_bytes)
        normalized.write_bytes(normalized_bytes)

        nw, nh, nch, nfmt = identify(normalized)
        if (nw, nh) != (sw, sh):
            raise SystemExit("ERRO: normalized dimension mismatch")

        final_hash = sha256(final)
        normalized_hash = sha256(normalized)

        evidence_final = evid / "itBI283.final.bmp"
        evidence_norm = evid / "itBI283.normalized-for-validation.bmp"
        evidence_final.write_bytes(final_bytes)
        evidence_norm.write_bytes(normalized_bytes)

        print(f"final_size={len(final_bytes)}")
        print(f"final_sha256={final_hash}")
        print(f"normalized_sha256={normalized_hash}")
        print(f"normalized_identify={nw}x{nh} {nch} {nfmt}")
        print("STRUCTURAL_VALIDATION=PASS")
        print("DIB_AND_PIXELS_UNCHANGED=YES")

        if args.action == "audit":
            print("APPLY=NOT_RUN")
            return

        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists():
            raise SystemExit("ERRO: target apareceu durante apply")
        shutil.copy2(final, target)
        target_hash = sha256(target)
        if target_hash != final_hash:
            target.unlink(missing_ok=True)
            raise SystemExit("ERRO: SHA runtime divergiu")

        fields = [
            "target","source","source_sha256","target_sha256",
            "family","width","height"
        ]
        with manifest.open("w", encoding="utf-8", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
            w.writeheader()
            w.writerow({
                "target": str(target),
                "source": str(SOURCE),
                "source_sha256": src_sha,
                "target_sha256": target_hash,
                "family": family_name,
                "width": sw,
                "height": sh,
            })

        print(f"OK: criado SOMENTE {target}")
        print("RUNTIME_TEST=READY")

if __name__ == "__main__":
    main()

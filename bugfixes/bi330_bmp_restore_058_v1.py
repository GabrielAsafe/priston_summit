#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import re
import shutil
import struct
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

SOURCE = Path("/home/gabriel/armazem/priston/ExMachina/client/image/Sinimage/Items/Premium/itBI330.dds")
TARGET_REL = Path("image/Sinimage/Items/Premium/itBI330.bmp")
EXPECTED_SOURCE_SHA256 = "341ae500c6ed8cdd231cc7064b8472a5a9978a3ef6bcc597f61f2bdf54b2d25d"
TARGET_ID = 330
BI_RE = re.compile(r"^itbi(\d+)\.bmp$", re.I)

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
        and r["bpp"] in (24, 32)
        and r["compression"] == 0
    ):
        return ("BMP_STANDARD_DIB40", r["bpp"])

    if (
        r["sig"] != b"BM"
        and r["dib"] == 40
        and r["planes"] == 1
        and r["bpp"] in (24, 32)
        and r["compression"] == 0
    ):
        return (
            "BMP_PROTECTED_HEADER14",
            r["sig"],
            r["bfSize"] - r["actual"],
            r["reserved1"],
            r["reserved2"],
            r["offbits"],
            r["bpp"],
        )

    return (
        "OTHER",
        r["sig"], r["dib"], r["bpp"], r["compression"], r["offbits"]
    )

def fam_desc(k):
    if k[0] == "BMP_PROTECTED_HEADER14":
        return (
            f"{k[0]} {k[6]}bpp sig={k[1].hex()} "
            f"delta=0x{k[2]:08x} r1=0x{k[3]:04x} "
            f"r2=0x{k[4]:04x} off=0x{k[5]:08x}"
        )
    if k[0] == "BMP_STANDARD_DIB40":
        return f"{k[0]} {k[1]}bpp"
    return str(k)

def image_to_rgba_raw(src: Path, w: int, h: int) -> bytes:
    magick = shutil.which("magick")
    convert = shutil.which("convert")
    if magick:
        cmd = [magick, str(src), "-alpha", "on", "-depth", "8", "rgba:-"]
    elif convert:
        cmd = [convert, str(src), "-alpha", "on", "-depth", "8", "rgba:-"]
    else:
        raise RuntimeError("ImageMagick não encontrado")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if r.returncode:
        raise RuntimeError(r.stderr.decode(errors="replace").strip())
    raw = r.stdout
    expected = w * h * 4
    if len(raw) != expected:
        raise RuntimeError(f"RGBA raw size inesperado: {len(raw)} != {expected}")
    return raw

def build_standard_bmp32_from_rgba(raw: bytes, w: int, h: int) -> bytes:
    # ImageMagick raw RGBA is top-to-bottom. Legacy BMP with positive height is bottom-up.
    row_bytes = w * 4
    pixels = bytearray()

    for y in range(h - 1, -1, -1):
        row = raw[y * row_bytes:(y + 1) * row_bytes]
        for x in range(w):
            r, g, b, a = row[x*4:x*4+4]
            pixels += bytes((b, g, r, a))

    offbits = 54
    size_image = len(pixels)
    file_size = offbits + size_image

    file_header = struct.pack(
        "<2sIHHI",
        b"BM", file_size, 0, 0, offbits
    )
    dib = struct.pack(
        "<IiiHHIIiiII",
        40, w, h, 1, 32, 0, size_image,
        2835, 2835, 0, 0
    )
    return file_header + dib + pixels

def build_standard_bmp24(src: Path, out: Path):
    magick = shutil.which("magick")
    convert = shutil.which("convert")
    if magick:
        cmd = [magick, str(src), "-alpha", "off", "-depth", "8",
               "-type", "TrueColor", f"BMP3:{out}"]
    elif convert:
        cmd = [convert, str(src), "-alpha", "off", "-depth", "8",
               "-type", "TrueColor", f"BMP3:{out}"]
    else:
        raise RuntimeError("ImageMagick não encontrado")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if r.returncode:
        raise RuntimeError(r.stderr.strip())
    return out.read_bytes()

def validate_standard_bytes(b: bytes, w: int, h: int, bpp: int):
    if len(b) < 54:
        raise RuntimeError("BMP standard curto")
    checks = {
        "sig": b[:2] == b"BM",
        "bfSize": u32(b, 2) == len(b),
        "reserved1": u16(b, 6) == 0,
        "reserved2": u16(b, 8) == 0,
        "offbits": u32(b, 10) == 54,
        "dib": u32(b, 14) == 40,
        "width": abs(i32(b, 18)) == w,
        "height": abs(i32(b, 22)) == h,
        "planes": u16(b, 26) == 1,
        "bpp": u16(b, 28) == bpp,
        "compression": u32(b, 30) == 0,
    }
    if not all(checks.values()):
        raise RuntimeError(f"BMP standard inválido: {checks}")

def protect(std_bytes: bytes, fam):
    _, sig, delta, r1, r2, offbits, _bpp = fam
    b = bytearray(std_bytes)
    b[0:2] = sig
    struct.pack_into("<I", b, 2, len(b) + delta)
    struct.pack_into("<H", b, 6, r1)
    struct.pack_into("<H", b, 8, r2)
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

def choose_bi_family(premium_dir: Path, w: int, h: int):
    siblings = []
    for p in sorted(premium_dir.iterdir(), key=lambda x: x.name.casefold()):
        if not p.is_file():
            continue
        m = BI_RE.match(p.name)
        if not m:
            continue
        item_id = int(m.group(1))
        if item_id == TARGET_ID:
            continue
        r = parse_bmp(p)
        if not r:
            continue
        if abs(r["width"]) != w or abs(r["height"]) != h:
            continue
        siblings.append((item_id, p, r, family_key(r)))

    print(f"same_dimension_itBI_siblings={len(siblings)}")
    if not siblings:
        raise SystemExit("ERRO: nenhum irmão itBI*.bmp com a mesma dimensão")

    counts = Counter(k for _, _, _, k in siblings)
    print("itBI_family_counts:")
    for k, n in counts.most_common():
        print(f"  {fam_desc(k)} = {n}")

    lowers = [x for x in siblings if x[0] < TARGET_ID]
    uppers = [x for x in siblings if x[0] > TARGET_ID]
    if not lowers or not uppers:
        raise SystemExit("ERRO: sem vizinhos dos dois lados de BI330")

    lower = max(lowers, key=lambda x: x[0])
    upper = min(uppers, key=lambda x: x[0])

    ordered = sorted(siblings, key=lambda x: (abs(x[0] - TARGET_ID), x[0]))
    print("nearest_itBI_siblings:")
    for item_id, p, r, k in ordered[:8]:
        print(f"  BI{item_id}: {p.name} {fam_desc(k)}")

    print(f"nearest_lower=BI{lower[0]} {fam_desc(lower[3])}")
    print(f"nearest_upper=BI{upper[0]} {fam_desc(upper[3])}")

    if lower[3] != upper[3]:
        raise SystemExit("ERRO: vizinhos imediato inferior/superior divergem")

    candidate = lower[3]
    if candidate[0] not in {"BMP_STANDARD_DIB40", "BMP_PROTECTED_HEADER14"}:
        raise SystemExit(f"ERRO: família dos vizinhos não suportada: {fam_desc(candidate)}")

    nearest6 = ordered[:6]
    support = sum(1 for x in nearest6 if x[3] == candidate)
    print(f"nearest6_support={support}/6")
    if support < 4:
        raise SystemExit("ERRO: suporte local insuficiente")

    print("family_selection=IMMEDIATE_BI_NEIGHBORS")
    return candidate

def main():
    ap = argparse.ArgumentParser(description="BI330 restore with protected 24/32bpp itBI family support.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    premium_dir = client / "image/Sinimage/Items/Premium"
    target = client / TARGET_REL
    evid = repo / "bugfixes/058_asset_reconciliation/evidence/bi330_experimental_v1"
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
        if sha256(target) != r["target_sha256"]:
            raise SystemExit("ERRO: rollback recusado; target mudou")
        target.unlink()
        print(f"OK: rollback removeu somente {target}")
        return

    if not SOURCE.is_file():
        raise SystemExit(f"ERRO: source ausente: {SOURCE}")
    if target.exists():
        raise SystemExit(f"ERRO: target já existe: {target}")

    src_sha = sha256(SOURCE)
    if src_sha != EXPECTED_SOURCE_SHA256:
        raise SystemExit(f"ERRO: source SHA mudou: {src_sha}")

    sw, sh, sch, sfmt = identify(SOURCE)
    print("=== BI330 itBI FAMILY EXPERIMENT 058 V3 ===")
    print(f"source={SOURCE}")
    print(f"source_sha256={src_sha}")
    print(f"source_identify={sw}x{sh} {sch} {sfmt}")
    print(f"target={target}")

    fam = choose_bi_family(premium_dir, sw, sh)
    print(f"selected_family={fam_desc(fam)}")

    if fam[0] == "BMP_STANDARD_DIB40":
        target_bpp = fam[1]
    else:
        target_bpp = fam[6]

    with tempfile.TemporaryDirectory(prefix="summit_bi330_v3_") as td:
        td = Path(td)
        standard_path = td / "itBI330.standard.bmp"
        final_path = td / "itBI330.final.bmp"
        norm_path = td / "itBI330.normalized.bmp"

        if target_bpp == 32:
            raw = image_to_rgba_raw(SOURCE, sw, sh)
            std_bytes = build_standard_bmp32_from_rgba(raw, sw, sh)
        elif target_bpp == 24:
            std_bytes = build_standard_bmp24(SOURCE, standard_path)
        else:
            raise SystemExit(f"ERRO: bpp inesperado: {target_bpp}")

        validate_standard_bytes(std_bytes, sw, sh, target_bpp)
        standard_path.write_bytes(std_bytes)

        if fam[0] == "BMP_PROTECTED_HEADER14":
            final_bytes = protect(std_bytes, fam)
            if final_bytes[14:] != std_bytes[14:]:
                raise SystemExit("ERRO: proteção alterou DIB/pixels")
            norm_bytes = normalize(final_bytes)
            if norm_bytes[14:] != std_bytes[14:]:
                raise SystemExit("ERRO: normalização alterou DIB/pixels")
        else:
            final_bytes = std_bytes
            norm_bytes = std_bytes

        final_path.write_bytes(final_bytes)
        norm_path.write_bytes(norm_bytes)

        validate_standard_bytes(norm_bytes, sw, sh, target_bpp)
        nw, nh, nch, nfmt = identify(norm_path)
        if (nw, nh) != (sw, sh):
            raise SystemExit("ERRO: normalized dimension mismatch")

        final_hash = sha256(final_path)
        norm_hash = sha256(norm_path)

        (evid / "itBI330.standard.bmp").write_bytes(std_bytes)
        (evid / "itBI330.final.bmp").write_bytes(final_bytes)
        (evid / "itBI330.normalized-for-validation.bmp").write_bytes(norm_bytes)

        print(f"target_bpp={target_bpp}")
        print(f"standard_size={len(std_bytes)}")
        print(f"final_size={len(final_bytes)}")
        print(f"final_sha256={final_hash}")
        print(f"normalized_sha256={norm_hash}")
        print(f"normalized_identify={nw}x{nh} {nch} {nfmt}")
        print("STRUCTURAL_VALIDATION=PASS")
        print("DIB_AND_PIXELS_UNCHANGED_BY_PROTECTION=YES")

        if args.action == "audit":
            print("APPLY=NOT_RUN")
            return

        shutil.copy2(final_path, target)
        target_hash = sha256(target)
        if target_hash != final_hash:
            target.unlink(missing_ok=True)
            raise SystemExit("ERRO: SHA runtime divergiu")

        fields = [
            "target","source","source_sha256","target_sha256",
            "family","bpp","width","height"
        ]
        with manifest.open("w", encoding="utf-8", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
            w.writeheader()
            w.writerow({
                "target": str(target),
                "source": str(SOURCE),
                "source_sha256": src_sha,
                "target_sha256": target_hash,
                "family": fam[0],
                "bpp": target_bpp,
                "width": sw,
                "height": sh,
            })

        print(f"OK: criado SOMENTE {target}")
        print(f"runtime_sha256={target_hash}")
        print("RUNTIME_TEST=READY")

if __name__ == "__main__":
    main()

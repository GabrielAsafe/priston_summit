#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import os
import shutil
import struct
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

TARGET_REL = Path("image/Sinimage/Items/Defense/itDA247.bmp")
SOURCE_REL = Path("client/image/Sinimage/Items/Defense/itda247.dds")
EXPECTED_W = 66
EXPECTED_H = 88

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def u16(b: bytes | bytearray, off: int) -> int:
    return struct.unpack_from("<H", b, off)[0]

def u32(b: bytes | bytearray, off: int) -> int:
    return struct.unpack_from("<I", b, off)[0]

def i32(b: bytes | bytearray, off: int) -> int:
    return struct.unpack_from("<i", b, off)[0]

def identify(p: Path):
    magick = shutil.which("magick")
    ident = shutil.which("identify")
    if magick:
        cmd = [magick, "identify", "-format", "%w\t%h\t%[channels]\t%m", str(p)]
    elif ident:
        cmd = [ident, "-format", "%w\t%h\t%[channels]\t%m", str(p)]
    else:
        raise SystemExit("ERRO: ImageMagick (magick/identify) não encontrado.")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if r.returncode:
        raise RuntimeError(f"identify falhou para {p}: {r.stderr.strip()}")
    parts = r.stdout.strip().split("\t")
    if len(parts) < 4:
        raise RuntimeError(f"identify retornou saída inesperada para {p}: {r.stdout!r}")
    return int(parts[0]), int(parts[1]), parts[2], parts[3]

def convert_dds_to_bmp3(src: Path, dst: Path):
    magick = shutil.which("magick")
    convert = shutil.which("convert")
    if magick:
        cmd = [
            magick, str(src),
            "-alpha", "off",
            "-depth", "8",
            "-type", "TrueColor",
            f"BMP3:{dst}",
        ]
    elif convert:
        cmd = [
            convert, str(src),
            "-alpha", "off",
            "-depth", "8",
            "-type", "TrueColor",
            f"BMP3:{dst}",
        ]
    else:
        raise SystemExit("ERRO: ImageMagick não encontrado.")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if r.returncode:
        raise RuntimeError(f"conversão DDS->BMP3 falhou: {r.stderr.strip()}")

def parse_family_header(p: Path):
    b = p.read_bytes()
    if len(b) < 54:
        raise RuntimeError(f"BMP curto demais: {p}")
    return {
        "path": p,
        "actual_size": len(b),
        "sig": bytes(b[0:2]),
        "bf_size_field": u32(b, 2),
        "size_delta": u32(b, 2) - len(b),
        "reserved1": u16(b, 6),
        "reserved2": u16(b, 8),
        "offbits": u32(b, 10),
        "dib": u32(b, 14),
        "width": i32(b, 18),
        "height": i32(b, 22),
        "planes": u16(b, 26),
        "bpp": u16(b, 28),
        "compression": u32(b, 30),
        "image_size": u32(b, 34),
        "xppm": i32(b, 38),
        "yppm": i32(b, 42),
        "clr_used": u32(b, 46),
        "clr_important": u32(b, 50),
    }

def infer_family(defense_dir: Path):
    siblings = sorted(
        p for p in defense_dir.glob("*.bmp")
        if p.name.casefold().startswith("itda")
        and p.name.casefold() != TARGET_REL.name.casefold()
    )
    if len(siblings) < 20:
        raise RuntimeError(f"poucos irmãos itDA*.bmp: {len(siblings)}")

    rows = [parse_family_header(p) for p in siblings]

    protected = [
        r for r in rows
        if r["dib"] == 40
        and r["planes"] == 1
        and r["bpp"] == 24
        and r["compression"] == 0
        and r["sig"] != b"BM"
    ]
    if len(protected) < 20:
        raise RuntimeError(
            f"família BMP_PROTECTED_HEADER14 insuficiente entre itDA*: {len(protected)}"
        )

    fields = {
        "sig": Counter(r["sig"] for r in protected),
        "size_delta": Counter(r["size_delta"] for r in protected),
        "reserved1": Counter(r["reserved1"] for r in protected),
        "reserved2": Counter(r["reserved2"] for r in protected),
        "offbits": Counter(r["offbits"] for r in protected),
    }

    for name, c in fields.items():
        if len(c) != 1:
            raise RuntimeError(f"campo protegido não uniforme em itDA*: {name}={dict(c)}")

    same_dim = [
        r for r in protected
        if abs(r["width"]) == EXPECTED_W and abs(r["height"]) == EXPECTED_H
    ]
    if not same_dim:
        raise RuntimeError("nenhum irmão itDA protegido 66x88 encontrado")

    return {
        "samples": protected,
        "same_dim": same_dim,
        "sig": next(iter(fields["sig"])),
        "size_delta": next(iter(fields["size_delta"])),
        "reserved1": next(iter(fields["reserved1"])),
        "reserved2": next(iter(fields["reserved2"])),
        "offbits": next(iter(fields["offbits"])),
    }

def validate_standard_bmp3(p: Path):
    b = p.read_bytes()
    if len(b) < 54:
        raise RuntimeError("BMP3 gerado curto demais")
    vals = {
        "sig": bytes(b[:2]),
        "actual": len(b),
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
    if vals["sig"] != b"BM":
        raise RuntimeError(f"BMP3 sem assinatura BM: {vals['sig']!r}")
    if vals["bfSize"] != vals["actual"]:
        raise RuntimeError(f"bfSize padrão diverge: {vals['bfSize']} != {vals['actual']}")
    if vals["dib"] != 40:
        raise RuntimeError(f"DIB esperado 40, obtido {vals['dib']}")
    if abs(vals["width"]) != EXPECTED_W or abs(vals["height"]) != EXPECTED_H:
        raise RuntimeError(
            f"dimensão BMP errada: {vals['width']}x{vals['height']} "
            f"(esperado {EXPECTED_W}x{EXPECTED_H})"
        )
    if vals["planes"] != 1 or vals["bpp"] != 24 or vals["compression"] != 0:
        raise RuntimeError(
            f"perfil BMP incompatível: planes={vals['planes']} "
            f"bpp={vals['bpp']} compression={vals['compression']}"
        )
    if vals["offbits"] != 54:
        raise RuntimeError(f"pixel offset BMP3 esperado 54, obtido {vals['offbits']}")
    return vals

def protect_header14(standard: bytes, family: dict) -> bytes:
    b = bytearray(standard)
    actual = len(b)

    # Reproduz exatamente BMP_PROTECTED_HEADER14 inferido dos itDA* reais.
    b[0:2] = family["sig"]
    struct.pack_into("<I", b, 2, actual + family["size_delta"])
    struct.pack_into("<H", b, 6, family["reserved1"])
    struct.pack_into("<H", b, 8, family["reserved2"])
    struct.pack_into("<I", b, 10, family["offbits"])
    return bytes(b)

def normalize_header14(protected: bytes) -> bytes:
    b = bytearray(protected)
    actual = len(b)
    dib = u32(b, 14)
    b[0:2] = b"BM"
    struct.pack_into("<I", b, 2, actual)
    struct.pack_into("<H", b, 6, 0)
    struct.pack_into("<H", b, 8, 0)
    # 24bpp BI_RGB, DIB40, sem paleta.
    struct.pack_into("<I", b, 10, 14 + dib)
    return bytes(b)

def main():
    ap = argparse.ArgumentParser(
        description="Generate/test only itDA247.bmp using Summit BMP_PROTECTED_HEADER14 family."
    )
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    source = Path("/home/gabriel/armazem/priston/ExMachina") / SOURCE_REL
    target = client / TARGET_REL
    defense_dir = target.parent

    evid = repo / "bugfixes" / "058_asset_reconciliation" / "evidence" / "itda247_experimental"
    evid.mkdir(parents=True, exist_ok=True)
    manifest = evid / "manifest.tsv"
    generated_copy = evid / "itDA247.protected.bmp"
    normalized_copy = evid / "itDA247.normalized-for-validation.bmp"

    if args.action == "rollback":
        if not manifest.is_file():
            raise SystemExit("ERRO: manifest não encontrado.")
        with manifest.open("r", encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f, delimiter="\t"))
        if len(rows) != 1:
            raise SystemExit("ERRO: manifest inesperado.")
        r = rows[0]
        if r["created"] != "1":
            print("Nada para remover.")
            return
        if not target.exists():
            print("OK: target já está ausente.")
            return
        cur = sha256(target)
        if cur != r["protected_sha256"]:
            raise SystemExit(
                "ERRO: itDA247.bmp mudou após o experimento; rollback recusado.\n"
                f"atual={cur}\nesperado={r['protected_sha256']}"
            )
        target.unlink()
        print(f"OK: rollback removeu somente {target}")
        return

    if not client.is_dir():
        raise SystemExit(f"ERRO: cliente não encontrado: {client}")
    if not source.is_file():
        raise SystemExit(f"ERRO: DDS fonte não encontrado: {source}")
    if target.exists():
        raise SystemExit(f"ERRO: destino já existe; não vou sobrescrever: {target}")

    sw, sh, sch, sfmt = identify(source)
    if (sw, sh) != (EXPECTED_W, EXPECTED_H):
        raise SystemExit(
            f"ERRO: source DDS é {sw}x{sh}, esperado {EXPECTED_W}x{EXPECTED_H}"
        )

    family = infer_family(defense_dir)

    print("=== itDA247 BMP_PROTECTED_HEADER14 EXPERIMENT ===")
    print(f"source={source}")
    print(f"source_sha256={sha256(source)}")
    print(f"source_identify={sw}x{sh} {sch} {sfmt}")
    print(f"target={target}")
    print(f"family_samples={len(family['samples'])}")
    print(f"same_dimension_samples={len(family['same_dim'])}")
    print(f"family_signature={family['sig'].hex()} ({family['sig'].decode('latin1', errors='replace')})")
    print(f"family_bfSize_delta=0x{family['size_delta']:08x}")
    print(f"family_reserved1=0x{family['reserved1']:04x}")
    print(f"family_reserved2=0x{family['reserved2']:04x}")
    print(f"family_offbits=0x{family['offbits']:08x}")

    with tempfile.TemporaryDirectory(prefix="summit_itda247_") as td:
        td = Path(td)
        standard = td / "itDA247.standard.bmp"
        protected_tmp = td / "itDA247.protected.bmp"
        normalized_tmp = td / "itDA247.normalized.bmp"

        convert_dds_to_bmp3(source, standard)
        std = validate_standard_bmp3(standard)
        std_bytes = standard.read_bytes()

        protected_bytes = protect_header14(std_bytes, family)
        protected_tmp.write_bytes(protected_bytes)

        # Prova de que só HEADER14 mudou.
        if protected_bytes[14:] != std_bytes[14:]:
            raise SystemExit("ERRO: proteção alterou bytes fora do BITMAPFILEHEADER de 14 bytes")

        # Validação reversa: normalizar o experimental deve produzir BMP legível,
        # mesmas dimensões e DIB/pixels idênticos ao BMP3 de origem.
        normalized_bytes = normalize_header14(protected_bytes)
        normalized_tmp.write_bytes(normalized_bytes)

        if normalized_bytes[14:] != std_bytes[14:]:
            raise SystemExit("ERRO: normalização alterou DIB/pixels")

        nw, nh, nch, nfmt = identify(normalized_tmp)
        if (nw, nh) != (EXPECTED_W, EXPECTED_H):
            raise SystemExit(f"ERRO: normalizado identifica {nw}x{nh}")

        # Campos protegidos do resultado.
        ph = parse_family_header(protected_tmp)
        expected_bfsize = len(protected_bytes) + family["size_delta"]

        checks = {
            "sig": ph["sig"] == family["sig"],
            "bfSize": ph["bf_size_field"] == expected_bfsize,
            "reserved1": ph["reserved1"] == family["reserved1"],
            "reserved2": ph["reserved2"] == family["reserved2"],
            "offbits": ph["offbits"] == family["offbits"],
            "dib": ph["dib"] == 40,
            "width": abs(ph["width"]) == EXPECTED_W,
            "height": abs(ph["height"]) == EXPECTED_H,
            "planes": ph["planes"] == 1,
            "bpp": ph["bpp"] == 24,
            "compression": ph["compression"] == 0,
        }
        bad = [k for k, ok in checks.items() if not ok]
        if bad:
            raise SystemExit(f"ERRO: validação estrutural falhou: {bad}")

        # Evidence copies.
        generated_copy.write_bytes(protected_bytes)
        normalized_copy.write_bytes(normalized_bytes)

        protected_hash = sha256(protected_tmp)
        normalized_hash = sha256(normalized_tmp)

        print(f"standard_size={std['actual']}")
        print(f"protected_bfSize=0x{ph['bf_size_field']:08x}")
        print(f"protected_sha256={protected_hash}")
        print(f"normalized_sha256={normalized_hash}")
        print(f"normalized_identify={nw}x{nh} {nch} {nfmt}")
        print("STRUCTURAL_VALIDATION=PASS")
        print("DIB_AND_PIXELS_UNCHANGED_BY_PROTECTION=YES")

        if args.action == "audit":
            print("APPLY=NOT_RUN")
            return

        # Final no-overwrite check immediately before creation.
        if target.exists():
            raise SystemExit(f"ERRO: destino apareceu durante apply: {target}")

        shutil.copy2(protected_tmp, target)
        if sha256(target) != protected_hash:
            target.unlink(missing_ok=True)
            raise SystemExit("ERRO: SHA-256 do runtime divergiu após cópia")

        with manifest.open("w", encoding="utf-8", newline="") as f:
            fields = [
                "target","source","source_sha256","protected_sha256",
                "normalized_sha256","created","family_signature",
                "family_bfSize_delta","family_reserved1","family_reserved2",
                "family_offbits","width","height","bpp","compression"
            ]
            w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
            w.writeheader()
            w.writerow({
                "target": str(target),
                "source": str(source),
                "source_sha256": sha256(source),
                "protected_sha256": protected_hash,
                "normalized_sha256": normalized_hash,
                "created": "1",
                "family_signature": family["sig"].hex(),
                "family_bfSize_delta": f"0x{family['size_delta']:08x}",
                "family_reserved1": f"0x{family['reserved1']:04x}",
                "family_reserved2": f"0x{family['reserved2']:04x}",
                "family_offbits": f"0x{family['offbits']:08x}",
                "width": EXPECTED_W,
                "height": EXPECTED_H,
                "bpp": 24,
                "compression": 0,
            })

        print(f"OK: criado SOMENTE {target}")
        print("RUNTIME_TEST=READY")

if __name__ == "__main__":
    main()

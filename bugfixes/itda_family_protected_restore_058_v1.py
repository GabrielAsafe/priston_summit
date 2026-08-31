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

DEFAULT_IDS = ("147", "153", "253")
EXPECTED_W = 66
EXPECTED_H = 88

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
        raise SystemExit("ERRO: ImageMagick não encontrado.")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if r.returncode:
        raise RuntimeError(f"identify falhou para {p}: {r.stderr.strip()}")
    parts = r.stdout.strip().split("\t")
    if len(parts) < 4:
        raise RuntimeError(f"identify inesperado para {p}: {r.stdout!r}")
    return int(parts[0]), int(parts[1]), parts[2], parts[3]

def convert_dds_to_bmp3(src: Path, dst: Path):
    magick = shutil.which("magick")
    convert = shutil.which("convert")
    if magick:
        cmd = [magick, str(src), "-alpha", "off", "-depth", "8",
               "-type", "TrueColor", f"BMP3:{dst}"]
    elif convert:
        cmd = [convert, str(src), "-alpha", "off", "-depth", "8",
               "-type", "TrueColor", f"BMP3:{dst}"]
    else:
        raise SystemExit("ERRO: ImageMagick não encontrado.")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if r.returncode:
        raise RuntimeError(f"conversão falhou: {src}: {r.stderr.strip()}")

def parse_header(p: Path):
    b = p.read_bytes()
    if len(b) < 54:
        raise RuntimeError(f"arquivo curto demais: {p}")
    return {
        "actual_size": len(b),
        "sig": bytes(b[:2]),
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
    }

def infer_family(defense_dir: Path):
    siblings = sorted(
        p for p in defense_dir.glob("*.bmp")
        if p.name.casefold().startswith("itda")
    )
    rows = [parse_header(p) for p in siblings]
    protected = [
        r for r in rows
        if r["dib"] == 40 and r["planes"] == 1 and r["bpp"] == 24
        and r["compression"] == 0 and r["sig"] != b"BM"
    ]
    if len(protected) < 20:
        raise RuntimeError(f"família protegida insuficiente: {len(protected)}")

    fields = {
        "sig": Counter(r["sig"] for r in protected),
        "size_delta": Counter(r["size_delta"] for r in protected),
        "reserved1": Counter(r["reserved1"] for r in protected),
        "reserved2": Counter(r["reserved2"] for r in protected),
        "offbits": Counter(r["offbits"] for r in protected),
    }
    for name, c in fields.items():
        if len(c) != 1:
            raise RuntimeError(f"campo não uniforme: {name}={dict(c)}")

    same_dim = [
        r for r in protected
        if abs(r["width"]) == EXPECTED_W and abs(r["height"]) == EXPECTED_H
    ]
    if not same_dim:
        raise RuntimeError("nenhum irmão itDA protegido 66x88")

    return {
        "samples": len(protected),
        "same_dim": len(same_dim),
        "sig": next(iter(fields["sig"])),
        "size_delta": next(iter(fields["size_delta"])),
        "reserved1": next(iter(fields["reserved1"])),
        "reserved2": next(iter(fields["reserved2"])),
        "offbits": next(iter(fields["offbits"])),
    }

def validate_standard(p: Path):
    b = p.read_bytes()
    vals = {
        "sig": bytes(b[:2]), "actual": len(b), "bfSize": u32(b, 2),
        "reserved1": u16(b, 6), "reserved2": u16(b, 8),
        "offbits": u32(b, 10), "dib": u32(b, 14),
        "width": i32(b, 18), "height": i32(b, 22),
        "planes": u16(b, 26), "bpp": u16(b, 28),
        "compression": u32(b, 30),
    }
    if vals["sig"] != b"BM": raise RuntimeError("BMP3 sem BM")
    if vals["bfSize"] != vals["actual"]: raise RuntimeError("bfSize inválido")
    if vals["dib"] != 40: raise RuntimeError(f"DIB={vals['dib']}")
    if abs(vals["width"]) != EXPECTED_W or abs(vals["height"]) != EXPECTED_H:
        raise RuntimeError(f"dimensão={vals['width']}x{vals['height']}")
    if vals["planes"] != 1 or vals["bpp"] != 24 or vals["compression"] != 0:
        raise RuntimeError("perfil BMP3 incompatível")
    if vals["offbits"] != 54: raise RuntimeError(f"offbits padrão={vals['offbits']}")
    return vals

def protect(std: bytes, family: dict) -> bytes:
    b = bytearray(std)
    actual = len(b)
    b[0:2] = family["sig"]
    struct.pack_into("<I", b, 2, actual + family["size_delta"])
    struct.pack_into("<H", b, 6, family["reserved1"])
    struct.pack_into("<H", b, 8, family["reserved2"])
    struct.pack_into("<I", b, 10, family["offbits"])
    return bytes(b)

def normalize(prot: bytes) -> bytes:
    b = bytearray(prot)
    actual = len(b)
    dib = u32(b, 14)
    b[0:2] = b"BM"
    struct.pack_into("<I", b, 2, actual)
    struct.pack_into("<H", b, 6, 0)
    struct.pack_into("<H", b, 8, 0)
    struct.pack_into("<I", b, 10, 14 + dib)
    return bytes(b)

def main():
    ap = argparse.ArgumentParser(
        description="Restore approved itDA BMP_PROTECTED_HEADER14 assets."
    )
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    ap.add_argument("--ids", default=",".join(DEFAULT_IDS),
                    help="DA numeric IDs, comma-separated. Default: 147,153,253")
    args = ap.parse_args()

    ids = tuple(x.strip() for x in args.ids.split(",") if x.strip())
    if not ids or any(not x.isdigit() for x in ids):
        raise SystemExit("ERRO: --ids inválido")

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    defense = client / "image" / "Sinimage" / "Items" / "Defense"
    ex = Path("/home/gabriel/armazem/priston/ExMachina/client/image/Sinimage/Items/Defense")
    evid = repo / "bugfixes/058_asset_reconciliation/evidence/itda_family_restore"
    evid.mkdir(parents=True, exist_ok=True)
    manifest = evid / "manifest.tsv"

    targets = [(x, ex / f"itda{x}.dds", defense / f"itDA{x}.bmp") for x in ids]

    if args.action == "rollback":
        if not manifest.is_file():
            raise SystemExit("ERRO: manifest não encontrado")
        removed = 0
        with manifest.open("r", encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f, delimiter="\t"))
        wanted = set(ids)
        for r in rows:
            if r["id"] not in wanted or r["created"] != "1":
                continue
            p = Path(r["target"])
            if not p.exists():
                continue
            if sha256(p) != r["protected_sha256"]:
                raise SystemExit(f"ERRO: rollback recusado; arquivo mudou: {p}")
            p.unlink()
            removed += 1
        print(f"OK: rollback removeu {removed} arquivos")
        return

    family = infer_family(defense)

    print("=== itDA FAMILY BMP_PROTECTED_HEADER14 ===")
    print(f"ids={','.join(ids)}")
    print(f"family_samples={family['samples']}")
    print(f"same_dimension_samples={family['same_dim']}")
    print(f"family_signature={family['sig'].hex()} ({family['sig'].decode('latin1', errors='replace')})")
    print(f"family_bfSize_delta=0x{family['size_delta']:08x}")
    print(f"family_reserved1=0x{family['reserved1']:04x}")
    print(f"family_reserved2=0x{family['reserved2']:04x}")
    print(f"family_offbits=0x{family['offbits']:08x}")

    blockers = []
    prepared = []

    with tempfile.TemporaryDirectory(prefix="summit_itda_family_") as td:
        td = Path(td)

        for item_id, src, dst in targets:
            if not src.is_file():
                blockers.append(f"DA{item_id}: source ausente: {src}")
                continue
            if dst.exists():
                blockers.append(f"DA{item_id}: destino já existe: {dst}")
                continue

            sw, sh, sch, sfmt = identify(src)
            if (sw, sh) != (EXPECTED_W, EXPECTED_H):
                blockers.append(f"DA{item_id}: source {sw}x{sh}, esperado 66x88")
                continue

            standard = td / f"itDA{item_id}.standard.bmp"
            protected_tmp = td / f"itDA{item_id}.protected.bmp"
            normalized_tmp = td / f"itDA{item_id}.normalized.bmp"

            try:
                convert_dds_to_bmp3(src, standard)
                std = validate_standard(standard)
                std_bytes = standard.read_bytes()
                prot = protect(std_bytes, family)

                if prot[14:] != std_bytes[14:]:
                    raise RuntimeError("proteção alterou DIB/pixels")

                protected_tmp.write_bytes(prot)
                norm = normalize(prot)
                normalized_tmp.write_bytes(norm)

                if norm[14:] != std_bytes[14:]:
                    raise RuntimeError("normalização alterou DIB/pixels")

                nw, nh, nch, nfmt = identify(normalized_tmp)
                if (nw, nh) != (EXPECTED_W, EXPECTED_H):
                    raise RuntimeError(f"normalizado {nw}x{nh}")

                ph = parse_header(protected_tmp)
                checks = [
                    ph["sig"] == family["sig"],
                    ph["bf_size_field"] == len(prot) + family["size_delta"],
                    ph["reserved1"] == family["reserved1"],
                    ph["reserved2"] == family["reserved2"],
                    ph["offbits"] == family["offbits"],
                    ph["dib"] == 40,
                    abs(ph["width"]) == 66,
                    abs(ph["height"]) == 88,
                    ph["planes"] == 1,
                    ph["bpp"] == 24,
                    ph["compression"] == 0,
                ]
                if not all(checks):
                    raise RuntimeError("validação estrutural protegida falhou")

                phash = sha256(protected_tmp)
                nhash = sha256(normalized_tmp)

                evid_file = evid / f"itDA{item_id}.protected.bmp"
                evid_file.write_bytes(prot)

                prepared.append({
                    "id": item_id,
                    "src": src,
                    "dst": dst,
                    "protected_tmp": protected_tmp,
                    "source_sha256": sha256(src),
                    "protected_sha256": phash,
                    "normalized_sha256": nhash,
                    "source_identify": f"{sw}x{sh} {sch} {sfmt}",
                    "normalized_identify": f"{nw}x{nh} {nch} {nfmt}",
                })

                print(
                    f"DA{item_id}: PASS "
                    f"source={sw}x{sh} protected_sha256={phash}"
                )
            except Exception as e:
                blockers.append(f"DA{item_id}: {e}")

        print(f"prepared={len(prepared)}")
        print(f"blockers={len(blockers)}")
        for b in blockers:
            print("BLOCKER:", b)

        if args.action == "audit":
            if blockers:
                raise SystemExit("AUDIT=BLOCKED")
            print("STRUCTURAL_VALIDATION=PASS")
            print("DIB_AND_PIXELS_UNCHANGED_BY_PROTECTION=YES")
            print("APPLY=NOT_RUN")
            return

        if blockers:
            raise SystemExit("ERRO: blockers; nada aplicado")

        created = []
        rows = []
        try:
            for r in prepared:
                if r["dst"].exists():
                    raise RuntimeError(f"destino apareceu: {r['dst']}")
                shutil.copy2(r["protected_tmp"], r["dst"])
                if sha256(r["dst"]) != r["protected_sha256"]:
                    raise RuntimeError(f"SHA pós-cópia divergiu: {r['dst']}")
                created.append(r["dst"])
                rows.append({
                    "id": r["id"],
                    "target": str(r["dst"]),
                    "source": str(r["src"]),
                    "source_sha256": r["source_sha256"],
                    "protected_sha256": r["protected_sha256"],
                    "normalized_sha256": r["normalized_sha256"],
                    "created": "1",
                    "width": 66,
                    "height": 88,
                    "bpp": 24,
                    "compression": 0,
                })
        except Exception:
            for p in created:
                try: p.unlink()
                except OSError: pass
            raise

        fields = [
            "id","target","source","source_sha256","protected_sha256",
            "normalized_sha256","created","width","height","bpp","compression"
        ]
        with manifest.open("w", encoding="utf-8", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
            w.writeheader(); w.writerows(rows)

        print(f"OK: criados {len(rows)} assets: " + ", ".join(f"itDA{x}.bmp" for x in ids))
        print("RUNTIME_TEST=READY")

if __name__ == "__main__":
    main()

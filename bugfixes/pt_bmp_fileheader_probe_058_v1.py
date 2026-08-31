#!/usr/bin/env python3
from __future__ import annotations

import argparse
import collections
import os
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path

def u16(b, off): return struct.unpack_from("<H", b, off)[0]
def u32(b, off): return struct.unpack_from("<I", b, off)[0]
def i32(b, off): return struct.unpack_from("<i", b, off)[0]

def identify(path: Path):
    magick = shutil.which("magick")
    ident = shutil.which("identify")
    if magick:
        cmd = [magick, "identify", "-format", "%w\t%h\t%m", str(path)]
    elif ident:
        cmd = [ident, "-format", "%w\t%h\t%m", str(path)]
    else:
        raise SystemExit("ERRO: ImageMagick não encontrado.")
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode:
        return False, p.stderr.strip().replace("\n", " | ")
    return True, p.stdout.strip()

def main():
    ap = argparse.ArgumentParser(description="Prove whether protected PT BMPs alter only the 14-byte BITMAPFILEHEADER.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("--limit", type=int, default=0, help="0 = all itDA*.bmp")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    d = repo / "Summit Tale Client Full" / "cliente" / "image" / "Sinimage" / "Items" / "Defense"
    files = sorted(p for p in d.glob("*.bmp") if p.name.casefold().startswith("itda"))
    if args.limit:
        files = files[:args.limit]
    if not files:
        raise SystemExit(f"ERRO: nenhum itDA*.bmp em {d}")

    sigs = collections.Counter()
    r1s = collections.Counter()
    r2s = collections.Counter()
    offs = collections.Counter()
    dibs = collections.Counter()
    bpps = collections.Counter()
    comps = collections.Counter()
    xor_sizes = collections.Counter()
    delta_sizes = collections.Counter()

    probe_ok = 0
    probe_fail = []
    dim_mismatch = []

    for p in files:
        raw = bytearray(p.read_bytes())
        if len(raw) < 54:
            probe_fail.append((p.name, "arquivo curto"))
            continue

        actual = len(raw)
        sig = bytes(raw[:2])
        bfsize = u32(raw, 2)
        r1 = u16(raw, 6)
        r2 = u16(raw, 8)
        off = u32(raw, 10)
        dib = u32(raw, 14)
        w = i32(raw, 18)
        h = i32(raw, 22)
        planes = u16(raw, 26)
        bpp = u16(raw, 28)
        comp = u32(raw, 30)

        sigs[sig.hex()] += 1
        r1s[f"0x{r1:04x}"] += 1
        r2s[f"0x{r2:04x}"] += 1
        offs[f"0x{off:08x}"] += 1
        dibs[str(dib)] += 1
        bpps[str(bpp)] += 1
        comps[str(comp)] += 1
        xor_sizes[f"0x{(bfsize ^ actual):08x}"] += 1
        delta_sizes[str(bfsize - actual)] += 1

        # Normalize ONLY BITMAPFILEHEADER (bytes 0..13).
        # Keep DIB header and all pixel bytes untouched.
        test = bytearray(raw)
        test[0:2] = b"BM"
        struct.pack_into("<I", test, 2, actual)
        struct.pack_into("<H", test, 6, 0)
        struct.pack_into("<H", test, 8, 0)

        # For BITMAPINFOHEADER + 24bpp BI_RGB with no palette, canonical pixel offset is 54.
        canonical_off = 14 + dib
        if bpp <= 8:
            clr_used = u32(test, 46) if dib >= 40 else 0
            colors = clr_used if clr_used else (1 << bpp)
            canonical_off += colors * 4
        struct.pack_into("<I", test, 10, canonical_off)

        fd, tmpname = tempfile.mkstemp(prefix="summit_itda_header_", suffix=".bmp")
        os.close(fd)
        tmp = Path(tmpname)
        try:
            tmp.write_bytes(test)
            ok, info = identify(tmp)
            if ok:
                probe_ok += 1
                try:
                    iw, ih, _ = info.split("\t", 2)
                    if (int(iw), int(ih)) != (abs(w), abs(h)):
                        dim_mismatch.append((p.name, f"DIB={w}x{h}", f"identify={iw}x{ih}"))
                except Exception:
                    pass
            else:
                probe_fail.append((p.name, info))
        finally:
            try: tmp.unlink()
            except OSError: pass

    def fmt(counter):
        return "; ".join(f"{k}:{v}" for k, v in counter.most_common())

    print("=== PT BMP FILEHEADER PROBE 058 ===")
    print(f"SAMPLES={len(files)}")
    print(f"SIGNATURES={fmt(sigs)}")
    print(f"RESERVED1={fmt(r1s)}")
    print(f"RESERVED2={fmt(r2s)}")
    print(f"OFFBITS={fmt(offs)}")
    print(f"DIB={fmt(dibs)}")
    print(f"BPP={fmt(bpps)}")
    print(f"COMPRESSION={fmt(comps)}")
    print(f"BFSIZE_XOR_ACTUAL={fmt(xor_sizes)}")
    print(f"BFSIZE_MINUS_ACTUAL={fmt(delta_sizes)}")
    print(f"NORMALIZED_FILEHEADER_IDENTIFY_OK={probe_ok}/{len(files)}")
    print(f"DIMENSION_MISMATCH={len(dim_mismatch)}")
    print(f"FAILURES={len(probe_fail)}")

    if probe_ok == len(files) and not dim_mismatch:
        print("VERDICT=FILEHEADER_ONLY_PROTECTION_PROVED")
    elif probe_ok:
        print("VERDICT=PARTIAL_FILEHEADER_RECOVERY")
    else:
        print("VERDICT=PROTECTION_EXTENDS_BEYOND_FILEHEADER")

    if probe_fail:
        print("\nFIRST_FAILURES:")
        for name, err in probe_fail[:10]:
            print(f"{name}: {err}")

if __name__ == "__main__":
    main()

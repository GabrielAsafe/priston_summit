#!/usr/bin/env python3
"""
PT BMP Visible Test Patch V1

Creates a visibly tinted test copy of a STANDARD 24-bit BI_RGB BMP.

Safety:
- input is never overwritten;
- header/non-pixel bytes are copied byte-for-byte;
- only the red byte of pixel triplets is changed;
- dimensions/bpp/compression are validated;
- output is intended only as a runtime pipeline gate.
"""

from __future__ import annotations
import argparse
import hashlib
import pathlib
import struct


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def u16(b, o): return struct.unpack_from("<H", b, o)[0]
def u32(b, o): return struct.unpack_from("<I", b, o)[0]
def i32(b, o): return struct.unpack_from("<i", b, o)[0]


def inspect(data):
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError("input must be a standard BMP beginning with BM")
    declared = u32(data, 2)
    pixel_offset = u32(data, 10)
    dib = u32(data, 14)
    width = i32(data, 18)
    height = i32(data, 22)
    planes = u16(data, 26)
    bpp = u16(data, 28)
    compression = u32(data, 30)

    if declared not in (0, len(data)):
        raise ValueError(f"declared BMP size {declared} != actual {len(data)}")
    if dib != 40:
        raise ValueError(f"unsupported DIB size {dib}; expected 40")
    if width <= 0 or height == 0:
        raise ValueError(f"unsupported dimensions {width}x{height}")
    if planes != 1 or bpp != 24 or compression != 0:
        raise ValueError(
            f"expected planes=1,bpp=24,BI_RGB; got {planes},{bpp},{compression}"
        )

    stride = ((width * 24 + 31) // 32) * 4
    pixel_bytes = stride * abs(height)
    pixel_end = pixel_offset + pixel_bytes
    if pixel_end > len(data):
        raise ValueError("pixel payload exceeds file")

    return {
        "width": width,
        "height": height,
        "pixel_offset": pixel_offset,
        "pixel_end": pixel_end,
        "pixel_bytes": pixel_bytes,
        "stride": stride,
    }


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--in", dest="src", required=True)
    p.add_argument("--out", required=True)
    p.add_argument(
        "--amount",
        type=int,
        default=72,
        help="red-channel increase, 1..255 (default 72)",
    )
    args = p.parse_args()

    if not 1 <= args.amount <= 255:
        raise ValueError("--amount must be 1..255")

    src = pathlib.Path(args.src).expanduser().resolve()
    out = pathlib.Path(args.out).expanduser().resolve()
    if not src.is_file():
        raise FileNotFoundError(src)
    if out.exists():
        raise FileExistsError(f"output already exists: {out}")

    original = src.read_bytes()
    info = inspect(original)
    data = bytearray(original)

    changed = 0
    changed_pixels = 0
    start = info["pixel_offset"]
    stride = info["stride"]
    width = info["width"]
    height = abs(info["height"])

    # 24-bit BMP stores B,G,R. Modify R only, leaving B/G and row padding exact.
    for row in range(height):
        row_start = start + row * stride
        for x in range(width):
            red_off = row_start + x * 3 + 2
            old = data[red_off]
            new = min(255, old + args.amount)
            if new != old:
                data[red_off] = new
                changed += 1
                changed_pixels += 1

    result = bytes(data)
    if result[:start] != original[:start]:
        raise RuntimeError("header/non-pixel prefix changed")
    if result[info["pixel_end"]:] != original[info["pixel_end"]:]:
        raise RuntimeError("trailing bytes changed")

    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(result)

    print("PT_BMP_VISIBLE_TEST_PATCH_V1")
    print(f"input={src}")
    print(f"input_sha256={sha256_bytes(original)}")
    print(f"output={out}")
    print(f"output_sha256={sha256_bytes(result)}")
    print(f"dimensions={info['width']}x{info['height']}")
    print("bpp=24")
    print(f"red_channel_increase={args.amount}")
    print(f"changed_pixel_bytes={changed}")
    print(f"changed_pixels={changed_pixels}")
    print("header_and_nonpixel_bytes_preserved=True")
    print("visible_test_patch_check=PASS")


if __name__ == "__main__":
    main()

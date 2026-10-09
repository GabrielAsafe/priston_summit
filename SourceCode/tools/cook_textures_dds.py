#!/usr/bin/env python3
"""Offline DDS cooker for Summit AssetStreamingV3.

The default world-texture policy mirrors the client's current large-texture
runtime policy: sources whose RGBA footprint is >= 1 MiB and whose top level
is block-compatible are cooked to DXT1/DXT5. Login and character-selection UI
used by login/character-selection (game/Login, game/UI,
game/GUI/Opening/Login/UI and StartImage/login) are additionally cooked
losslessly to A8R8G8B8 DDS so startup UI avoids D3DX source decode without DXT artifacts.
Item icons under image/Sinimage/Items are runtime-required: large block-compatible
icons use DXT1/DXT5 while smaller/non-block-compatible icons use lossless A8R8G8B8.
Mip generation remains opt-in for world textures.

The runtime keeps the original logical asset name and automatically prefers
cooked/<logical path with .dds>, falling back to the original file when the
cooked artifact is absent or invalid.
"""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import io
import json
import os
from pathlib import Path
import struct
import sys
import time
from typing import Iterable

try:
    from PIL import Image, ImageChops
except ImportError as exc:  # pragma: no cover - user-facing dependency error
    raise SystemExit(
        "Pillow is required only for the offline cooker. Install with: "
        "python3 -m pip install --user Pillow"
    ) from exc

DDS_MAGIC = b"DDS "
DDSD_CAPS = 0x00000001
DDSD_HEIGHT = 0x00000002
DDSD_WIDTH = 0x00000004
DDSD_PITCH = 0x00000008
DDSD_PIXELFORMAT = 0x00001000
DDSD_MIPMAPCOUNT = 0x00020000
DDSD_LINEARSIZE = 0x00080000
DDSCAPS_COMPLEX = 0x00000008
DDSCAPS_TEXTURE = 0x00001000
DDSCAPS_MIPMAP = 0x00400000
DDPF_ALPHAPIXELS = 0x00000001
DDPF_FOURCC = 0x00000004
DDPF_RGB = 0x00000040
FOURCC_DXT1 = struct.unpack("<I", b"DXT1")[0]
FOURCC_DXT5 = struct.unpack("<I", b"DXT5")[0]

SOURCE_EXTENSIONS = {".png", ".tga", ".bmp", ".jpg", ".jpeg", ".stg", ".sta"}
ALPHA_EXTENSIONS = {".png", ".tga", ".sta"}

# Same 18-byte protection key already proven by the PT Asset Tool's
# PT_PROTECTED_TGA_V2 adapter. Bytes 2..17 are c*c modulo 256; FULL18 also
# carries the historical G8 prefix (0x47, 0x38). KEEP_00_16 keeps bytes 0..1
# authored and protects only bytes 2..17.
PT_TGA_HEADER_KEY = bytes.fromhex("47380409101924314051647990a9c4e10021")
# Ten character-selection ship BMPs use a second protected 40-byte DIB header
# after the legacy first-14-byte BMP transform. This key and family proof come
# from PT Asset Tool's certified rare_support/rare_outliers implementation.
SHIP_BMP_DIB40_KEY = bytes.fromhex(
    "7a5b3a07e2cb92674a1beaa7724b02c79a5b1ac7844bf2a77c26ca6724d66207ba5bfa8722cb52e7"
)
TGA_FOOTER_SIGNATURE = b"TRUEVISION-XFILE.\x00"
TGA_FOOTER_SIZE = 26

DOCUMENTED_TEXTURE_OUTLIERS = {
    "field/ricarten/ch_tree1.tga": {
        "asset_classification": "NON_IMAGE_HTML_MISLABELED_TGA",
        "cook_policy": "DO_NOT_FORCE_DDS",
        "runtime_note": "PT Asset Tool identifies this physical file as an IIS/HTML error document.",
    },
    "effect/assaeffect/monsterfury/m_fury.tga": {
        "asset_classification": "TGA_ZERO_FILLED_NON_IMAGE_PLACEHOLDER",
        "cook_policy": "DO_NOT_FORCE_DDS",
        "runtime_note": "PT Asset Tool identifies this physical file as zero-filled/non-image.",
    },
    "effect/particle/archivotmp.tga": {
        "asset_classification": "TGA_DIMENSIONS_SINGLE_BIT_CORRUPT_RECOVERABLE_PREVIEW_ONLY",
        "cook_policy": "DO_NOT_FORCE_DDS_RUNTIME_UNPROVEN",
        "runtime_note": "PT Asset Tool can recover it for preview only; old-client runtime validity is intentionally unproven.",
    },
    "effect/assaeffect/force.bmp": {
        "asset_classification": "INVALID_HISTORICAL_ASSET_UNSUPPORTED",
        "cook_policy": "DO_NOT_FORCE_DDS",
        "runtime_note": "Known PT Asset Tool rare outlier; no certified runtime decode adapter.",
    },
    "effect/sky/s.bmp": {
        "asset_classification": "NON_IMAGE_TEXT_CONFIG_MISLABELED_BMP",
        "cook_policy": "DO_NOT_FORCE_DDS",
        "runtime_note": "Observed payload begins with ASCII server/config text rather than a BMP header; keep as non-image evidence until reachability decides recovery/quarantine review.",
    },
}

SKIP_CLASS_BY_REASON = {
    "below-runtime-dxt-threshold": "VALID_DECODE_POLICY_THRESHOLD",
    "too-small-for-dxt": "VALID_DECODE_DXT_TOO_SMALL",
    "top-level-not-block-aligned": "VALID_DECODE_DXT_ALIGNMENT",
    "login-ui-lossless": "VALID_DECODE_LOSSLESS_UI",
}


def classify_skip_reason(reason: str) -> str:
    return SKIP_CLASS_BY_REASON.get(str(reason or ""), "VALID_DECODE_POLICY_SKIP")


def documented_integrity_state(meta: dict[str, object]) -> str:
    cls = str(meta.get("asset_classification") or "").upper()
    if "PREVIEW_ONLY" in str(meta.get("cook_policy") or "").upper() or "RECOVERABLE_PREVIEW_ONLY" in cls:
        return "PREVIEW_RECOVERABLE_RUNTIME_UNPROVEN"
    if "NON_IMAGE" in cls or "ZERO_FILLED" in cls or "INVALID_HISTORICAL" in cls:
        return "KNOWN_INVALID_OR_NON_IMAGE"
    return "KNOWN_OUTLIER_RUNTIME_UNPROVEN"

ITEM_ICON_PREFIXES = (
    "image/sinimage/items/",
)

LOGIN_UI_PREFIXES = (
    "game/login/",
    "game/ui/",
    "game/gui/opening/login/ui/",
    "startimage/login/",
)


def _tga_header_fields(header: bytes) -> dict[str, int]:
    if len(header) != 18:
        raise ValueError("TGA header must be exactly 18 bytes")
    return {
        "id_length": header[0],
        "color_map_type": header[1],
        "image_type": header[2],
        "color_map_first": struct.unpack_from("<H", header, 3)[0],
        "color_map_length": struct.unpack_from("<H", header, 5)[0],
        "color_map_depth": header[7],
        "x_origin": struct.unpack_from("<H", header, 8)[0],
        "y_origin": struct.unpack_from("<H", header, 10)[0],
        "width": struct.unpack_from("<H", header, 12)[0],
        "height": struct.unpack_from("<H", header, 14)[0],
        "pixel_depth": header[16],
        "descriptor": header[17],
    }


def _tga_header_plausible(fields: dict[str, int]) -> bool:
    return (
        fields["color_map_type"] == 0
        and fields["image_type"] in (2, 10)
        and 0 < fields["width"] <= 8192
        and 0 < fields["height"] <= 8192
        and fields["pixel_depth"] in (24, 32)
    )


def _tga_footer_layout(data: bytes) -> tuple[bool, int, int, int]:
    if len(data) < TGA_FOOTER_SIZE or data[-18:] != TGA_FOOTER_SIGNATURE:
        return False, len(data), 0, 0
    footer = len(data) - TGA_FOOTER_SIZE
    extension_offset = struct.unpack_from("<I", data, footer)[0]
    developer_offset = struct.unpack_from("<I", data, footer + 4)[0]
    return True, footer, extension_offset, developer_offset


def _tga_payload_bounds(data: bytes, fields: dict[str, int]) -> tuple[int, int]:
    start = 18 + int(fields["id_length"])
    if start > len(data):
        raise ValueError("TGA id/payload start outside file")
    has_footer, footer, extension_offset, developer_offset = _tga_footer_layout(data)
    end = footer if has_footer else len(data)
    for offset in (extension_offset, developer_offset):
        if start <= offset < end:
            end = offset
    if end < start:
        raise ValueError("TGA payload boundary precedes payload start")
    return start, end


def _validate_tga_candidate(data: bytes, header: bytes) -> None:
    """Fail-closed structural proof for protected TGA candidate headers."""
    fields = _tga_header_fields(header)
    if not _tga_header_plausible(fields):
        raise ValueError("implausible protected TGA header")
    start, end = _tga_payload_bounds(data, fields)
    pixel_bytes = fields["pixel_depth"] // 8
    expected_pixels = fields["width"] * fields["height"]

    if fields["image_type"] == 2:
        expected_bytes = expected_pixels * pixel_bytes
        if end - start != expected_bytes:
            raise ValueError(
                f"protected TGA type-2 payload mismatch: actual={end - start} expected={expected_bytes}"
            )
        return

    pos = start
    decoded_pixels = 0
    while pos < end:
        packet = data[pos]
        pos += 1
        count = (packet & 0x7F) + 1
        if decoded_pixels + count > expected_pixels:
            raise ValueError("protected TGA RLE pixel overrun")
        body = pixel_bytes if (packet & 0x80) else count * pixel_bytes
        if pos + body > end:
            raise ValueError("protected TGA RLE packet truncated")
        pos += body
        decoded_pixels += count
    if pos != end or decoded_pixels != expected_pixels:
        raise ValueError(
            f"protected TGA RLE closure mismatch: decoded={decoded_pixels} expected={expected_pixels}"
        )


def _decode_protected_tga_header(data: bytes) -> tuple[bytes, str]:
    """Decode STANDARD, FULL18 (G8) and KEEP_00_16 protected TGA headers.

    KEEP_00_16 is already proven by PT Asset Tool's PT_PROTECTED_TGA_V2.
    Protected candidates are accepted only after exact structural closure.
    """
    if len(data) < 18:
        raise ValueError("TGA header shorter than 18 bytes")

    raw = data[:18]
    direct_fields = _tga_header_fields(raw)
    if _tga_header_plausible(direct_fields):
        return data, "STANDARD"

    candidates: list[tuple[str, bytes]] = []
    for mode in ("FULL18", "KEEP_00_16"):
        decoded = bytearray(18)
        if mode == "FULL18":
            for i in range(18):
                decoded[i] = (raw[i] - PT_TGA_HEADER_KEY[i]) & 0xFF
        else:
            decoded[0:2] = raw[0:2]
            for i in range(2, 18):
                decoded[i] = (raw[i] - PT_TGA_HEADER_KEY[i]) & 0xFF

        header = bytes(decoded)
        try:
            _validate_tga_candidate(data, header)
        except ValueError:
            continue
        candidates.append((mode, header))

    if len(candidates) == 1:
        mode, header = candidates[0]
        return header + data[18:], mode
    if len(candidates) > 1:
        raise ValueError(
            "protected TGA header ambiguous: "
            + ",".join(mode for mode, _ in candidates)
            + f" raw={raw.hex()}"
        )

    dimensions_only = _decode_dimensions_12_15_candidate(data)
    if dimensions_only is not None:
        return dimensions_only

    return data, "UNRESOLVED"


def _decode_dimensions_12_15_candidate(data: bytes) -> tuple[bytes, str] | None:
    """Decode the exact PT Asset Tool DIMENSIONS_12_15 physical TGA family.

    Bytes 0..11 and 16..17 must already form the proven standard true-color
    shape; only width/height bytes 12..15 are protected. The candidate is still
    accepted only after full payload closure, so this is not a dimensions guess.
    """
    if len(data) < 18:
        return None
    raw = data[:18]
    if not (
        raw[:12] == b"\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        and raw[16] in (24, 32)
    ):
        return None

    decoded = bytearray(raw)
    for i in range(12, 16):
        decoded[i] = (decoded[i] - PT_TGA_HEADER_KEY[i]) & 0xFF
    header = bytes(decoded)
    try:
        _validate_tga_candidate(data, header)
    except ValueError:
        return None
    return header + data[18:], "DIMENSIONS_12_15"


def _sourcecode_decrypt_bmp_header14(data: bytes) -> bytes:
    if len(data) < 14:
        raise ValueError("BMP header shorter than 14 bytes")
    out = bytearray(data[:14])
    if out[:2] != b"BM":
        out[:2] = b"BM"
        for c in range(2, 14):
            out[c] = (out[c] - (c * c)) & 0xFF
    out[2:6] = b"\x00\x00\x00\x00"
    return bytes(out)


def _decode_ship_bmp_candidate(data: bytes) -> bytes | None:
    """Return a standard-BMP equivalent for the proven chrselect ship family."""
    if len(data) < 56:
        return None
    out = bytearray(data)
    out[:14] = _sourcecode_decrypt_bmp_header14(data)
    for i, key in enumerate(SHIP_BMP_DIB40_KEY):
        out[14 + i] = (out[14 + i] - key) & 0xFF

    try:
        dib = struct.unpack_from("<I", out, 14)[0]
        width = struct.unpack_from("<i", out, 18)[0]
        height = struct.unpack_from("<i", out, 22)[0]
        planes = struct.unpack_from("<H", out, 26)[0]
        bpp = struct.unpack_from("<H", out, 28)[0]
        compression = struct.unpack_from("<I", out, 30)[0]
        image_size = struct.unpack_from("<I", out, 34)[0]
        offbits = struct.unpack_from("<I", out, 10)[0]
    except struct.error:
        return None

    if not (
        dib == 40
        and width > 0
        and height > 0
        and planes == 1
        and bpp == 24
        and compression == 0
        and offbits == 54
    ):
        return None

    expected_pixels = width * height * 3
    if image_size != expected_pixels:
        return None
    pixel_end = offbits + expected_pixels
    if pixel_end + 2 != len(data) or data[pixel_end:] != b"\x00\x00":
        return None

    if bytes(out[offbits:pixel_end]) != data[offbits:pixel_end]:
        return None
    if bytes(out[pixel_end:]) != data[pixel_end:]:
        return None

    struct.pack_into("<I", out, 2, len(out))
    if out[:2] != b"BM" or struct.unpack_from("<I", out, 14)[0] != 40:
        return None
    return bytes(out)


def decrypt_legacy_bytes_with_info(path: Path, data: bytes) -> tuple[bytes, dict[str, object]]:
    """Mirror legacy BMP/TGA deobfuscation and expose provenance metadata."""
    ext = path.suffix.lower()
    buf = bytearray(data)
    info: dict[str, object] = {"legacy_decode_mode": "STANDARD"}

    if ext == ".bmp":
        if len(buf) < 14:
            raise ValueError("BMP header shorter than 14 bytes")

        ship = _decode_ship_bmp_candidate(data)
        if ship is not None:
            info["legacy_decode_mode"] = "BMP_SHIP_DIB40"
            return ship, info

        buf[:14] = _sourcecode_decrypt_bmp_header14(data)
        if bytes(data[:2]) != b"BM":
            info["legacy_decode_mode"] = "BMP_HEADER_2_13"
        return bytes(buf), info

    if ext in {".tga", ".sta"}:
        decoded, mode = _decode_protected_tga_header(data)
        info["legacy_decode_mode"] = f"TGA_{mode}"
        return decoded, info

    return bytes(buf), info


def decrypt_legacy_bytes(path: Path, data: bytes) -> bytes:
    """Compatibility wrapper retained for existing tests/importers."""
    decoded, _ = decrypt_legacy_bytes_with_info(path, data)
    return decoded


def open_legacy_image_with_info(path: Path) -> tuple[Image.Image, dict[str, object]]:
    raw, decode_info = decrypt_legacy_bytes_with_info(path, path.read_bytes())
    image = Image.open(io.BytesIO(raw))
    image.load()
    # D3DXCreateTextureFromFileInMemoryEx currently receives 0xFFF0FFFF as a
    # color key. Bake the same exact RGB key into alpha offline so the cooked
    # import can use colorKey=0 without changing authored transparency.
    rgba = image.convert("RGBA")
    rgb = rgba.convert("RGB")
    key = Image.new("RGB", rgba.size, (240, 255, 255))
    difference = ImageChops.difference(rgb, key).convert("L")
    non_key_mask = difference.point(lambda value: 0 if value == 0 else 255)
    alpha = ImageChops.multiply(rgba.getchannel("A"), non_key_mask)
    rgba.putalpha(alpha)
    decode_info["decoded_width"] = rgba.width
    decode_info["decoded_height"] = rgba.height
    return rgba, decode_info


def open_legacy_image(path: Path) -> Image.Image:
    """Compatibility wrapper returning only the decoded image."""
    image, _ = open_legacy_image_with_info(path)
    return image


def mip_chain(image: Image.Image) -> list[Image.Image]:
    levels = [image]
    while levels[-1].width > 1 or levels[-1].height > 1:
        current = levels[-1]
        size = (max(1, current.width // 2), max(1, current.height // 2))
        levels.append(current.resize(size, Image.Resampling.LANCZOS))
    return levels


def pillow_dds_payload(image: Image.Image, pixel_format: str) -> bytes:
    temp = io.BytesIO()
    image.save(temp, format="DDS", pixel_format=pixel_format)
    encoded = temp.getvalue()
    if not encoded.startswith(DDS_MAGIC) or len(encoded) < 128:
        raise ValueError("Pillow returned an invalid DDS stream")
    # DXT1/DXT5 generated by Pillow use the classic 128-byte DDS header.
    return encoded[128:]


def make_dds_header(width: int, height: int, mip_count: int, pixel_format: str) -> bytes:
    if pixel_format == "DXT1":
        fourcc = FOURCC_DXT1
        block_bytes = 8
    elif pixel_format == "DXT5":
        fourcc = FOURCC_DXT5
        block_bytes = 16
    else:
        raise ValueError(f"unsupported cooked format: {pixel_format}")

    linear_size = max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * block_bytes
    flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE
    caps = DDSCAPS_TEXTURE
    if mip_count > 1:
        flags |= DDSD_MIPMAPCOUNT
        caps |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP

    header = bytearray()
    header += DDS_MAGIC
    header += struct.pack(
        "<7I",
        124,
        flags,
        height,
        width,
        linear_size,
        0,  # depth
        mip_count,
    )
    header += struct.pack("<11I", *((0,) * 11))
    header += struct.pack("<8I", 32, DDPF_FOURCC, fourcc, 0, 0, 0, 0, 0)
    header += struct.pack("<5I", caps, 0, 0, 0, 0)
    if len(header) != 128:
        raise AssertionError(f"invalid DDS header size: {len(header)}")
    return bytes(header)


def make_argb8888_dds_header(width: int, height: int) -> bytes:
    pitch = width * 4
    flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_PITCH
    header = bytearray()
    header += DDS_MAGIC
    header += struct.pack("<7I", 124, flags, height, width, pitch, 0, 1)
    header += struct.pack("<11I", *((0,) * 11))
    # Classic DDS A8R8G8B8. On little-endian hosts the byte payload is BGRA.
    header += struct.pack(
        "<8I",
        32,
        DDPF_RGB | DDPF_ALPHAPIXELS,
        0,
        32,
        0x00FF0000,
        0x0000FF00,
        0x000000FF,
        0xFF000000,
    )
    header += struct.pack("<5I", DDSCAPS_TEXTURE, 0, 0, 0, 0)
    if len(header) != 128:
        raise AssertionError(f"invalid A8R8G8B8 DDS header size: {len(header)}")
    return bytes(header)


def encode_argb8888_dds(image: Image.Image) -> tuple[bytes, int]:
    rgba = image.convert("RGBA")
    payload = rgba.tobytes("raw", "BGRA")
    return make_argb8888_dds_header(rgba.width, rgba.height) + payload, 1


def is_login_ui(relative: Path) -> bool:
    normalized = relative.as_posix().lower()
    return any(normalized.startswith(prefix) for prefix in LOGIN_UI_PREFIXES)


def is_item_icon(relative: Path) -> bool:
    """Return whether *relative* belongs to the legacy item-icon namespace."""
    normalized = relative.as_posix().lower()
    return any(normalized.startswith(prefix) for prefix in ITEM_ICON_PREFIXES)


def item_icon_pixel_format(image: Image.Image, source: Path, min_rgba_bytes: int) -> tuple[str, str]:
    """Choose a cooked DDS format for runtime-required item icons.

    Large block-compatible icons retain the existing DXT1/DXT5 policy. Small
    icons and dimensions that cannot be represented by DXT are emitted
    losslessly as A8R8G8B8 rather than being skipped.
    """
    dxt_eligible, _ = should_cook(image, min_rgba_bytes)
    if dxt_eligible:
        return ("DXT5" if source.suffix.lower() in ALPHA_EXTENSIONS else "DXT1"), "item-icon-runtime-required-dxt"
    return "A8R8G8B8", "item-icon-runtime-required-lossless"


def probe_dds_header(path: Path) -> tuple[int, int, str]:
    """Read DDS dimensions/pixel format without decoding the full texture."""
    data = path.read_bytes()[:128]
    if len(data) < 128 or data[:4] != DDS_MAGIC:
        raise ValueError("invalid DDS header")
    height = struct.unpack_from("<I", data, 12)[0]
    width = struct.unpack_from("<I", data, 16)[0]
    flags = struct.unpack_from("<I", data, 80)[0]
    fourcc = struct.unpack_from("<I", data, 84)[0]
    rgb_bits = struct.unpack_from("<I", data, 88)[0]
    if flags & DDPF_FOURCC:
        if fourcc == FOURCC_DXT1:
            pixel_format = "DXT1"
        elif fourcc == FOURCC_DXT5:
            pixel_format = "DXT5"
        else:
            raise ValueError(f"unsupported cooked format fourcc=0x{fourcc:08x}")
    elif flags & DDPF_RGB and rgb_bits == 32:
        pixel_format = "A8R8G8B8"
    else:
        raise ValueError("unsupported cooked format")
    return width, height, pixel_format


def encode_dds(image: Image.Image, pixel_format: str, with_mipmaps: bool) -> tuple[bytes, int]:
    levels = mip_chain(image) if with_mipmaps else [image]
    payload = bytearray()
    for level in levels:
        payload += pillow_dds_payload(level.convert("RGBA"), pixel_format)
    return make_dds_header(image.width, image.height, len(levels), pixel_format) + payload, len(levels)


def output_path_for(source_root: Path, output_root: Path, source: Path) -> Path:
    relative = source.relative_to(source_root)
    return (output_root / relative).with_suffix(".dds")


def discover_sources(source_root: Path, output_root: Path, login_ui_only: bool = False, item_icons_only: bool = False) -> Iterable[Path]:
    output_resolved = output_root.resolve()
    for path in source_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in SOURCE_EXTENSIONS:
            continue
        relative = path.relative_to(source_root)
        if login_ui_only and not is_login_ui(relative):
            continue
        if item_icons_only and not is_item_icon(relative):
            continue
        try:
            path.resolve().relative_to(output_resolved)
            continue
        except ValueError:
            pass
        yield path


def should_cook(image: Image.Image, min_rgba_bytes: int) -> tuple[bool, str]:
    rgba_bytes = image.width * image.height * 4
    if image.width < 4 or image.height < 4:
        return False, "too-small-for-dxt"
    if image.width % 4 or image.height % 4:
        return False, "top-level-not-block-aligned"
    if rgba_bytes < min_rgba_bytes:
        return False, "below-runtime-dxt-threshold"
    return True, "eligible"



def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def classify_failure(exc: Exception) -> str:
    message = str(exc).casefold()
    if "header shorter" in message:
        return "HEADER_TOO_SHORT"
    if "truncated file read" in message or "truncated" in message:
        return "TRUNCATED_INPUT"
    if "unsupported bmp header" in message:
        return "UNSUPPORTED_BMP_HEADER"
    if "decompression bomb" in message or "exceeds limit" in message:
        return "IMPLAUSIBLE_IMAGE_DIMENSIONS"
    if "cannot identify image file" in message:
        return "UNRECOGNIZED_LEGACY_IMAGE_ENCODING"
    if "invalid dds" in message or "unsupported cooked format" in message:
        return "DDS_ENCODE_FAILURE"
    return "UNCLASSIFIED_FAILURE"


def source_probe(path: Path) -> dict[str, object]:
    try:
        st = path.stat()
        with path.open("rb") as f:
            header = f.read(32)
        return {
            "source_bytes": st.st_size,
            "source_mtime_ns": st.st_mtime_ns,
            "source_sha256": sha256_file(path),
            "source_header_hex": header.hex(),
        }
    except Exception as exc:
        return {"source_probe_error": f"{type(exc).__name__}:{exc}"}


def failure_entry(source_root: Path, output_root: Path, source: Path, target: Path, stage: str, exc: Exception) -> dict[str, object]:
    try:
        relative = source.relative_to(source_root).as_posix()
    except ValueError:
        relative = source.as_posix()
    row: dict[str, object] = {
        "source": relative,
        "cooked": target.relative_to(output_root).as_posix() if target.is_relative_to(output_root) else target.as_posix(),
        "extension": source.suffix.lower(),
        "status": "error",
        "failure_stage": stage,
        "failure_class": classify_failure(exc),
        "error_type": type(exc).__name__,
        "error_message": str(exc),
    }
    row.update(source_probe(source))
    documented = DOCUMENTED_TEXTURE_OUTLIERS.get(relative.casefold())
    if documented:
        row["documented_outlier"] = True
        row.update(documented)
        row["outlier_reason"] = str(documented.get("asset_classification") or "KNOWN_TEXTURE_OUTLIER")
        row["asset_integrity_state"] = documented_integrity_state(documented)
        row["retention_decision_required"] = True
    else:
        row["documented_outlier"] = False
        row["asset_integrity_state"] = "UNRESOLVED_DECODE_FAILURE"
        row["retention_decision_required"] = True
    row["deletion_authorized"] = False
    return row


def load_json_object(path: Path) -> dict[str, object]:
    if not path.is_file():
        return {}
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return {}
    return value if isinstance(value, dict) else {}


def failure_sources_from_manifest(path: Path) -> set[str]:
    data = load_json_object(path)
    out: set[str] = set()
    for row in data.get("entries", []) if isinstance(data.get("entries"), list) else []:
        if isinstance(row, dict) and str(row.get("status") or "").casefold() == "error":
            rel = str(row.get("source") or "").replace("\\", "/").lstrip("./")
            if rel:
                out.add(rel.casefold())
    for row in data.get("failures", []) if isinstance(data.get("failures"), list) else []:
        if isinstance(row, dict):
            rel = str(row.get("source") or "").replace("\\", "/").lstrip("./")
            if rel:
                out.add(rel.casefold())
    return out


def failure_sources_from_log(path: Path, source_root: Path) -> set[str]:
    out: set[str] = set()
    if not path.is_file():
        raise FileNotFoundError(f"retry log does not exist: {path}")
    prefix = "ERROR "
    root_text = str(source_root).replace("\\", "/").rstrip("/") + "/"
    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if not raw.startswith(prefix):
            continue
        payload = raw[len(prefix):]
        # Existing cooker format is: ERROR <absolute-or-relative-path>: <message>.
        # Split on ': ' from the right so paths remain intact on Windows drives.
        asset = payload.rsplit(": ", 1)[0].replace("\\", "/")
        if asset.startswith(root_text):
            asset = asset[len(root_text):]
        else:
            try:
                asset = Path(asset).resolve().relative_to(source_root).as_posix()
            except Exception:
                asset = asset.lstrip("./")
        if asset:
            out.add(asset.casefold())
    return out


def inventory_stats(entries: list[dict[str, object]]) -> dict[str, object]:
    statuses = Counter(str(x.get("status") or "unknown") for x in entries)
    failures = [x for x in entries if str(x.get("status") or "") == "error"]
    skips = [x for x in entries if str(x.get("status") or "") == "skipped"]
    return {
        "entry_count": len(entries),
        "status_counts": dict(sorted(statuses.items())),
        "failure_count": len(failures),
        "failure_class_counts": dict(sorted(Counter(str(x.get("failure_class") or "UNCLASSIFIED_FAILURE") for x in failures).items())),
        "failure_stage_counts": dict(sorted(Counter(str(x.get("failure_stage") or "unknown") for x in failures).items())),
        "documented_outlier_failure_count": sum(bool(x.get("documented_outlier")) for x in failures),
        "skip_count": len(skips),
        "skip_class_counts": dict(sorted(Counter(str(x.get("skip_class") or "UNCLASSIFIED_SKIP") for x in skips).items())),
        "asset_integrity_state_counts": dict(sorted(Counter(str(x.get("asset_integrity_state") or "UNSPECIFIED") for x in entries).items())),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Cook Summit textures to DDS for AssetStreamingV3")
    parser.add_argument("--input", default=".", help="client asset root (default: current directory)")
    parser.add_argument("--output", default=None, help="cooked root (default: <input>/cooked)")
    parser.add_argument(
        "--min-rgba-mib",
        type=float,
        default=1.0,
        help="cook 512x512-and-larger RGBA footprints by default; default 1 MiB",
    )
    parser.add_argument("--force", action="store_true", help="rebuild even when DDS is newer than source")
    parser.add_argument(
        "--no-login-ui-lossless",
        dest="login_ui_lossless",
        action="store_false",
        help="do not cook login/selection UI roots as lossless A8R8G8B8 DDS",
    )
    parser.set_defaults(login_ui_lossless=True)
    parser.add_argument("--mipmaps", action="store_true", help="emit a full mip chain (off by default for visual parity with current V3)")
    parser.add_argument("--dry-run", action="store_true", help="scan and report without writing DDS files")
    parser.add_argument(
        "--login-ui-only",
        action="store_true",
        help="only scan/cook login and character-selection UI roots",
    )
    parser.add_argument(
        "--item-icons-only",
        action="store_true",
        help="only scan/cook image/Sinimage/Items runtime item-icon roots",
    )
    parser.add_argument(
        "--no-item-icons-required",
        dest="item_icons_required",
        action="store_false",
        help="restore legacy threshold behavior for item icons (A/B only)",
    )
    parser.set_defaults(item_icons_required=True)
    parser.add_argument("--quiet", action="store_true", help="suppress per-file COOKED/WOULD_COOK output")
    parser.add_argument("--limit", type=int, default=0, help="cook at most N eligible files (0 = unlimited)")
    parser.add_argument("--manifest", default=None, help="manifest JSON path (default: <output>/manifest.json)")
    parser.add_argument("--failed-manifest", default=None, help="failure JSON path (default: <output>/failed.json)")
    parser.add_argument(
        "--retry-failed",
        nargs="?",
        const="__DEFAULT__",
        default=None,
        metavar="MANIFEST",
        help="retry only sources recorded as error in MANIFEST; without a path uses --manifest/<output>/manifest.json",
    )
    parser.add_argument(
        "--retry-log",
        default=None,
        help="retry only ERROR paths parsed from a previous cooker console log; useful to bootstrap a V2 manifest",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    source_root = Path(args.input).resolve()
    output_root = Path(args.output).resolve() if args.output else source_root / "cooked"
    manifest_path = Path(args.manifest).resolve() if args.manifest else output_root / "manifest.json"
    failed_manifest_path = Path(args.failed_manifest).resolve() if args.failed_manifest else output_root / "failed.json"
    min_rgba_bytes = int(max(0.0, args.min_rgba_mib) * 1024 * 1024)

    if args.login_ui_only and args.item_icons_only:
        print("ERROR --login-ui-only and --item-icons-only are mutually exclusive", file=sys.stderr)
        return 2

    if not source_root.is_dir():
        print(f"ERROR input directory does not exist: {source_root}", file=sys.stderr)
        return 2

    prior_manifest = load_json_object(manifest_path)
    prior_entries = {
        str(row.get("source") or "").replace("\\", "/").lstrip("./").casefold(): dict(row)
        for row in (prior_manifest.get("entries") or [])
        if isinstance(row, dict) and row.get("source")
    }

    retry_sources: set[str] | None = None
    retry_inputs: list[str] = []
    if args.retry_failed is not None:
        retry_manifest = manifest_path if args.retry_failed == "__DEFAULT__" else Path(args.retry_failed).resolve()
        retry_sources = failure_sources_from_manifest(retry_manifest)
        retry_inputs.append(f"manifest:{retry_manifest}")
    if args.retry_log:
        log_sources = failure_sources_from_log(Path(args.retry_log).resolve(), source_root)
        retry_sources = (retry_sources or set()) | log_sources
        retry_inputs.append(f"log:{Path(args.retry_log).resolve()}")

    stats = {"scanned": 0, "eligible": 0, "cooked": 0, "up_to_date": 0, "skipped": 0, "errors": 0}
    run_entries: list[dict[str, object]] = []
    started = time.time()

    for source in discover_sources(source_root, output_root, args.login_ui_only, args.item_icons_only):
        relative = source.relative_to(source_root)
        relative_key = relative.as_posix().casefold()
        if retry_sources is not None and relative_key not in retry_sources:
            continue

        stats["scanned"] += 1
        target = output_path_for(source_root, output_root, source)
        item_icon = args.item_icons_required and is_item_icon(relative)
        stage = "incremental-check"
        try:
            # Fast incremental path remains valid for ordinary assets. Item
            # icons are decoded before reuse so stale DDS dimensions/formats
            # cannot be accepted solely because their mtime is newer.
            if not item_icon and not args.force and target.exists() and target.stat().st_mtime_ns >= source.stat().st_mtime_ns:
                stats["eligible"] += 1
                stats["up_to_date"] += 1
                prior = dict(prior_entries.get(relative_key) or {})
                preserved = {
                    key: prior[key]
                    for key in (
                        "legacy_decode_mode", "decoded_width", "decoded_height",
                        "width", "height", "format", "mips", "asset_integrity_state",
                        "skip_class", "skip_reason", "documented_outlier",
                        "asset_classification", "cook_policy", "runtime_note", "outlier_reason",
                    )
                    if key in prior
                }
                run_entries.append({
                    "source": relative.as_posix(),
                    "cooked": target.relative_to(output_root).as_posix(),
                    "extension": source.suffix.lower(),
                    "source_bytes": source.stat().st_size,
                    "source_mtime_ns": source.stat().st_mtime_ns,
                    "cooked_bytes": target.stat().st_size,
                    "cooked_mtime_ns": target.stat().st_mtime_ns,
                    "status": "up-to-date",
                    "asset_integrity_state": preserved.get("asset_integrity_state", "COOK_ARTIFACT_UP_TO_DATE"),
                    **preserved,
                })
                continue

            stage = "decode"
            image, decode_info = open_legacy_image_with_info(source)
            lossless_login_ui = args.login_ui_lossless and is_login_ui(relative)
            if lossless_login_ui:
                eligible, reason = True, "login-ui-lossless"
                pixel_format = "A8R8G8B8"
            elif item_icon:
                eligible = True
                pixel_format, reason = item_icon_pixel_format(image, source, min_rgba_bytes)
            else:
                eligible, reason = should_cook(image, min_rgba_bytes)
                pixel_format = "DXT5" if source.suffix.lower() in ALPHA_EXTENSIONS else "DXT1"
            if not eligible:
                stats["skipped"] += 1
                run_entries.append({
                    "source": relative.as_posix(),
                    "cooked": target.relative_to(output_root).as_posix(),
                    "extension": source.suffix.lower(),
                    "width": image.width,
                    "height": image.height,
                    "source_bytes": source.stat().st_size,
                    "source_mtime_ns": source.stat().st_mtime_ns,
                    "status": "skipped",
                    "skip_reason": reason,
                    "skip_class": classify_skip_reason(reason),
                    "asset_integrity_state": "VALID_IMAGE_POLICY_SKIPPED",
                    "decoded_ok": True,
                    "runtime_dds_expected": False,
                    "deletion_authorized": False,
                    **decode_info,
                })
                continue

            stats["eligible"] += 1

            if item_icon and not args.force and target.exists() and target.stat().st_mtime_ns >= source.stat().st_mtime_ns:
                try:
                    cooked_width, cooked_height, cooked_format = probe_dds_header(target)
                except Exception:
                    cooked_width = cooked_height = 0
                    cooked_format = "INVALID"
                if (cooked_width, cooked_height, cooked_format) == (image.width, image.height, pixel_format):
                    stats["up_to_date"] += 1
                    run_entries.append({
                        "source": relative.as_posix(),
                        "cooked": target.relative_to(output_root).as_posix(),
                        "extension": source.suffix.lower(),
                        "source_bytes": source.stat().st_size,
                        "source_mtime_ns": source.stat().st_mtime_ns,
                        "cooked_bytes": target.stat().st_size,
                        "cooked_mtime_ns": target.stat().st_mtime_ns,
                        "status": "up-to-date",
                        "asset_integrity_state": "COOK_ARTIFACT_UP_TO_DATE",
                        "runtime_dds_expected": True,
                        "cook_reason": reason,
                        "format": pixel_format,
                        "width": image.width,
                        "height": image.height,
                        **decode_info,
                    })
                    continue

            if args.dry_run:
                run_entries.append({
                    "source": relative.as_posix(),
                    "cooked": target.relative_to(output_root).as_posix(),
                    "extension": source.suffix.lower(),
                    "width": image.width,
                    "height": image.height,
                    "format": pixel_format,
                    "status": "would-cook",
                    "cook_reason": reason,
                    "runtime_dds_expected": True,
                    "asset_integrity_state": "VALID_IMAGE_WOULD_COOK",
                    "deletion_authorized": False,
                    **decode_info,
                })
                if not args.quiet:
                    print(f"WOULD_COOK {relative.as_posix()} -> {pixel_format} {image.width}x{image.height}")
                continue

            stage = "encode"
            if pixel_format == "A8R8G8B8":
                encoded, mip_count = encode_argb8888_dds(image)
            else:
                encoded, mip_count = encode_dds(image, pixel_format, args.mipmaps)

            stage = "write"
            target.parent.mkdir(parents=True, exist_ok=True)
            temp = target.with_suffix(target.suffix + ".tmp")
            temp.write_bytes(encoded)
            os.replace(temp, target)
            stats["cooked"] += 1
            run_entries.append({
                "source": relative.as_posix(),
                "cooked": target.relative_to(output_root).as_posix(),
                "extension": source.suffix.lower(),
                "width": image.width,
                "height": image.height,
                "mips": mip_count,
                "format": pixel_format,
                "bytes": len(encoded),
                "source_bytes": source.stat().st_size,
                "source_mtime_ns": source.stat().st_mtime_ns,
                "cooked_mtime_ns": target.stat().st_mtime_ns,
                "status": "cooked",
                "cook_reason": reason,
                "asset_integrity_state": "VALID_IMAGE_COOKED",
                "decoded_ok": True,
                "runtime_dds_expected": True,
                "deletion_authorized": False,
                **decode_info,
            })
            if not args.quiet:
                print(f"COOKED {relative.as_posix()} -> {pixel_format} {image.width}x{image.height} ({len(encoded)} bytes)")

            if args.limit and stats["cooked"] >= args.limit:
                break
        except Exception as exc:
            stats["errors"] += 1
            row = failure_entry(source_root, output_root, source, target, stage, exc)
            run_entries.append(row)
            print(f"ERROR {source}: {exc}", file=sys.stderr)

    if retry_sources is not None:
        merged = dict(prior_entries)
        for row in run_entries:
            merged[str(row.get("source") or "").casefold()] = row
        entries = [merged[k] for k in sorted(merged)]
    else:
        entries = sorted(run_entries, key=lambda x: str(x.get("source") or "").casefold())

    failures = [x for x in entries if str(x.get("status") or "") == "error"]
    inv_stats = inventory_stats(entries)

    if not args.dry_run:
        output_root.mkdir(parents=True, exist_ok=True)
        manifest = {
            "format": "SUMMIT_TEXTURE_COOK_MANIFEST_V2",
            "version": 2,
            "metadata_revision": 3,
            "generated_at": time.time(),
            "policy": {
                "min_rgba_mib": args.min_rgba_mib,
                "formats": {
                    "opaque": "DXT1",
                    "alpha_capable": "DXT5",
                    "login_ui": "A8R8G8B8" if args.login_ui_lossless else "threshold-policy",
                },
                "mipmaps": "full-chain" if args.mipmaps else "top-level-only",
                "scope": "login-ui-only" if args.login_ui_only else ("item-icons-only" if args.item_icons_only else "all-assets"),
                "source_assets_are_never_removed_by_cooker": True,
                "item_icons_runtime_required": args.item_icons_required,
                "item_icon_policy": "DXT1_DXT5_WHEN_DEFAULT_ELIGIBLE_ELSE_A8R8G8B8",
                "failed_entries_are_persisted": True,
                "protected_tga_modes": ["FULL18", "KEEP_00_16", "DIMENSIONS_12_15"],
                "protected_tga_contract": "PT_ASSET_TOOL_PROVEN_TGA_FAMILIES_FAIL_CLOSED",
                "protected_bmp_families": ["BMP_SHIP_DIB40_PROTECTED_EXACT"],
                "documented_outliers_are_kept_as_failures": True,
                "skipped_entries_are_explicitly_classified": True,
                "decode_failure_never_authorizes_delete": True,
                "asset_integrity_state_is_evidence_only": True,
            },
            "source_root": str(source_root),
            "output_root": str(output_root),
            "run": {
                "mode": "retry" if retry_sources is not None else "full",
                "retry_inputs": retry_inputs,
                "retry_requested_count": len(retry_sources or []),
                "stats": stats,
            },
            # Keep the historical top-level field for old consumers.
            "stats": stats,
            "inventory_stats": inv_stats,
            "failure_count": len(failures),
            "failure_class_counts": inv_stats["failure_class_counts"],
            "entries": entries,
            "failures": failures,
            "failed_manifest": str(failed_manifest_path),
        }
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

        failed_manifest = {
            "format": "SUMMIT_TEXTURE_COOK_FAILURES_V1",
            "metadata_revision": 3,
            "generated_at": manifest["generated_at"],
            "source_root": str(source_root),
            "output_root": str(output_root),
            "manifest": str(manifest_path),
            "failure_count": len(failures),
            "failure_class_counts": inv_stats["failure_class_counts"],
            "failure_stage_counts": inv_stats["failure_stage_counts"],
            "documented_outlier_failure_count": inv_stats["documented_outlier_failure_count"],
            "asset_integrity_state_counts": dict(sorted(Counter(str(x.get("asset_integrity_state") or "UNSPECIFIED") for x in failures).items())),
            "entries": failures,
        }
        failed_manifest_path.parent.mkdir(parents=True, exist_ok=True)
        failed_manifest_path.write_text(json.dumps(failed_manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    elapsed = time.time() - started
    print(
        "RESULT "
        + " ".join(f"{key}={value}" for key, value in stats.items())
        + f" remaining_failures={len(failures)} elapsed={elapsed:.2f}s"
    )
    return 1 if stats["errors"] else 0


if __name__ == "__main__":
    raise SystemExit(main())

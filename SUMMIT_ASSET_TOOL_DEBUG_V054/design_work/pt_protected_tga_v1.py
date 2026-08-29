#!/usr/bin/env python3
"""Summit/Priston protected TGA header adapter V1.

The protected family keeps TGA pixel payload/footer intact and transforms only
its 18-byte header by byte-wise addition modulo 256.

Two observed header modes in the client survey:
  FULL18:     bytes 0..17 += KEY
  KEEP_00_16: bytes 0..1 stay 00 00, bytes 2..17 += KEY[2:]

This tool never modifies the input file. `build` preserves the original
protected header and original 26-byte TGA 2.0 footer exactly.
"""
from __future__ import annotations
import argparse, csv, hashlib, json, struct
from pathlib import Path

TOOL = "PT_PROTECTED_TGA_V1"
KEY = bytes.fromhex("47380409101924314051647990a9c4e10021")
FOOTER_SIG = b"TRUEVISION-XFILE.\x00"
FOOTER_SIZE = 26


def sha(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest()


def header_fields(h: bytes):
    if len(h) != 18:
        raise ValueError("TGA header must be 18 bytes")
    return {
        "id_length": h[0],
        "color_map_type": h[1],
        "image_type": h[2],
        "color_map_first": struct.unpack_from("<H", h, 3)[0],
        "color_map_length": struct.unpack_from("<H", h, 5)[0],
        "color_map_depth": h[7],
        "x_origin": struct.unpack_from("<H", h, 8)[0],
        "y_origin": struct.unpack_from("<H", h, 10)[0],
        "width": struct.unpack_from("<H", h, 12)[0],
        "height": struct.unpack_from("<H", h, 14)[0],
        "pixel_depth": h[16],
        "descriptor": h[17],
    }


def plausible(fields: dict) -> bool:
    return (
        fields["id_length"] <= 255
        and fields["color_map_type"] == 0
        and fields["image_type"] in (2, 10)
        and 0 < fields["width"] <= 8192
        and 0 < fields["height"] <= 8192
        and fields["pixel_depth"] in (24, 32)
    )


def has_footer(data: bytes) -> bool:
    return len(data) >= FOOTER_SIZE and data[-18:] == FOOTER_SIG


def decode_candidate(raw: bytes, mode: str) -> bytes:
    if len(raw) != 18:
        raise ValueError("raw header must be 18 bytes")
    out = bytearray(18)
    if mode == "FULL18":
        for i in range(18):
            out[i] = (raw[i] - KEY[i]) & 0xFF
    elif mode == "KEEP_00_16":
        out[0:2] = raw[0:2]
        for i in range(2, 18):
            out[i] = (raw[i] - KEY[i]) & 0xFF
    else:
        raise ValueError(mode)
    return bytes(out)


def protect_header(std: bytes, mode: str) -> bytes:
    if len(std) != 18:
        raise ValueError("standard header must be 18 bytes")
    out = bytearray(18)
    if mode == "FULL18":
        for i in range(18):
            out[i] = (std[i] + KEY[i]) & 0xFF
    elif mode == "KEEP_00_16":
        out[0:2] = std[0:2]
        for i in range(2, 18):
            out[i] = (std[i] + KEY[i]) & 0xFF
    else:
        raise ValueError(mode)
    return bytes(out)


def inspect_bytes(data: bytes):
    if len(data) < 18:
        raise ValueError("TGA too small")
    raw = data[:18]
    direct = header_fields(raw)
    if plausible(direct):
        return {
            "format": "TGA_STANDARD",
            "protected": False,
            "protection_mode": None,
            "decoded_header": raw,
            "fields": direct,
            "footer_tga20": has_footer(data),
        }

    candidates = []
    for mode in ("FULL18", "KEEP_00_16"):
        dec = decode_candidate(raw, mode)
        f = header_fields(dec)
        if plausible(f):
            candidates.append((mode, dec, f))
    if len(candidates) != 1:
        raise ValueError(
            f"protected TGA header unresolved/ambiguous: candidates={[x[0] for x in candidates]} raw={raw.hex()}"
        )
    mode, dec, fields = candidates[0]
    if not has_footer(data):
        raise ValueError("protected TGA gate requires TGA 2.0 TRUEVISION-XFILE footer")

    # Strong payload-size invariant for uncompressed true-color.
    payload_start = 18 + fields["id_length"]
    payload_end = len(data) - FOOTER_SIZE
    actual_payload = payload_end - payload_start
    expected_payload = None
    if fields["image_type"] == 2:
        expected_payload = fields["width"] * fields["height"] * (fields["pixel_depth"] // 8)
        if actual_payload != expected_payload:
            raise ValueError(
                f"uncompressed payload size mismatch: actual={actual_payload} expected={expected_payload}"
            )

    return {
        "format": f"TGA_PRISTON_HEADER18_{mode}",
        "protected": True,
        "protection_mode": mode,
        "decoded_header": dec,
        "fields": fields,
        "footer_tga20": True,
        "payload_start": payload_start,
        "payload_end": payload_end,
        "payload_bytes": actual_payload,
        "expected_uncompressed_payload": expected_payload,
        "protected_header_hex": raw.hex(),
        "decoded_header_hex": dec.hex(),
    }


def decode_bytes(data: bytes):
    info = inspect_bytes(data)
    if not info["protected"]:
        return data, info
    out = info["decoded_header"] + data[18:]
    check = inspect_bytes(out)
    if check["protected"]:
        raise RuntimeError("decoded output still detected as protected")
    return out, info


def build_bytes(baseline: bytes, edited_standard: bytes):
    binfo = inspect_bytes(baseline)
    if not binfo["protected"]:
        raise ValueError("baseline must be protected TGA")
    einfo = inspect_bytes(edited_standard)
    if einfo["protected"]:
        raise ValueError("edited file must be standard TGA")

    keys = ("id_length", "color_map_type", "image_type", "width", "height", "pixel_depth", "descriptor")
    diffs = {k: (binfo["fields"][k], einfo["fields"][k]) for k in keys if binfo["fields"][k] != einfo["fields"][k]}
    if diffs:
        raise ValueError(f"edited TGA structure differs from baseline: {diffs}")
    if len(edited_standard) != len(baseline):
        raise ValueError(
            f"edited TGA size differs from baseline: {len(edited_standard)} != {len(baseline)}"
        )
    if not has_footer(edited_standard):
        raise ValueError("edited standard TGA must retain TGA 2.0 footer")

    # Preserve proprietary header and footer byte-for-byte; accept only pixel/body edits.
    out = baseline[:18] + edited_standard[18:-FOOTER_SIZE] + baseline[-FOOTER_SIZE:]
    if out[:18] != baseline[:18] or out[-FOOTER_SIZE:] != baseline[-FOOTER_SIZE:]:
        raise RuntimeError("non-payload TGA bytes changed unexpectedly")
    return out, binfo, einfo


def cmd_inspect(a):
    p = Path(a.input).expanduser().resolve()
    data = p.read_bytes(); info = inspect_bytes(data)
    f = info["fields"]
    print(TOOL); print("action=inspect")
    print(f"input={p}"); print(f"sha256={sha(data)}")
    print(f"format={info['format']}")
    print(f"dimensions={f['width']}x{f['height']}")
    print(f"image_type={f['image_type']}"); print(f"pixel_depth={f['pixel_depth']}")
    print(f"descriptor={f['descriptor']}"); print(f"footer_tga20={info['footer_tga20']}")
    if info.get("protected_header_hex"): print(f"protected_header_hex={info['protected_header_hex']}")
    print("client_modified=False"); print("tga_inspect_check=PASS")


def cmd_decode(a):
    p = Path(a.input).expanduser().resolve(); outp = Path(a.out).expanduser().resolve()
    data = p.read_bytes(); out, info = decode_bytes(data)
    outp.parent.mkdir(parents=True, exist_ok=True); outp.write_bytes(out)
    print(TOOL); print("action=decode")
    print(f"input_sha256={sha(data)}"); print(f"output_sha256={sha(out)}")
    print(f"output={outp}"); print(f"format={info['format']}")
    print("client_modified=False"); print("tga_decode_check=PASS")


def cmd_build(a):
    bp=Path(a.baseline).expanduser().resolve(); ep=Path(a.edited).expanduser().resolve(); op=Path(a.out).expanduser().resolve()
    base=bp.read_bytes(); edited=ep.read_bytes(); out,bi,ei=build_bytes(base,edited)
    op.parent.mkdir(parents=True, exist_ok=True); op.write_bytes(out)
    print(TOOL); print("action=build")
    print(f"baseline_sha256={sha(base)}"); print(f"edited_sha256={sha(edited)}"); print(f"output_sha256={sha(out)}")
    print(f"byte_identical_to_baseline={out == base}"); print(f"output={op}")
    print("client_modified=False"); print("tga_build_check=PASS")


def cmd_audit_csv(a):
    p=Path(a.csv).expanduser().resolve(); total=good=bad=0; modes={}; types={}; dims={}; bad_rows=[]
    with p.open(encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            count=int(row["file_count"]); total+=count
            h16=bytes.fromhex(row["head16"])
            # We only have first 16 bytes, so test the two known modes over those fields.
            matches=[]
            for mode in ("FULL18","KEEP_00_16"):
                dec=bytearray(16)
                if mode=="FULL18":
                    for i in range(16): dec[i]=(h16[i]-KEY[i])&0xff
                else:
                    dec[:2]=h16[:2]
                    for i in range(2,16): dec[i]=(h16[i]-KEY[i])&0xff
                image_type=dec[2]; w=int.from_bytes(dec[12:14],"little"); h=int.from_bytes(dec[14:16],"little")
                if dec[0]==0 and dec[1]==0 and image_type in (2,10) and 0<w<=8192 and 0<h<=8192:
                    matches.append((mode,image_type,w,h))
            if len(matches)==1:
                good+=count; mode,it,w,h=matches[0]; modes[mode]=modes.get(mode,0)+count; types[it]=types.get(it,0)+count; dims[f"{w}x{h}"]=dims.get(f"{w}x{h}",0)+count
            else:
                bad+=count; bad_rows.append({"count":count,"head16":row["head16"],"matches":matches})
    report={"tool":TOOL,"input":str(p),"total_files":total,"decoded_plausible":good,"unresolved":bad,"coverage_percent":round(good/total*100,6) if total else 0,"modes":modes,"image_types":types,"dimensions":dict(sorted(dims.items(), key=lambda kv:-kv[1])),"unresolved_rows":bad_rows,"client_modified":False,"tga_signature_audit_check":"PASS"}
    if a.report:
        rp=Path(a.report).expanduser().resolve(); rp.parent.mkdir(parents=True,exist_ok=True); rp.write_text(json.dumps(report,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    print(TOOL); print("action=audit-signature-csv"); print(f"total_files={total}"); print(f"decoded_plausible={good}"); print(f"unresolved={bad}"); print(f"coverage_percent={report['coverage_percent']}"); print(f"modes={modes}"); print(f"image_types={types}"); print("client_modified=False"); print("tga_signature_audit_check=PASS")


def main():
    ap=argparse.ArgumentParser(); sp=ap.add_subparsers(dest="cmd",required=True)
    p=sp.add_parser("inspect"); p.add_argument("input"); p.set_defaults(fn=cmd_inspect)
    p=sp.add_parser("decode"); p.add_argument("input"); p.add_argument("--out",required=True); p.set_defaults(fn=cmd_decode)
    p=sp.add_parser("build"); p.add_argument("--baseline",required=True); p.add_argument("--edited",required=True); p.add_argument("--out",required=True); p.set_defaults(fn=cmd_build)
    p=sp.add_parser("audit-signature-csv"); p.add_argument("csv"); p.add_argument("--report"); p.set_defaults(fn=cmd_audit_csv)
    a=ap.parse_args(); a.fn(a)

if __name__ == "__main__":
    try: main()
    except Exception as exc:
        print(f"tga_core_check=FAIL: {type(exc).__name__}: {exc}")
        raise

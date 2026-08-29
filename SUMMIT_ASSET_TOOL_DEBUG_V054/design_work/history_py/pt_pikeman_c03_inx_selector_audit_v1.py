#!/usr/bin/env python3
"""
PT Pikeman C03 INX Selector Audit V1

Read-only.

Searches every INX in the client and decodes:
  szModelFile      @ 0x000
  szMotionFile     @ 0x040
  szSubModelFile   @ 0x080
  HighModel        @ 0x0C0
  DefaultModel     @ 0x104
  LowModel         @ 0x148

Reports INX files whose model groups select one or more requested NodeName values.

Default requested names are the C03 objects discovered in the real Pikeman SMD:
  TbhC03 TbmC03 TblC03

No file is modified.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import struct


def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def cstr(data, off, n):
    raw = data[off:off+n].split(b"\\0", 1)[0]
    try:
        return raw.decode("ascii").strip()
    except UnicodeDecodeError:
        return ""


def group(data, off):
    if len(data) < off + 68:
        return None
    count = struct.unpack_from("<i", data, off)[0]
    if not 0 <= count <= 4:
        return None
    names = []
    for i in range(4):
        s = cstr(data, off + 4 + i * 16, 16)
        if i < count:
            if not s:
                return None
            names.append(s)
    return names


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--client-root", required=True)
    ap.add_argument("--node", action="append", default=[])
    ap.add_argument("--report")
    args = ap.parse_args()

    wanted = args.node or ["TbhC03", "TbmC03", "TblC03"]
    wanted_lower = {x.lower() for x in wanted}

    root = pathlib.Path(args.client_root).expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(root)

    matches = []
    scanned = 0
    valid_modelinfo = 0

    for p in sorted(root.rglob("*")):
        if not p.is_file() or p.suffix.lower() != ".inx":
            continue
        scanned += 1
        try:
            data = p.read_bytes()
        except OSError:
            continue
        if len(data) < 0x18C:
            continue

        high = group(data, 0x0C0)
        default = group(data, 0x104)
        low = group(data, 0x148)
        if high is None or default is None or low is None:
            continue
        valid_modelinfo += 1

        selected = {
            "high": high,
            "default": default,
            "low": low,
        }
        hits = {}
        for lod, names in selected.items():
            h = [n for n in names if n.lower() in wanted_lower]
            if h:
                hits[lod] = h
        if not hits:
            continue

        model = cstr(data, 0x00, 64)
        motion = cstr(data, 0x40, 64)
        submodel = cstr(data, 0x80, 64)

        matches.append({
            "inx": str(p.relative_to(root)).replace("\\\\", "/"),
            "inx_size": p.stat().st_size,
            "inx_sha256": sha256_file(p),
            "szModelFile": model,
            "szMotionFile": motion,
            "szSubModelFile": submodel,
            "high": high,
            "default": default,
            "low": low,
            "hits": hits,
        })

    result = {
        "tool": "PT_PIKEMAN_C03_INX_SELECTOR_AUDIT_V1",
        "client_root": str(root),
        "wanted_nodes": wanted,
        "inx_scanned": scanned,
        "valid_modelinfo_count": valid_modelinfo,
        "match_count": len(matches),
        "matches": matches,
        "client_modified": False,
        "c03_selector_audit_check": "PASS",
    }

    if args.report:
        rp = pathlib.Path(args.report).expanduser()
        if not rp.is_absolute():
            rp = pathlib.Path.cwd() / rp
        rp = rp.resolve()
        rp.parent.mkdir(parents=True, exist_ok=True)
        rp.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\\n", encoding="utf-8")
    else:
        rp = None

    print("PT_PIKEMAN_C03_INX_SELECTOR_AUDIT_V1")
    print(f"wanted_nodes={wanted}")
    print(f"inx_scanned={scanned}")
    print(f"valid_modelinfo_count={valid_modelinfo}")
    print(f"match_count={len(matches)}")
    for i, row in enumerate(matches, 1):
        print(f"MATCH {i}")
        print(f"  inx={row['inx']}")
        print(f"  szModelFile={row['szModelFile']!r}")
        print(f"  szMotionFile={row['szMotionFile']!r}")
        print(f"  high={row['high']}")
        print(f"  default={row['default']}")
        print(f"  low={row['low']}")
        print(f"  hits={row['hits']}")
    if rp:
        print(f"report={rp}")
    print("client_modified=False")
    print("c03_selector_audit_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"c03_selector_audit_check=FAIL: {type(exc).__name__}: {exc}")
        raise

#!/usr/bin/env python3
"""
PT Pikeman C03 INX Layout Probe V1

Read-only structural probe for:
  C001.inx C002.inx C003.inx C004.inx C005.inx M4Bip.inx

Goals:
- print the leading smMODELINFO paths;
- print the known groups at 0x0C0 / 0x104 / 0x148;
- locate TbhC03/TbmC03/TblC03 exact byte offsets;
- scan the whole INX for any structurally valid _MODELGROUP-like record
  containing the requested nodes;
- show nearby printable strings and hex context;
- compare the C00x family without modifying it.

No semantic label is assigned to an unknown structure merely from its offset.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import struct


DEFAULT_FILES = ["C001.inx", "C002.inx", "C003.inx", "C004.inx", "C005.inx", "M4Bip.inx"]
DEFAULT_NODES = ["TbhC03", "TbmC03", "TblC03"]


def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def cstr(data, off, size):
    if off < 0 or off + size > len(data):
        return ""
    raw = data[off:off+size].split(b"\0", 1)[0]
    try:
        text = raw.decode("ascii")
    except UnicodeDecodeError:
        return ""
    if any(ord(ch) < 32 or ord(ch) > 126 for ch in text):
        return ""
    return text.strip()


def group_at(data, off):
    if off < 0 or off + 68 > len(data):
        return None
    count = struct.unpack_from("<i", data, off)[0]
    if not 0 <= count <= 4:
        return None

    names = []
    raw_slots = []
    for i in range(4):
        slot = data[off+4+i*16:off+4+(i+1)*16]
        # Every active slot must contain a NUL and valid printable ASCII.
        name = cstr(data, off+4+i*16, 16)
        raw_slots.append(name)
        if i < count:
            if not name:
                return None
            # Conservative NodeName-ish syntax.
            if any(ch not in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. " for ch in name):
                return None
            names.append(name)

    # Inactive slots must be empty for the strong candidate classifier.
    inactive_clean = all(not raw_slots[i] for i in range(count, 4))
    return {
        "offset": off,
        "count": count,
        "active": names,
        "slots": raw_slots,
        "inactive_clean": inactive_clean,
    }


def find_all(data, token):
    needle = token.encode("ascii").lower()
    low = data.lower()
    out = []
    start = 0
    while True:
        p = low.find(needle, start)
        if p < 0:
            return out
        out.append(p)
        start = p + 1


def printable_runs(data, start, end, min_len=4):
    start = max(0, start)
    end = min(len(data), end)
    out = []
    i = start
    while i < end:
        if 32 <= data[i] <= 126:
            j = i + 1
            while j < end and 32 <= data[j] <= 126:
                j += 1
            if j - i >= min_len:
                out.append({"offset": i, "text": data[i:j].decode("ascii", "replace")})
            i = j
        else:
            i += 1
    return out


def hex_context(data, pos, radius=48):
    a = max(0, pos-radius)
    b = min(len(data), pos+radius)
    raw = data[a:b]
    return {
        "start": a,
        "end": b,
        "hex": raw.hex(),
        "ascii": "".join(chr(x) if 32 <= x <= 126 else "." for x in raw),
    }


def scan_group_candidates(data, wanted_lower):
    candidates = []
    # Natural ABI alignment expected for the int at group start.
    for off in range(0, len(data)-67, 4):
        g = group_at(data, off)
        if not g or g["count"] == 0:
            continue
        if any(n.lower() in wanted_lower for n in g["active"]):
            candidates.append(g)
    return candidates


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--client-root", required=True)
    ap.add_argument("--file", action="append", default=[])
    ap.add_argument("--node", action="append", default=[])
    ap.add_argument("--report")
    args = ap.parse_args()

    root = pathlib.Path(args.client_root).expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(root)

    names = args.file or DEFAULT_FILES
    nodes = args.node or DEFAULT_NODES
    wanted_lower = {x.lower() for x in nodes}

    rows = []
    for name in names:
        p = root / "char/tmABCD" / name
        if not p.is_file():
            rows.append({"file": name, "exists": False})
            continue

        data = p.read_bytes()
        standard_groups = {
            "high_0x0C0": group_at(data, 0x0C0),
            "default_0x104": group_at(data, 0x104),
            "low_0x148": group_at(data, 0x148),
        }

        occurrences = {}
        for node in nodes:
            hits = []
            for pos in find_all(data, node):
                hits.append({
                    "offset": pos,
                    "offset_hex": f"0x{pos:X}",
                    "mod4": pos % 4,
                    "nearby_strings": printable_runs(data, pos-128, pos+160),
                    "context": hex_context(data, pos),
                })
            occurrences[node] = hits

        candidates = scan_group_candidates(data, wanted_lower)

        row = {
            "file": name,
            "exists": True,
            "path": str(p.relative_to(root)).replace("\\", "/"),
            "size": len(data),
            "sha256": sha256_file(p),
            "leading_fields": {
                "szModelFile_0x000": cstr(data, 0x000, 64),
                "szMotionFile_0x040": cstr(data, 0x040, 64),
                "szSubModelFile_0x080": cstr(data, 0x080, 64),
            },
            "standard_groups": standard_groups,
            "target_occurrences": occurrences,
            "group_like_candidates_with_targets": candidates,
        }
        rows.append(row)

    # Family comparison: first 0x18C contains the three known model groups.
    existing = [r for r in rows if r.get("exists")]
    family_summary = []
    for r in existing:
        family_summary.append({
            "file": r["file"],
            "model": r["leading_fields"]["szModelFile_0x000"],
            "motion": r["leading_fields"]["szMotionFile_0x040"],
            "submodel": r["leading_fields"]["szSubModelFile_0x080"],
            "high": (r["standard_groups"]["high_0x0C0"] or {}).get("active"),
            "default": (r["standard_groups"]["default_0x104"] or {}).get("active"),
            "low": (r["standard_groups"]["low_0x148"] or {}).get("active"),
            "target_offsets": {
                n: [h["offset_hex"] for h in r["target_occurrences"][n]]
                for n in nodes
            },
            "candidate_group_offsets": [
                f"0x{x['offset']:X}" for x in r["group_like_candidates_with_targets"]
            ],
        })

    result = {
        "tool": "PT_PIKEMAN_C03_INX_LAYOUT_PROBE_V1",
        "client_root": str(root),
        "files": names,
        "nodes": nodes,
        "family_summary": family_summary,
        "details": rows,
        "client_modified": False,
        "c03_inx_layout_probe_check": "PASS",
    }

    report = None
    if args.report:
        report = pathlib.Path(args.report).expanduser()
        if not report.is_absolute():
            report = pathlib.Path.cwd() / report
        report = report.resolve()
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    print("PT_PIKEMAN_C03_INX_LAYOUT_PROBE_V1")
    for s in family_summary:
        print(f"FILE {s['file']}")
        print(f"  model={s['model']!r}")
        print(f"  motion={s['motion']!r}")
        print(f"  submodel={s['submodel']!r}")
        print(f"  high={s['high']}")
        print(f"  default={s['default']}")
        print(f"  low={s['low']}")
        print(f"  target_offsets={s['target_offsets']}")
        print(f"  candidate_group_offsets={s['candidate_group_offsets']}")

    # Print candidate group content explicitly.
    for r in existing:
        for g in r["group_like_candidates_with_targets"]:
            print(
                f"GROUP_CANDIDATE file={r['file']} "
                f"offset=0x{g['offset']:X} count={g['count']} "
                f"active={g['active']} inactive_clean={g['inactive_clean']}"
            )

    if report:
        print(f"report={report}")
    print("client_modified=False")
    print("c03_inx_layout_probe_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"c03_inx_layout_probe_check=FAIL: {type(exc).__name__}: {exc}")
        raise

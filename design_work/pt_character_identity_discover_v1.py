#!/usr/bin/env python3
"""
PT Character Identity Discover V1

Read-only scanner for Summit/Priston character model-info caches.

Purpose:
- do not infer class identity from filenames alone;
- find INX files whose smMODELINFO.szMotionFile matches a requested motion stem;
- read the same INX's szModelFile;
- derive physical SMD and SMB targets;
- verify whether those physical assets exist;
- inventory texture-like strings embedded in the candidate model.

Current default:
  --motion-stem m4
because the supplied historical/source material maps m4 -> Pikeman.

No files are modified.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import struct
import sys


TEXTURE_EXTS = (".bmp", ".tga", ".png", ".jpg", ".jpeg", ".dds", ".stg", ".sta")


def sha256_file(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def cstr64(data: bytes, off: int) -> str:
    raw = data[off:off+64].split(b"\0", 1)[0]
    try:
        text = raw.decode("ascii")
    except UnicodeDecodeError:
        return ""
    if any(ord(ch) < 32 or ord(ch) > 126 for ch in text):
        return ""
    return text.strip()


def norm_logical(s: str) -> str:
    return s.replace("\\", "/").strip()


def logical_to_client_path(client_root: pathlib.Path, logical: str, new_ext=None):
    s = norm_logical(logical)
    if not s:
        return None
    p = pathlib.PurePosixPath(s)
    if new_ext:
        p = p.with_suffix(new_ext)
    out = (client_root / p).resolve()
    try:
        out.relative_to(client_root.resolve())
    except ValueError:
        return None
    return out


def extract_ascii_strings(data: bytes, min_len=4):
    # Binary-safe printable ASCII string extraction.
    patt = rb"[\x20-\x7e]{" + str(min_len).encode() + rb",}"
    return [m.group().decode("ascii", "ignore") for m in re.finditer(patt, data)]


def texture_strings_from_model(path: pathlib.Path):
    if not path or not path.is_file():
        return []
    strings = extract_ascii_strings(path.read_bytes(), 4)
    out = []
    seen = set()
    for s in strings:
        low = s.lower()
        if any(low.endswith(ext) for ext in TEXTURE_EXTS):
            n = norm_logical(s)
            key = n.lower()
            if key not in seen:
                seen.add(key)
                out.append(n)
    return out


def basename_stem(logical: str) -> str:
    if not logical:
        return ""
    return pathlib.PurePosixPath(norm_logical(logical)).stem.lower()


def scan(client_root, motion_stem):
    rows = []
    motion_stem = motion_stem.lower()

    for inx in sorted(client_root.rglob("*")):
        if not inx.is_file() or inx.suffix.lower() != ".inx":
            continue
        try:
            head = inx.read_bytes()[:192]
        except OSError:
            continue
        if len(head) < 128:
            continue

        model = cstr64(head, 0x00)
        motion = cstr64(head, 0x40)
        submodel = cstr64(head, 0x80) if len(head) >= 192 else ""

        if basename_stem(motion) != motion_stem:
            continue

        model_smd = logical_to_client_path(client_root, model, ".smd")
        motion_smb = logical_to_client_path(client_root, motion, ".smb")

        row = {
            "inx": str(inx),
            "inx_relative": str(inx.relative_to(client_root)),
            "inx_size": inx.stat().st_size,
            "inx_sha256": sha256_file(inx),
            "szModelFile": model,
            "szMotionFile": motion,
            "szSubModelFile": submodel,
            "derived_model_smd": str(model_smd) if model_smd else None,
            "derived_model_target": (
                str(model_smd.relative_to(client_root)).replace("\\", "/")
                if model_smd else None
            ),
            "model_smd_exists": bool(model_smd and model_smd.is_file()),
            "model_smd_size": (
                model_smd.stat().st_size if model_smd and model_smd.is_file() else None
            ),
            "model_smd_sha256": (
                sha256_file(model_smd) if model_smd and model_smd.is_file() else None
            ),
            "derived_skeleton_smb": str(motion_smb) if motion_smb else None,
            "derived_skeleton_target": (
                str(motion_smb.relative_to(client_root)).replace("\\", "/")
                if motion_smb else None
            ),
            "skeleton_smb_exists": bool(motion_smb and motion_smb.is_file()),
            "skeleton_smb_size": (
                motion_smb.stat().st_size if motion_smb and motion_smb.is_file() else None
            ),
            "skeleton_smb_sha256": (
                sha256_file(motion_smb) if motion_smb and motion_smb.is_file() else None
            ),
            "texture_strings": texture_strings_from_model(model_smd),
        }
        row["status"] = (
            "COMPLETE_PHYSICAL_CHAIN"
            if row["model_smd_exists"] and row["skeleton_smb_exists"]
            else "INCOMPLETE_PHYSICAL_CHAIN"
        )
        rows.append(row)
    return rows


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--client-root", required=True)
    p.add_argument("--motion-stem", default="m4")
    p.add_argument("--report")
    args = p.parse_args()

    root = pathlib.Path(args.client_root).expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(root)

    rows = scan(root, args.motion_stem)

    result = {
        "tool": "PT_CHARACTER_IDENTITY_DISCOVER_V1",
        "client_root": str(root),
        "motion_stem": args.motion_stem,
        "candidate_count": len(rows),
        "complete_count": sum(r["status"] == "COMPLETE_PHYSICAL_CHAIN" for r in rows),
        "candidates": rows,
        "identity_discovery_check": "PASS",
    }

    if args.report:
        rp = pathlib.Path(args.report).expanduser()
        if not rp.is_absolute():
            rp = pathlib.Path.cwd() / rp
        rp = rp.resolve()
        rp.parent.mkdir(parents=True, exist_ok=True)
        rp.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
    else:
        rp = None

    print("PT_CHARACTER_IDENTITY_DISCOVER_V1")
    print(f"client_root={root}")
    print(f"motion_stem={args.motion_stem}")
    print(f"candidate_count={len(rows)}")
    print(f"complete_count={result['complete_count']}")

    for i, row in enumerate(rows, 1):
        print(f"CANDIDATE {i}")
        print(f"  status={row['status']}")
        print(f"  inx={row['inx_relative']}")
        print(f"  szModelFile={row['szModelFile']!r}")
        print(f"  szMotionFile={row['szMotionFile']!r}")
        print(f"  szSubModelFile={row['szSubModelFile']!r}")
        print(f"  model_target={row['derived_model_target']}")
        print(f"  model_exists={row['model_smd_exists']}")
        print(f"  model_size={row['model_smd_size']}")
        print(f"  model_sha256={row['model_smd_sha256']}")
        print(f"  skeleton_target={row['derived_skeleton_target']}")
        print(f"  skeleton_exists={row['skeleton_smb_exists']}")
        print(f"  skeleton_size={row['skeleton_smb_size']}")
        print(f"  skeleton_sha256={row['skeleton_smb_sha256']}")
        print(f"  texture_strings={row['texture_strings']}")

    if rp:
        print(f"report={rp}")
    print("identity_discovery_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"identity_discovery_check=FAIL: {type(exc).__name__}: {exc}")
        raise

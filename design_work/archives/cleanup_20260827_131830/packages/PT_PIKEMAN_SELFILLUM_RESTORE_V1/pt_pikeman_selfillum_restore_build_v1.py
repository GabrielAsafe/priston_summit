#!/usr/bin/env python3
"""
PT Pikeman SelfIllum Restore Build V1

Build-only / no client writes.

Creates a verified no-op build from the immutable registered TmbC01.smd source,
for publishing back through the managed workspace/Restore V4 path.

Strict baseline:
  target = char/tmABCD/TmbC01.smd
  source SHA256 =
    9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c
  source size = 187504

No byte is modified.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import re
import tempfile


SOURCE_SHA = "9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SOURCE_SIZE = 187504
TARGET = "char/tmABCD/TmbC01.smd"


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def norm_rel(s):
    s = str(s).replace("\\", "/")
    s = re.sub(r"/+", "/", s).strip("/")
    p = pathlib.PurePosixPath(s)
    if not s or p.is_absolute() or ".." in p.parts:
        raise ValueError(f"unsafe target: {s!r}")
    return p.as_posix()


def atomic_write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    os.close(fd)
    tmp = pathlib.Path(tmp_name)
    try:
        tmp.write_bytes(data)
        os.replace(tmp, path)
    finally:
        if tmp.exists():
            tmp.unlink()


def atomic_json(path, obj):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    os.close(fd)
    tmp = pathlib.Path(tmp_name)
    try:
        tmp.write_text(
            json.dumps(obj, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        os.replace(tmp, path)
    finally:
        if tmp.exists():
            tmp.unlink()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--source", default="TmbC01.smd")
    ap.add_argument(
        "--out",
        default="pt_build/characters/pikeman_c01_m4/materials/"
                "TmbC01_selfillum_restore_source_v1.smd",
    )
    ap.add_argument(
        "--manifest",
        default="logs/characters/pikeman_c01_m4/materials/"
                "TmbC01_selfillum_restore_source_v1_build_manifest.json",
    )
    ap.add_argument("--roundtrip-target", default=TARGET)
    args = ap.parse_args()

    src = pathlib.Path(args.source).expanduser().resolve()
    if not src.is_file():
        raise FileNotFoundError(src)

    data = src.read_bytes()
    src_sha = sha256_bytes(data)
    src_size = len(data)

    if src_sha != SOURCE_SHA:
        raise ValueError(
            f"registered source SHA mismatch: got={src_sha} expected={SOURCE_SHA}"
        )
    if src_size != SOURCE_SIZE:
        raise ValueError(
            f"registered source size mismatch: got={src_size} expected={SOURCE_SIZE}"
        )
    if not data.startswith(b"SMD Model data Ver 0.62"):
        raise ValueError("unexpected SMD header")

    out = pathlib.Path(args.out).expanduser()
    if not out.is_absolute():
        out = pathlib.Path.cwd() / out
    out = out.resolve()
    atomic_write(out, data)

    out_sha = sha256_file(out)
    if out_sha != SOURCE_SHA:
        raise RuntimeError(
            f"restore artifact SHA mismatch: got={out_sha} expected={SOURCE_SHA}"
        )
    if out.stat().st_size != SOURCE_SIZE:
        raise RuntimeError("restore artifact size mismatch")

    target = norm_rel(args.roundtrip_target)

    manifest = pathlib.Path(args.manifest).expanduser()
    if not manifest.is_absolute():
        manifest = pathlib.Path.cwd() / manifest
    manifest = manifest.resolve()

    obj = {
        "tool": "PT_PIKEMAN_SELFILLUM_RESTORE_BUILD_V1",
        "role": "model_material_selfillum_restore",
        "roundtrip_target": target,
        "source": str(src),
        "source_sha256": src_sha,
        "source_size": src_size,
        "artifact": str(out),
        "artifact_sha256": out_sha,
        "artifact_size": out.stat().st_size,
        "changed_byte_count": 0,
        "noop_build": True,
        "byte_identical_to_registered_source": True,
        "runtime_installed": False,
        "build_check": "PASS",
    }
    atomic_json(manifest, obj)

    print("PT_PIKEMAN_SELFILLUM_RESTORE_BUILD_V1")
    print(f"source={src}")
    print(f"source_sha256={src_sha}")
    print(f"source_size={src_size}")
    print(f"artifact={out}")
    print(f"artifact_sha256={out_sha}")
    print(f"artifact_size={out.stat().st_size}")
    print("changed_byte_count=0")
    print("noop_build=True")
    print("byte_identical_to_registered_source=True")
    print(f"roundtrip_target={target}")
    print(f"manifest={manifest}")
    print("runtime_installed=False")
    print("build_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"build_check=FAIL: {type(exc).__name__}: {exc}")
        raise

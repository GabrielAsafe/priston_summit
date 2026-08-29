#!/usr/bin/env python3
"""
PT Pikeman SMD SelfIllum Build V1

Build-only / no client writes.

Source-proved facts used:
- smMATERIAL size = 320 bytes
- SelfIllum is float at offset 148 in smMATERIAL
- TmbC01.smd registered source SHA:
  9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c
- Material 0 in the real TmbC01.smd:
  texture = char\\tmABCD\\TmbC03.bmp
  BlendType = 1 (SMMAT_BLEND_ALPHA)
  Shade = 1
  TwoSide = 0
  SelfIllum = 0.0f

The runtime source audit proves:
  SelfIllum != 0
  -> renderer uses SetD3DRendBuff2
  -> RGB is forced white while alpha is preserved.

The tool:
- parses the material region structurally;
- verifies the exact source hash and target material identity;
- changes only material[0].SelfIllum from 0.0f to 1.0f;
- validates all bytes outside that 4-byte field are identical;
- reparses the candidate;
- emits a build manifest compatible with
  summit_workspace_asset_bridge_v1_2.py publish-file.

No workspace or client file is modified.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import re
import struct
import tempfile


HEADER_SIZE = 556
MAT_GROUP_SIZE = 88
MATERIAL_SIZE = 320
SELFILLUM_OFFSET = 148

PIKE_SOURCE_SHA256 = "9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
PIKE_TARGET = "char/tmABCD/TmbC01.smd"
EXPECTED_TEXTURE = "char/tmABCD/TmbC03.bmp"


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def f32(data, off):
    return struct.unpack_from("<f", data, off)[0]


def cstr(data, off, n):
    raw = data[off:off+n].split(b"\0", 1)[0]
    return raw.decode("latin1", "replace")


def norm_rel(s):
    s = s.replace("\\", "/")
    s = re.sub(r"/+", "/", s).strip("/")
    return pathlib.PurePosixPath(s).as_posix()


def split_strings(block):
    parts = block.split(b"\0")
    if parts and parts[-1] == b"":
        parts.pop()
    return [p.decode("latin1", "replace") for p in parts]


def parse_material_layout(data: bytes):
    if len(data) < HEADER_SIZE:
        raise ValueError("file too small for SMD header")
    header = cstr(data, 0, 24)
    if not header.startswith("SMD Model data Ver 0.62"):
        raise ValueError(f"unexpected SMD header: {header!r}")

    mat_count = i32(data, 28)
    mat_file = i32(data, 32)
    first_obj = i32(data, 36)

    if mat_count <= 0:
        raise ValueError(f"expected materials, MatCounter={mat_count}")
    if not HEADER_SIZE <= mat_file < first_obj <= len(data):
        raise ValueError("invalid material region bounds")

    if mat_file + MAT_GROUP_SIZE > first_obj:
        raise ValueError("material group header overruns region")
    group = data[mat_file:mat_file+MAT_GROUP_SIZE]
    group_count = i32(group, 8)
    if group_count != mat_count:
        raise ValueError(
            f"MaterialCount mismatch group={group_count} header={mat_count}"
        )

    cursor = mat_file + MAT_GROUP_SIZE
    mats = []

    for mat_id in range(mat_count):
        start = cursor
        end = start + MATERIAL_SIZE
        if end > first_obj:
            raise ValueError(f"material {mat_id} fixed block overruns region")

        raw = data[start:end]
        in_use = u32(raw, 0)
        tc = u32(raw, 4)
        ac = u32(raw, 304)
        if tc > 8:
            raise ValueError(f"material {mat_id}: TextureCounter={tc}")
        if ac > 32:
            raise ValueError(f"material {mat_id}: AnimTexCounter={ac}")

        mat = {
            "id": mat_id,
            "block_start": start,
            "block_end": end,
            "InUse": in_use,
            "TextureCounter": tc,
            "BlendType": u32(raw, 116),
            "Shade": u32(raw, 120),
            "TwoSide": u32(raw, 124),
            "SelfIllum": f32(raw, SELFILLUM_OFFSET),
            "SelfIllum_raw_hex": raw[SELFILLUM_OFFSET:SELFILLUM_OFFSET+4].hex(),
            "AnimTexCounter": ac,
            "textures": [],
            "anim_textures": [],
            "StrLen": None,
        }
        cursor = end

        if in_use != 0:
            if cursor + 4 > first_obj:
                raise ValueError(f"material {mat_id}: missing StrLen")
            strlen = i32(data, cursor)
            cursor += 4
            if strlen < 0 or cursor + strlen > first_obj:
                raise ValueError(f"material {mat_id}: bad StrLen={strlen}")
            block = data[cursor:cursor+strlen]
            cursor += strlen

            strings = split_strings(block)
            expected = 2 * (tc + ac)
            if len(strings) != expected:
                raise ValueError(
                    f"material {mat_id}: string count={len(strings)}, "
                    f"expected={expected}"
                )
            mat["StrLen"] = strlen
            k = 0
            for _ in range(tc):
                mat["textures"].append({
                    "Name": strings[k],
                    "NameA": strings[k+1],
                })
                k += 2
            for _ in range(ac):
                mat["anim_textures"].append({
                    "Name": strings[k],
                    "NameA": strings[k+1],
                })
                k += 2

        mats.append(mat)

    if cursor != first_obj:
        raise ValueError(
            f"material region did not close: cursor={cursor}, first_obj={first_obj}"
        )

    return {
        "header": header,
        "MatCounter": mat_count,
        "MatFilePoint": mat_file,
        "First_ObjInfoPoint": first_obj,
        "materials": mats,
        "material_region_end_check": "PASS",
    }


def semantic_snapshot(layout):
    out = []
    for m in layout["materials"]:
        out.append({
            k: m[k]
            for k in (
                "id", "InUse", "TextureCounter", "BlendType", "Shade",
                "TwoSide", "SelfIllum", "AnimTexCounter", "textures",
                "anim_textures", "StrLen",
            )
        })
    return out


def atomic_write_bytes(path: pathlib.Path, data: bytes):
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


def atomic_write_json(path: pathlib.Path, obj):
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
    ap.add_argument("--smd", default="TmbC01.smd")
    ap.add_argument(
        "--out",
        default="pt_build/characters/pikeman_c01_m4/materials/"
                "TmbC01_material0_selfillum_runtime_gate_v1.smd",
    )
    ap.add_argument(
        "--manifest",
        default="logs/characters/pikeman_c01_m4/materials/"
                "TmbC01_material0_selfillum_runtime_gate_v1_build_manifest.json",
    )
    ap.add_argument("--material-id", type=int, default=0)
    ap.add_argument("--self-illum", type=float, default=1.0)
    ap.add_argument("--expected-source-sha256", default=PIKE_SOURCE_SHA256)
    ap.add_argument("--expected-texture", default=EXPECTED_TEXTURE)
    ap.add_argument("--roundtrip-target", default=PIKE_TARGET)
    args = ap.parse_args()

    src = pathlib.Path(args.smd).expanduser().resolve()
    if not src.is_file():
        raise FileNotFoundError(src)

    src_data = src.read_bytes()
    src_sha = sha256_bytes(src_data)
    if src_sha.lower() != args.expected_source_sha256.lower():
        raise ValueError(
            "source SHA mismatch; refusing material patch: "
            f"got={src_sha} expected={args.expected_source_sha256}"
        )

    before = parse_material_layout(src_data)
    if not 0 <= args.material_id < len(before["materials"]):
        raise ValueError("material id out of range")
    mat = before["materials"][args.material_id]

    if mat["InUse"] == 0:
        raise ValueError("target material is not InUse")
    if mat["TextureCounter"] != 1:
        raise ValueError(
            f"target material TextureCounter={mat['TextureCounter']} expected=1"
        )
    got_tex = norm_rel(mat["textures"][0]["Name"])
    exp_tex = norm_rel(args.expected_texture)
    if got_tex.lower() != exp_tex.lower():
        raise ValueError(
            f"target material texture mismatch: got={got_tex!r} expected={exp_tex!r}"
        )

    # Keep the controlled test pinned to the audited baseline.
    if mat["BlendType"] != 1:
        raise ValueError(f"BlendType changed from audited baseline: {mat['BlendType']}")
    if mat["Shade"] != 1:
        raise ValueError(f"Shade changed from audited baseline: {mat['Shade']}")
    if mat["TwoSide"] != 0:
        raise ValueError(f"TwoSide changed from audited baseline: {mat['TwoSide']}")
    if mat["SelfIllum"] != 0.0:
        raise ValueError(
            f"SelfIllum source is not 0.0f: {mat['SelfIllum']}"
        )

    if args.self_illum == 0.0:
        raise ValueError("runtime gate value must be nonzero")

    target_off = mat["block_start"] + SELFILLUM_OFFSET
    target_end = target_off + 4
    old_field = src_data[target_off:target_end]
    new_field = struct.pack("<f", float(args.self_illum))

    candidate = bytearray(src_data)
    candidate[target_off:target_end] = new_field
    candidate = bytes(candidate)

    changed_offsets = [
        i for i, (a, b) in enumerate(zip(src_data, candidate)) if a != b
    ]
    outside = [
        i for i in changed_offsets if not (target_off <= i < target_end)
    ]
    if outside:
        raise RuntimeError(f"bytes changed outside SelfIllum field: {outside[:20]}")
    if not changed_offsets:
        raise RuntimeError("candidate is byte-identical; expected SelfIllum change")

    after = parse_material_layout(candidate)
    after_mat = after["materials"][args.material_id]

    if abs(after_mat["SelfIllum"] - float(args.self_illum)) > 1e-7:
        raise RuntimeError("reparsed SelfIllum differs from requested value")

    # All structural/semantic material fields except the chosen SelfIllum must match.
    b_snap = semantic_snapshot(before)
    a_snap = semantic_snapshot(after)
    for mid, (b, a) in enumerate(zip(b_snap, a_snap)):
        for key in b:
            if mid == args.material_id and key == "SelfIllum":
                continue
            if b[key] != a[key]:
                raise RuntimeError(
                    f"unexpected semantic change material={mid} field={key}: "
                    f"{b[key]!r} -> {a[key]!r}"
                )

    out = pathlib.Path(args.out).expanduser()
    if not out.is_absolute():
        out = pathlib.Path.cwd() / out
    out = out.resolve()
    atomic_write_bytes(out, candidate)

    if out.stat().st_size != src.stat().st_size:
        raise RuntimeError("candidate size changed")
    out_sha = sha256_file(out)
    if out_sha != sha256_bytes(candidate):
        raise RuntimeError("candidate SHA verification failed")

    manifest = pathlib.Path(args.manifest).expanduser()
    if not manifest.is_absolute():
        manifest = pathlib.Path.cwd() / manifest
    manifest = manifest.resolve()

    obj = {
        "tool": "PT_PIKEMAN_SMD_SELFILLUM_BUILD_V1",
        "role": "model_material_selfillum_runtime_gate",
        "roundtrip_target": norm_rel(args.roundtrip_target),
        "source": str(src),
        "source_sha256": src_sha,
        "source_size": src.stat().st_size,
        "artifact": str(out),
        "artifact_sha256": out_sha,
        "artifact_size": out.stat().st_size,
        "material_id": args.material_id,
        "material_texture": got_tex,
        "field": "SelfIllum",
        "field_type": "float32_le",
        "field_offset_within_smMATERIAL": SELFILLUM_OFFSET,
        "absolute_field_offset": target_off,
        "old_field_hex": old_field.hex(),
        "old_value": mat["SelfIllum"],
        "new_field_hex": new_field.hex(),
        "new_value": after_mat["SelfIllum"],
        "target_field_size": 4,
        "changed_byte_count": len(changed_offsets),
        "changed_offsets": changed_offsets,
        "outside_target_field_changed_byte_count": len(outside),
        "file_size_preserved": out.stat().st_size == src.stat().st_size,
        "material_region_end_check_before": before["material_region_end_check"],
        "material_region_end_check_after": after["material_region_end_check"],
        "semantic_changes_except_target": 0,
        "runtime_semantics": {
            "source_proved_condition": "SelfIllum != 0",
            "source_proved_renderer_path": "SetD3DRendBuff2",
            "source_proved_visual_effect": "vertex RGB forced white; alpha preserved",
        },
        "runtime_installed": False,
        "build_check": "PASS",
    }
    atomic_write_json(manifest, obj)

    print("PT_PIKEMAN_SMD_SELFILLUM_BUILD_V1")
    print(f"source={src}")
    print(f"source_sha256={src_sha}")
    print(f"source_size={src.stat().st_size}")
    print(f"material_id={args.material_id}")
    print(f"material_texture={got_tex}")
    print(f"field=SelfIllum")
    print(f"field_type=float32_le")
    print(f"absolute_field_offset={target_off}")
    print(f"old_value={mat['SelfIllum']}")
    print(f"new_value={after_mat['SelfIllum']}")
    print(f"old_field_hex={old_field.hex()}")
    print(f"new_field_hex={new_field.hex()}")
    print(f"target_field_size=4")
    print(f"changed_byte_count={len(changed_offsets)}")
    print(f"changed_offsets={changed_offsets}")
    print(f"outside_target_field_changed_byte_count={len(outside)}")
    print(f"file_size_preserved=True")
    print(f"material_region_end_check_before=PASS")
    print(f"material_region_end_check_after=PASS")
    print(f"semantic_changes_except_target=0")
    print(f"artifact={out}")
    print(f"artifact_sha256={out_sha}")
    print(f"artifact_size={out.stat().st_size}")
    print(f"manifest={manifest}")
    print("runtime_installed=False")
    print("build_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"build_check=FAIL: {type(exc).__name__}: {exc}")
        raise

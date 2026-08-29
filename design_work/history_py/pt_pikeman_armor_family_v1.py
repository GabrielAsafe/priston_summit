#!/usr/bin/env python3
"""
PT Pikeman Armor Family V1

Read-only / local-artifact bootstrap for C001..C005.

For each config:
- reads smMODELINFO leading fields + High/Default/Low groups;
- resolves the physical SMD from szModelFile;
- verifies m4.smb exists;
- runs the proven V3 importer with the config itself as --model-info;
- verifies every selected LOD node exists in the imported SMD;
- inventories material/texture lines from importer output;
- measures material usage on the DEFAULT LOD with a Blender read-only probe;
- produces one Blender preview per config.

No client file is modified.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import re
import shlex
import shutil
import struct
import subprocess


DEFAULT_CONFIGS = ["C001.inx", "C002.inx", "C003.inx", "C004.inx", "C005.inx"]
GROUPS = [("high", 0x0C0), ("default", 0x104), ("low", 0x148)]

OBJ_RE = re.compile(r"^OBJECT\s+(\S+):\s+V=(\d+)\s+F=(\d+)\s+UV=(\d+)")
MAT_RE = re.compile(r"^MATERIAL\s+(\d+):\s+tex='([^']*)'.*decode=([^\s]+)")


def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def cstr(data, off, n):
    raw = data[off:off+n].split(b"\0", 1)[0]
    return raw.decode("ascii", "strict").strip()


def group_at(data, off):
    if off + 68 > len(data):
        raise ValueError(f"INX too small for group at 0x{off:X}")
    count = struct.unpack_from("<i", data, off)[0]
    if not 0 <= count <= 4:
        raise ValueError(f"invalid ModelNameCnt={count} at 0x{off:X}")
    out = []
    for i in range(count):
        name = cstr(data, off + 4 + i*16, 16)
        if not name:
            raise ValueError(f"empty active NodeName at 0x{off:X}")
        out.append(name)
    return out


def normalize_rel(s):
    s = str(s).replace("\\", "/")
    s = re.sub(r"/+", "/", s).strip("/")
    p = pathlib.PurePosixPath(s)
    if not s or p.is_absolute() or ".." in p.parts:
        raise ValueError(f"unsafe path: {s!r}")
    return p.as_posix()


def derive_smd(client_root, logical_model):
    rel = pathlib.PurePosixPath(normalize_rel(logical_model)).with_suffix(".smd")
    path = (client_root / rel).resolve()
    path.relative_to(client_root.resolve())
    return path, rel.as_posix()


def blender_command():
    raw = os.environ.get("PT_BLENDER_COMMAND")
    if raw:
        return shlex.split(raw)
    return ["flatpak", "run", "--filesystem=home", "org.blender.Blender"]


def parse_import(text):
    objects = {}
    materials = {}
    for line in text.splitlines():
        s = line.strip()
        m = OBJ_RE.match(s)
        if m:
            objects[m.group(1)] = {
                "vertices": int(m.group(2)),
                "faces": int(m.group(3)),
                "uv_faces": int(m.group(4)),
            }
            continue
        m = MAT_RE.match(s)
        if m:
            materials[int(m.group(1))] = {
                "texture": normalize_rel(m.group(2)),
                "decode": m.group(3),
            }
    if not objects:
        raise RuntimeError("importer emitted no OBJECT inventory")
    return objects, materials


def copy_verified(src, dst):
    if dst.exists():
        if sha256_file(dst) != sha256_file(src):
            raise RuntimeError(f"local baseline drift: {dst}")
        return "EXISTS_VERIFIED"
    dst.parent.mkdir(parents=True, exist_ok=True)
    tmp = dst.with_name(dst.name + ".tmp")
    shutil.copy2(src, tmp)
    if sha256_file(tmp) != sha256_file(src):
        tmp.unlink(missing_ok=True)
        raise RuntimeError(f"copy verification failed: {src}")
    os.replace(tmp, dst)
    return "COPIED_VERIFIED"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--client-root", required=True)
    ap.add_argument("--config", action="append", default=[])
    ap.add_argument("--overwrite", action="store_true")
    ap.add_argument(
        "--report",
        default="logs/characters/pikeman_c01_m4/armor_family/armor_family_v1.json",
    )
    args = ap.parse_args()

    root = pathlib.Path(args.root).expanduser().resolve()
    client = pathlib.Path(args.client_root).expanduser().resolve()
    if not client.is_dir():
        raise FileNotFoundError(client)

    importer = root / "history_py/pt_smd_blender_import_v3_static_armature.py"
    if not importer.is_file():
        raise FileNotFoundError(importer)

    usage_probe = root / "pt_blender_selected_material_usage_probe_v1.py"
    if not usage_probe.is_file():
        raise FileNotFoundError(usage_probe)

    skeleton_client = client / "char/tmABCD/m4.smb"
    if not skeleton_client.is_file():
        raise FileNotFoundError(skeleton_client)
    skeleton_local = root / "m4.smb"
    skeleton_copy = copy_verified(skeleton_client, skeleton_local)

    configs = args.config or DEFAULT_CONFIGS
    out_rows = []
    unique_models = {}

    out_blend_dir = root / "blender/pikeman_c01_m4/armor_family"
    log_dir = root / "logs/characters/pikeman_c01_m4/armor_family"
    out_blend_dir.mkdir(parents=True, exist_ok=True)
    log_dir.mkdir(parents=True, exist_ok=True)

    for cfg_name in configs:
        cfg_client = client / "char/tmABCD" / cfg_name
        if not cfg_client.is_file():
            raise FileNotFoundError(cfg_client)
        data = cfg_client.read_bytes()

        model_logical = cstr(data, 0x00, 64)
        motion_logical = cstr(data, 0x40, 64)
        submodel_logical = cstr(data, 0x80, 64)
        lod = {name: group_at(data, off) for name, off in GROUPS}

        smd_client, smd_target = derive_smd(client, model_logical)
        if not smd_client.is_file():
            raise FileNotFoundError(smd_client)

        local_smd = root / smd_client.name
        smd_copy = copy_verified(smd_client, local_smd)
        unique_models[smd_target.lower()] = {
            "target": smd_target,
            "local": str(local_smd),
            "size": local_smd.stat().st_size,
            "sha256": sha256_file(local_smd),
            "copy_status": smd_copy,
        }

        local_cfg = root / cfg_name
        cfg_copy = copy_verified(cfg_client, local_cfg)

        stem = pathlib.Path(cfg_name).stem
        blend = out_blend_dir / f"{stem}_pikeman_armor_preview.blend"
        log = log_dir / f"{stem}_import.log"
        usage_json = log_dir / f"{stem}_selected_material_usage.json"

        if not args.overwrite:
            for artifact in (blend, log, usage_json):
                if artifact.exists():
                    raise FileExistsError(
                        f"artifact exists for {cfg_name}; use --overwrite intentionally: {artifact}"
                    )

        cmd = blender_command() + [
            "--background",
            "--python-exit-code", "1",
            "--python", str(importer), "--",
            "--smd", str(local_smd),
            "--skeleton", str(skeleton_local),
            "--model-info", str(local_cfg),
            "--lod", "default",
            "--client-root", str(client),
            "--out", str(blend),
            "--no-pack",
        ]

        print(f"=== {cfg_name} ===")
        print("COMMAND=" + " ".join(shlex.quote(x) for x in cmd))
        cp = subprocess.run(cmd, text=True, capture_output=True)
        combined = (cp.stdout or "") + (cp.stderr or "")
        log.write_text(combined, encoding="utf-8")
        print(combined, end="" if combined.endswith("\n") else "\n")
        if cp.returncode != 0:
            raise subprocess.CalledProcessError(cp.returncode, cmd)
        if "static_armature_import_check=PASS" not in combined:
            raise RuntimeError(f"{cfg_name}: V3 importer did not PASS")

        objects, materials = parse_import(combined)

        selected_all = []
        for names in lod.values():
            for n in names:
                if n not in selected_all:
                    selected_all.append(n)
        missing = [n for n in selected_all if n not in objects]
        if missing:
            raise RuntimeError(
                f"{cfg_name}: selected nodes missing from {smd_target}: {missing}"
            )

        cmd2 = blender_command() + [
            "--background", str(blend),
            "--python-exit-code", "1",
            "--python", str(usage_probe), "--",
            json.dumps(lod["default"]),
            str(usage_json),
        ]
        print("COMMAND=" + " ".join(shlex.quote(x) for x in cmd2))
        subprocess.run(cmd2, check=True)

        usage_data = json.loads(usage_json.read_text(encoding="utf-8"))
        usage = usage_data["material_face_counts"]

        active_textures = []
        for k, face_count in sorted(usage.items(), key=lambda kv: int(kv[0])):
            idx = int(k)
            mat = materials.get(idx)
            active_textures.append({
                "material_id": idx,
                "face_count_default_lod": int(face_count),
                "texture": mat["texture"] if mat else None,
                "decode": mat["decode"] if mat else None,
            })

        out_rows.append({
            "config": cfg_name,
            "config_target": f"char/tmABCD/{cfg_name}",
            "config_sha256": sha256_file(local_cfg),
            "config_size": local_cfg.stat().st_size,
            "config_copy_status": cfg_copy,
            "szModelFile": model_logical,
            "szMotionFile": motion_logical,
            "szSubModelFile": submodel_logical,
            "model_smd_target": smd_target,
            "model_smd_sha256": sha256_file(local_smd),
            "lod_groups": lod,
            "selected_node_count": len(selected_all),
            "selected_nodes_all_lods": selected_all,
            "default_lod_active_textures": active_textures,
            "blend_preview": str(blend),
            "import_log": str(log),
            "import_check": "PASS",
        })

    result = {
        "tool": "PT_PIKEMAN_ARMOR_FAMILY_V1",
        "profile_id": "pikeman_c01_m4",
        "family": "C001..C005",
        "semantics": {
            "class": "Pikeman",
            "configuration_kind": "equipped_armor_model_info",
            "exact_required_level_per_config": "UNPROVEN",
            "source_rule": "SetPlayerArmor selects szArmorNumFile[class][visual_index]",
        },
        "skeleton": {
            "target": "char/tmABCD/m4.smb",
            "sha256": sha256_file(skeleton_local),
            "size": skeleton_local.stat().st_size,
            "copy_status": skeleton_copy,
        },
        "unique_models": list(unique_models.values()),
        "configs": out_rows,
        "client_modified": False,
        "armor_family_check": "PASS",
    }

    report = pathlib.Path(args.report).expanduser()
    if not report.is_absolute():
        report = root / report
    report = report.resolve()
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print("PT_PIKEMAN_ARMOR_FAMILY_V1")
    print(f"config_count={len(out_rows)}")
    print(f"unique_model_count={len(unique_models)}")
    for row in out_rows:
        print(f"CONFIG {row['config']}")
        print(f"  model={row['model_smd_target']}")
        print(f"  high={row['lod_groups']['high']}")
        print(f"  default={row['lod_groups']['default']}")
        print(f"  low={row['lod_groups']['low']}")
        print(f"  default_lod_active_textures={row['default_lod_active_textures']}")
        print(f"  blend={row['blend_preview']}")
    print(f"report={report}")
    print("exact_required_level_per_config=UNPROVEN")
    print("client_modified=False")
    print("armor_family_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"armor_family_check=FAIL: {type(exc).__name__}: {exc}")
        raise

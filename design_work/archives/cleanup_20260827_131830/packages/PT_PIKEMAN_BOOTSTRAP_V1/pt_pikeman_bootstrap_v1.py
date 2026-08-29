#!/usr/bin/env python3
"""
PT Pikeman Bootstrap V1

Builds a real Pikeman Character Asset profile from the current client evidence:

  M4Bip.inx
    szModelFile  -> TmbC01.ASE -> TmbC01.smd
    szMotionFile -> m4.ase     -> m4.smb

No client file is modified.

Workflow:
1. verify physical chain and hashes;
2. decode INX High/Default/Low model groups directly;
3. make verified local baseline copies in design_work root;
4. run the existing V3 importer once (HIGH) to inventory all SMD objects/materials;
5. parse importer output for object counts and texture/material-slot mapping;
6. generate a Pikeman profile with geometry status explicitly UNPROVEN.

The generated Blender file is an inspection artifact only.
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
import sys
from collections import defaultdict


def sha256_file(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def safe_copy_verified(src: pathlib.Path, dst: pathlib.Path):
    src = src.resolve()
    dst = dst.resolve()
    if dst.exists():
        if sha256_file(dst) != sha256_file(src):
            raise RuntimeError(
                f"local baseline exists with different SHA; refusing overwrite: {dst}"
            )
        return "EXISTS_VERIFIED"
    dst.parent.mkdir(parents=True, exist_ok=True)
    tmp = dst.with_name(dst.name + ".copy.tmp")
    shutil.copy2(src, tmp)
    if sha256_file(tmp) != sha256_file(src) or tmp.stat().st_size != src.stat().st_size:
        tmp.unlink(missing_ok=True)
        raise RuntimeError(f"copy verification failed: {src} -> {dst}")
    os.replace(tmp, dst)
    return "COPIED_VERIFIED"


def cstr(data: bytes, off: int, n: int) -> str:
    raw = data[off:off+n].split(b"\0", 1)[0]
    return raw.decode("ascii", "strict").strip()


def read_group(data: bytes, off: int):
    if off + 68 > len(data):
        raise ValueError(f"INX too small for _MODELGROUP at 0x{off:X}")
    count = struct.unpack_from("<i", data, off)[0]
    if not 0 <= count <= 4:
        raise ValueError(f"invalid ModelNameCnt={count} at 0x{off:X}")
    names = []
    for i in range(4):
        name = cstr(data, off + 4 + i * 16, 16)
        if i < count:
            if not name:
                raise ValueError(f"empty model name inside active group at 0x{off:X}")
            names.append(name)
    return names


def normalize_rel(s: str) -> str:
    return s.replace("\\", "/").strip("/")


def logical_to_physical(client_root: pathlib.Path, logical: str, ext: str):
    rel = pathlib.PurePosixPath(normalize_rel(logical)).with_suffix(ext)
    p = (client_root / rel).resolve()
    p.relative_to(client_root.resolve())
    return p, str(rel).replace("\\", "/")


def blender_command():
    raw = os.environ.get("PT_BLENDER_COMMAND")
    if raw:
        return shlex.split(raw)
    return ["flatpak", "run", "--filesystem=home", "org.blender.Blender"]


MAT_RE = re.compile(r"^MATERIAL\s+(\d+):\s+tex='([^']*)'")
OBJ_RE = re.compile(r"^OBJECT\s+(\S+):\s+V=(\d+)\s+F=(\d+)\s+UV=(\d+)")


def parse_import_log(text: str):
    materials = {}
    objects = {}
    for line in text.splitlines():
        m = MAT_RE.match(line.strip())
        if m:
            materials[int(m.group(1))] = normalize_rel(m.group(2))
            continue
        m = OBJ_RE.match(line.strip())
        if m:
            name = m.group(1)
            objects[name] = {
                "vertices": int(m.group(2)),
                "faces": int(m.group(3)),
                "texlinks": int(m.group(4)),
            }

    if not objects:
        raise RuntimeError("no OBJECT lines found in importer log")
    if not materials:
        raise RuntimeError("no MATERIAL lines found in importer log")

    return materials, objects


def material_usage_probe_script(path: pathlib.Path):
    code = r"""
import bpy, json, pathlib, sys

argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
out = pathlib.Path(argv[0]).expanduser().resolve()

usage = {}
for obj in bpy.data.objects:
    if obj.type != "MESH":
        continue
    counts = {}
    for poly in obj.data.polygons:
        idx = int(poly.material_index)
        counts[str(idx)] = counts.get(str(idx), 0) + 1
    usage[obj.name] = counts

out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(usage, indent=2), encoding="utf-8")
print("PT_MATERIAL_USAGE_BOOTSTRAP")
print(f"objects={len(usage)}")
print(f"report={out}")
print("material_usage_bootstrap_check=PASS")
"""
    path.write_text(code, encoding="utf-8")


def roles_from_materials(materials, usage):
    total_by_slot = defaultdict(int)
    for counts in usage.values():
        for k, v in counts.items():
            total_by_slot[int(k)] += int(v)

    tex_slots = defaultdict(list)
    for slot, tex in sorted(materials.items()):
        if not tex:
            continue
        tex_slots[tex].append(slot)

    rows = []
    for tex, slots in sorted(tex_slots.items(), key=lambda kv: kv[0].lower()):
        basename = pathlib.PurePosixPath(tex).name
        lower = basename.lower()
        if lower == "tmbc01.bmp":
            role = "texture"
            representation = "Diffuse/Character Texture"
        elif lower == "tmbc03.bmp":
            role = "texture_variant"
            representation = "Variant Character Texture"
        else:
            role = "texture_" + re.sub(r"[^a-z0-9]+", "_", pathlib.Path(lower).stem).strip("_")
            representation = "Character Texture"

        rows.append({
            "role": role,
            "target": tex,
            "material_slots": slots,
            "used_face_count": sum(total_by_slot.get(s, 0) for s in slots),
            "representation": representation,
        })
    return rows


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--root", default=".")
    p.add_argument("--client-root", required=True)
    p.add_argument("--inx", default="char/tmABCD/M4Bip.inx")
    p.add_argument("--profile-out", default="profiles/pt_character_profile_pikeman_c01_m4_v1.json")
    p.add_argument("--blend-out", default="blender/TmbC01_pikeman_bootstrap_v1.blend")
    p.add_argument("--overwrite-artifacts", action="store_true")
    args = p.parse_args()

    root = pathlib.Path(args.root).expanduser().resolve()
    client_root = pathlib.Path(args.client_root).expanduser().resolve()
    if not client_root.is_dir():
        raise FileNotFoundError(client_root)

    inx = (client_root / pathlib.PurePosixPath(normalize_rel(args.inx))).resolve()
    inx.relative_to(client_root)
    if not inx.is_file():
        raise FileNotFoundError(inx)

    raw = inx.read_bytes()
    if len(raw) < 0x18C:
        raise ValueError("INX too small for smMODELINFO model groups")

    model_logical = cstr(raw, 0x00, 64)
    motion_logical = cstr(raw, 0x40, 64)
    submodel_logical = cstr(raw, 0x80, 64)

    if pathlib.PurePosixPath(normalize_rel(motion_logical)).stem.lower() != "m4":
        raise ValueError(
            f"M4Bip identity gate expected motion stem m4, got {motion_logical!r}"
        )

    model_smd_client, model_target = logical_to_physical(
        client_root, model_logical, ".smd"
    )
    skeleton_client, skeleton_target = logical_to_physical(
        client_root, motion_logical, ".smb"
    )

    if not model_smd_client.is_file():
        raise FileNotFoundError(model_smd_client)
    if not skeleton_client.is_file():
        raise FileNotFoundError(skeleton_client)

    lod_groups = {
        "high": read_group(raw, 0x0C0),
        "default": read_group(raw, 0x104),
        "low": read_group(raw, 0x148),
    }
    all_lod_names = []
    for lod in ("high", "default", "low"):
        for name in lod_groups[lod]:
            if name not in all_lod_names:
                all_lod_names.append(name)

    local_model = root / model_smd_client.name
    local_skeleton = root / skeleton_client.name
    local_inx = root / inx.name

    copy_status = {
        str(local_model): safe_copy_verified(model_smd_client, local_model),
        str(local_skeleton): safe_copy_verified(skeleton_client, local_skeleton),
        str(local_inx): safe_copy_verified(inx, local_inx),
    }

    importer = root / "history_py/pt_smd_blender_import_v3_static_armature.py"
    if not importer.is_file():
        raise FileNotFoundError(importer)

    blend_out = (root / args.blend_out).resolve()
    log_dir = root / "logs/characters/pikeman_c01_m4/bootstrap"
    log_dir.mkdir(parents=True, exist_ok=True)
    import_log = log_dir / "TmbC01_pikeman_bootstrap_v1_import.log"
    usage_json = log_dir / "TmbC01_pikeman_bootstrap_v1_material_usage.json"
    helper = log_dir / "_pt_material_usage_bootstrap.py"

    for artifact in (blend_out, import_log, usage_json):
        if artifact.exists() and not args.overwrite_artifacts:
            raise FileExistsError(
                f"bootstrap artifact exists; use --overwrite-artifacts intentionally: {artifact}"
            )

    blend_out.parent.mkdir(parents=True, exist_ok=True)

    cmd = blender_command() + [
        "--background",
        "--python-exit-code", "1",
        "--python", str(importer), "--",
        "--smd", str(local_model),
        "--skeleton", str(local_skeleton),
        "--model-info", str(local_inx),
        "--lod", "high",
        "--client-root", str(client_root),
        "--out", str(blend_out),
        "--no-pack",
    ]

    print("=== PIKEMAN V3 IMPORT INVENTORY ===")
    print("COMMAND=" + " ".join(shlex.quote(x) for x in cmd))
    cp = subprocess.run(cmd, text=True, capture_output=True)
    combined = (cp.stdout or "") + (cp.stderr or "")
    import_log.write_text(combined, encoding="utf-8")
    print(combined, end="" if combined.endswith("\n") else "\n")
    if cp.returncode != 0:
        raise subprocess.CalledProcessError(cp.returncode, cmd)
    if "static_armature_import_check=PASS" not in combined:
        raise RuntimeError("V3 importer did not report static_armature_import_check=PASS")

    materials, objects = parse_import_log(combined)

    missing_lod = [x for x in all_lod_names if x not in objects]
    if missing_lod:
        raise RuntimeError(f"INX LOD objects missing from SMD import: {missing_lod}")

    material_usage_probe_script(helper)
    cmd2 = blender_command() + [
        "--background", str(blend_out),
        "--python-exit-code", "1",
        "--python", str(helper), "--",
        str(usage_json),
    ]
    print("=== MATERIAL USAGE INVENTORY ===")
    print("COMMAND=" + " ".join(shlex.quote(x) for x in cmd2))
    subprocess.run(cmd2, check=True)
    usage = json.loads(usage_json.read_text(encoding="utf-8"))

    texture_rows = roles_from_materials(materials, usage)
    if not texture_rows:
        raise RuntimeError("no texture/material mappings resolved")

    # Require the two textures already identified by the physical identity scan.
    expected = {"tmbc01.bmp", "tmbc03.bmp"}
    found = {pathlib.PurePosixPath(x["target"]).name.lower() for x in texture_rows}
    if not expected.issubset(found):
        raise RuntimeError(
            f"expected Pikeman textures {sorted(expected)}, found {sorted(found)}"
        )

    components = [
        {
            "role": "model",
            "required": True,
            "editable": True,
            "local_source": local_model.name,
            "roundtrip_target": model_target,
            "workspace_path": f"Characters/Pikeman/C01/Model/{local_model.name}",
            "category": "Characters",
            "subcategory": "Character Model",
            "representation": "Equipped Body Mesh",
        },
        {
            "role": "skeleton",
            "required": True,
            "editable": False,
            "local_source": local_skeleton.name,
            "roundtrip_target": skeleton_target,
            "workspace_path": f"Characters/Pikeman/C01/Skeleton/{local_skeleton.name}",
            "category": "Characters",
            "subcategory": "Character Skeleton",
            "representation": "Skeleton",
        },
        {
            "role": "model_info",
            "required": True,
            "editable": False,
            "local_source": local_inx.name,
            "roundtrip_target": str(inx.relative_to(client_root)).replace("\\", "/"),
            "workspace_path": f"Characters/Pikeman/C01/Config/{local_inx.name}",
            "category": "Characters",
            "subcategory": "Character Model Info",
            "representation": "INX",
        },
    ]

    for row in texture_rows:
        target = row["target"]
        basename = pathlib.PurePosixPath(target).name
        components.append({
            "role": row["role"],
            "required": True,
            "editable": True,
            "roundtrip_target": target,
            "workspace_path": f"Characters/Pikeman/C01/Textures/{basename}",
            "category": "Textures",
            "subcategory": "Character Texture",
            "representation": row["representation"],
            "material_slots": row["material_slots"],
        })

    profile = {
        "profile_version": 1,
        "profile_id": "pikeman_c01_m4",
        "asset_set_id": "CHARSET-PIKEMAN-C01-M4",
        "display_name": "Pikeman C01 / M4",
        "class_code": "C",
        "owner_name": "Pikeman",
        "description": (
            "Generated from the current client by PT Pikeman Bootstrap V1. "
            "Identity chain: M4Bip.inx -> TmbC01.smd + m4.smb."
        ),
        "model_smd": local_model.name,
        "model_smd_sha256": sha256_file(local_model),
        "skeleton_smb": local_skeleton.name,
        "skeleton_smb_sha256": sha256_file(local_skeleton),
        "model_info_inx": local_inx.name,
        "model_info_inx_sha256": sha256_file(local_inx),
        "roundtrip_target": model_target,
        "runtime_object_limit": 128,
        "runtime_vertex_limit_per_object": 30000,
        "runtime_face_limit_per_object": 30000,
        "lod_groups": lod_groups,
        "known_mesh_objects": list(objects.keys()),
        "baseline_counts": {
            name: {
                "vertices": row["vertices"],
                "faces": row["faces"],
            }
            for name, row in objects.items()
        },
        "components": components,
        "geometry_status": {
            "single_object_runtime_proven": False,
            "high_lod_runtime_proven": False,
            "all_lods_runtime_proven": False,
            "artist_free_topology_runtime_proven": False,
            "production_force_test_allowed": False,
            "status_note": "UNPROVEN for Pikeman until explicit Pikeman runtime gates pass",
        },
        "identity_evidence": {
            "status": "PHYSICAL_CHAIN_PROVEN",
            "inx": str(inx.relative_to(client_root)).replace("\\", "/"),
            "szModelFile": model_logical,
            "szMotionFile": motion_logical,
            "szSubModelFile": submodel_logical,
            "model_smd_sha256": sha256_file(local_model),
            "skeleton_smb_sha256": sha256_file(local_skeleton),
            "model_info_inx_sha256": sha256_file(local_inx),
        },
        "bootstrap_evidence": {
            "import_log": str(import_log),
            "bootstrap_blend": str(blend_out),
            "material_usage_report": str(usage_json),
            "static_armature_import_check": "PASS",
            "client_modified": False,
        },
    }

    profile_out = (root / args.profile_out).resolve()
    if profile_out.exists() and not args.overwrite_artifacts:
        raise FileExistsError(
            f"profile already exists; use --overwrite-artifacts intentionally: {profile_out}"
        )
    profile_out.parent.mkdir(parents=True, exist_ok=True)
    tmp = profile_out.with_name(profile_out.name + ".tmp")
    tmp.write_text(json.dumps(profile, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    os.replace(tmp, profile_out)

    report = log_dir / "pikeman_bootstrap_report.json"
    report.write_text(json.dumps({
        "bootstrap_check": "PASS",
        "profile": str(profile_out),
        "profile_sha256": sha256_file(profile_out),
        "copy_status": copy_status,
        "lod_groups": lod_groups,
        "object_count": len(objects),
        "material_count": len(materials),
        "textures": texture_rows,
        "client_modified": False,
    }, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    print("PT_PIKEMAN_BOOTSTRAP_V1")
    print("identity=M4Bip.inx -> TmbC01.smd + m4.smb")
    print(f"model_sha256={sha256_file(local_model)}")
    print(f"skeleton_sha256={sha256_file(local_skeleton)}")
    print(f"inx_sha256={sha256_file(local_inx)}")
    print(f"lod_high={lod_groups['high']}")
    print(f"lod_default={lod_groups['default']}")
    print(f"lod_low={lod_groups['low']}")
    print(f"object_count={len(objects)}")
    print(f"known_mesh_objects={list(objects.keys())}")
    for row in texture_rows:
        print(
            f"TEXTURE role={row['role']} target={row['target']} "
            f"material_slots={row['material_slots']} used_faces={row['used_face_count']}"
        )
    print(f"profile={profile_out}")
    print(f"profile_sha256={sha256_file(profile_out)}")
    print(f"report={report}")
    print("client_modified=False")
    print("pikeman_bootstrap_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"pikeman_bootstrap_check=FAIL: {type(exc).__name__}: {exc}")
        raise

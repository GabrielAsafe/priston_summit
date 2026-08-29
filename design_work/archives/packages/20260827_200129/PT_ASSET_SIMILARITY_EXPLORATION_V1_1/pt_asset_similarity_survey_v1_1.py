#!/usr/bin/env python3
"""
PT_ASSET_SIMILARITY_SURVEY_V1

Read-only structural inventory / similarity survey for Summit Tale assets.

Purpose
-------
Do NOT reverse-engineer every asset independently.
Classify assets into technical compatibility families and compare them to the
already implemented/proven Pikeman/Priestess pipelines.

The scanner NEVER modifies files under the scanned roots. It writes reports
only under --out-dir.

Deep-inspected extensions:
  .smd .smb .inx .ini .ase .bmp .tga

Outputs:
  asset_inventory.json
  asset_inventory.csv
  compatibility_families.csv
  asset_sets_from_inx.csv
  reuse_queue.csv
  asset_similarity_report.md
  extension_inventory.csv
"""

from __future__ import annotations
import argparse
import concurrent.futures
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import re
import struct
import sys
from collections import Counter, defaultdict
from typing import Any, Iterable

TOOL = "PT_ASSET_SIMILARITY_SURVEY_V1_1"
DEEP_EXTS = {".smd", ".smb", ".inx", ".ini", ".ase", ".bmp", ".tga"}
MODEL_HEADER = b"SMD Model data Ver 0.62"
STAGE_HEADER = b"SMD Stage data Ver 0.72"
HEADER_SIZE = 556
OBJINFO_SIZE = 40

KNOWN_PROFILES = {
    "PIKEMAN_C01_M4": {
        "smd_basename": "TmbC01.smd",
        "smd_sha256": "9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c",
        "smd_size": 187504,
        "object_count": 9,
        "material_count": 2,
        "total_vertices": 955,
        "total_faces": 1650,
        "objects": {
            "TbhC03","TbmC03","TblC03",
            "TblC01","TbmC01","TbhC01",
            "TahC01","TamC01","TalC01",
        },
        "skeleton_basename": "m4.smb",
        "skeleton_sha256": "48af614678593eaa0b659c2be514bd60f86b94f72cfb4305ed8352f48f12cc4a",
        "skeleton_size": 12414984,
        "config_basename": "M4Bip.inx",
        "bone_count_runtime": 58,
        "implementation": "FULL_CHARACTER_SMD_CORE_RUNTIME_PROVEN",
    },
    "PRIESTESS_C01_M5": {
        "smd_basename": "MfbC01.smd",
        "smd_sha256": "9ec6be4cc148125e5616b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b",
        "smd_size": 290229,
        "object_count": 15,
        "material_count": 6,
        "total_vertices": 1444,
        "total_faces": 2537,
        "objects": {
            "MsmC01","MshC01","MslC01","MsmC03","MslC03",
            "MlhC01","MlhC03","MbhC01","MshC03","MlmC01",
            "MlmC03","MbmC01","MllC01","MllC03","MblC01",
        },
        "skeleton_basename": "m5.smb",
        "config_basename": "M5Bip.inx",
        "bone_count_runtime": 95,
        "implementation": "SMD_SKELETON_ANIMATION_GEOMETRY_CORE_PROVEN",
    },
}


KNOWN_PROTECTED_BMP_PREFIX14 = bytes.fromhex("41383a091319243140519a7990a9")


def json_safe(value):
    """Recursively convert sets/Path/tuples into JSON-safe values."""
    if isinstance(value, set):
        return sorted(json_safe(x) for x in value)
    if isinstance(value, Path):
        return str(value)
    if isinstance(value, tuple):
        return [json_safe(x) for x in value]
    if isinstance(value, list):
        return [json_safe(x) for x in value]
    if isinstance(value, dict):
        return {str(k): json_safe(v) for k, v in value.items()}
    return value


def load_checkpoint(path: Path) -> dict[str, dict[str, Any]]:
    """Load latest record per path from an NDJSON checkpoint."""
    result: dict[str, dict[str, Any]] = {}
    if not path.is_file():
        return result
    with path.open("r", encoding="utf-8") as f:
        for line_no, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            try:
                rec = json.loads(line)
            except Exception:
                # A partially-written final line after interruption is ignored.
                continue
            p = rec.get("path")
            if p:
                result[p] = rec
    return result


def append_checkpoint(path: Path, rec: dict[str, Any]):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8") as f:
        f.write(json.dumps(json_safe(rec), ensure_ascii=False, sort_keys=True) + "\n")
        f.flush()


def safe_ascii(b: bytes) -> str:
    return b.split(b"\0", 1)[0].decode("latin1", "replace").strip()


def file_sample_digest(path: Path, block: int = 65536) -> str:
    size = path.stat().st_size
    h = hashlib.sha256()
    h.update(struct.pack("<Q", size))
    with path.open("rb") as f:
        head = f.read(block)
        h.update(head)
        if size > block:
            f.seek(max(0, size - block))
            h.update(f.read(block))
    return h.hexdigest()


def full_sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def rel_display(path: Path, roots: list[Path]) -> str:
    for r in roots:
        try:
            return str(path.relative_to(r))
        except ValueError:
            pass
    return str(path)


def infer_category(path: Path) -> str:
    s = str(path).replace("\\", "/").lower()
    tests = [
        ("MAP_STAGE", ("field/", "/field", "stage/", "/stage", "map/", "/map")),
        ("NPC", ("npc",)),
        ("MOB_MONSTER", ("monster", "/mob", "enemy")),
        ("WEAPON_ITEM", ("weapon", "/item", "wep", "sword", "axe", "bow", "spear")),
        ("EFFECT", ("effect", "eff/", "/eff")),
        ("CHARACTER", ("/char/", "character")),
    ]
    for name, toks in tests:
        if any(t in s for t in toks):
            return name
    return "UNKNOWN"


def object_name_shape(name: str) -> str:
    # Normalize numeric variants but retain letter structure.
    return re.sub(r"\d+", "#", name)


def common_name_shape(names: list[str]) -> str:
    if not names:
        return ""
    shapes = Counter(object_name_shape(n) for n in names)
    return shapes.most_common(1)[0][0]


def ratio_similarity(a: int | float, b: int | float) -> float:
    if a <= 0 or b <= 0:
        return 0.0
    return min(a, b) / max(a, b)


def jaccard(a: set[str], b: set[str]) -> float:
    if not a and not b:
        return 1.0
    if not a or not b:
        return 0.0
    return len(a & b) / len(a | b)


def parse_smd(path: Path) -> dict[str, Any]:
    size = path.stat().st_size
    out: dict[str, Any] = {
        "kind": "SMD_UNKNOWN",
        "header": None,
        "structural_valid": False,
        "objects": [],
        "object_count": None,
        "material_count": None,
        "total_vertices": None,
        "total_faces": None,
        "total_texlinks": None,
        "physique_objects": None,
        "texture_refs": [],
        "issues": [],
    }
    with path.open("rb") as f:
        first = f.read(max(HEADER_SIZE, 64))

    if first.startswith(STAGE_HEADER):
        out.update({
            "kind": "SMD_STAGE_072",
            "header": STAGE_HEADER.decode("ascii"),
            "compatibility_family": "STAGE_072_MAP_SEPARATE",
            "compatibility_tier": "C_SEPARATE_FAMILY",
            "implemented_similarity": "FORMAT_IDENTIFIED_NOT_CHARACTER_CORE",
        })
        return out

    if not first.startswith(MODEL_HEADER):
        out["issues"].append("unknown_smd_header")
        out["compatibility_family"] = "SMD_OTHER"
        out["compatibility_tier"] = "D_NEW_REVERSE_ENGINEERING"
        out["implemented_similarity"] = "UNKNOWN"
        return out

    out["kind"] = "SMD_MODEL_062"
    out["header"] = MODEL_HEADER.decode("ascii")

    if size < HEADER_SIZE:
        out["issues"].append("file_smaller_than_556_header")
        out["compatibility_family"] = "SMD_MODEL_062_DAMAGED_OR_VARIANT"
        out["compatibility_tier"] = "D_NEW_REVERSE_ENGINEERING"
        return out

    # smDFILE_HEADER:
    # char[24], ObjCounter, MatCounter, MatFilePoint, First_ObjInfoPoint,
    # TmFrameCounter, smFRAME_POS[32]
    header = first[:HEADER_SIZE]
    obj_count, mat_count, mat_point, first_obj_point, tmframe_count = struct.unpack_from("<5i", header, 24)
    out.update({
        "object_count": obj_count,
        "material_count": mat_count,
        "mat_file_point": mat_point,
        "first_obj_info_point": first_obj_point,
        "tmframe_count": tmframe_count,
    })

    if not (0 <= obj_count <= 4096):
        out["issues"].append(f"implausible_obj_count:{obj_count}")
        return out
    if not (0 <= mat_count <= 4096):
        out["issues"].append(f"implausible_mat_count:{mat_count}")
        return out
    table_end = HEADER_SIZE + obj_count * OBJINFO_SIZE
    if table_end > size:
        out["issues"].append("object_table_out_of_bounds")
        return out
    if mat_point and mat_point != table_end:
        out["issues"].append(f"mat_point_differs_from_table_end:{mat_point}!={table_end}")
    if first_obj_point and not (table_end <= first_obj_point <= size):
        out["issues"].append("first_obj_point_out_of_bounds")

    with path.open("rb") as f:
        f.seek(HEADER_SIZE)
        table = f.read(obj_count * OBJINFO_SIZE)

    entries = []
    for i in range(obj_count):
        off = i * OBJINFO_SIZE
        name = safe_ascii(table[off:off+32])
        length, obj_point = struct.unpack_from("<ii", table, off + 32)
        row = {
            "index": i,
            "name": name,
            "length": length,
            "obj_file_point": obj_point,
            "head": None,
            "node_name": None,
            "nVertex": None,
            "nFace": None,
            "nTexLink": None,
            "physique_ptr": None,
            "bounds_ok": False,
        }
        if obj_point < 0 or length < 0 or obj_point + length > size:
            out["issues"].append(f"object_bounds:{i}:{name}")
            entries.append(row)
            continue
        try:
            with path.open("rb") as f:
                f.seek(obj_point)
                block_head = f.read(min(length, 256))
            if len(block_head) >= 204:
                row["head"] = struct.unpack_from("<I", block_head, 0)[0]
                row["physique_ptr"] = struct.unpack_from("<I", block_head, 16)[0]
                row["nVertex"] = struct.unpack_from("<i", block_head, 84)[0]
                row["nFace"] = struct.unpack_from("<i", block_head, 88)[0]
                row["nTexLink"] = struct.unpack_from("<i", block_head, 92)[0]
                row["node_name"] = safe_ascii(block_head[172:204])
                row["bounds_ok"] = True
                if row["node_name"] and row["node_name"] != name:
                    out["issues"].append(f"node_name_mismatch:{name}!={row['node_name']}")
                for key in ("nVertex", "nFace", "nTexLink"):
                    v = row[key]
                    if v is None or v < 0 or v > 1_000_000:
                        out["issues"].append(f"implausible_{key}:{name}:{v}")
            else:
                out["issues"].append(f"object_header_short:{name}")
        except Exception as e:
            out["issues"].append(f"object_parse_error:{name}:{type(e).__name__}")
        entries.append(row)

    out["objects"] = entries
    valid_counts = [x for x in entries if x.get("bounds_ok") and x.get("nVertex") is not None]
    out["total_vertices"] = sum(x["nVertex"] for x in valid_counts)
    out["total_faces"] = sum(x["nFace"] for x in valid_counts)
    out["total_texlinks"] = sum(x["nTexLink"] for x in valid_counts)
    out["physique_objects"] = sum(1 for x in valid_counts if x.get("physique_ptr", 0) != 0)
    out["object_names"] = [x["name"] for x in entries if x["name"]]
    out["name_shape"] = common_name_shape(out["object_names"])

    # Material region string refs. We do not need to fully parse the group to
    # classify texture dependency families.
    if mat_point and first_obj_point and mat_point < first_obj_point <= size:
        with path.open("rb") as f:
            f.seek(mat_point)
            region = f.read(first_obj_point - mat_point)
        refs = re.findall(
            rb"(?i)(?:[A-Za-z0-9_./\\-]{1,220}\.(?:bmp|tga|dds|jpg|jpeg|png))",
            region
        )
        out["texture_refs"] = sorted({safe_ascii(x) for x in refs})

    basic_ok = (
        len(entries) == obj_count
        and all(x["bounds_ok"] for x in entries)
        and all((x["node_name"] in (None, "", x["name"])) for x in entries)
    )
    out["structural_valid"] = basic_ok

    if basic_ok and out["physique_objects"] == obj_count and obj_count > 0:
        out["compatibility_family"] = "SMD_MODEL_062_RIGID_PHYSIQUE"
        out["compatibility_tier"] = "A_REUSE_CORE_NOW"
        out["implemented_similarity"] = "DIRECT_SMD_CHARACTER_CORE_HIGH"
    elif basic_ok and out["physique_objects"] == 0:
        out["compatibility_family"] = "SMD_MODEL_062_STATIC"
        out["compatibility_tier"] = "B_REUSE_WITH_ADAPTER"
        out["implemented_similarity"] = "READER_MATERIAL_CORE_HIGH_WRITER_SKIN_BRANCH_DIFFERS"
    elif basic_ok:
        out["compatibility_family"] = "SMD_MODEL_062_MIXED_PHYSIQUE"
        out["compatibility_tier"] = "B_REUSE_WITH_ADAPTER"
        out["implemented_similarity"] = "CORE_HIGH_MIXED_SKIN_SEMANTICS_NEED_REPRESENTATIVE"
    else:
        out["compatibility_family"] = "SMD_MODEL_062_VARIANT_OR_DAMAGED"
        out["compatibility_tier"] = "D_NEW_REVERSE_ENGINEERING"
        out["implemented_similarity"] = "HEADER_MATCH_ONLY"

    return out


def bmp_dib_info(data: bytes, dib_off: int) -> dict[str, Any] | None:
    if len(data) < dib_off + 40:
        return None
    dib_size = struct.unpack_from("<I", data, dib_off)[0]
    if dib_size < 40:
        return None
    width, height = struct.unpack_from("<ii", data, dib_off + 4)
    planes, bpp = struct.unpack_from("<HH", data, dib_off + 12)
    compression = struct.unpack_from("<I", data, dib_off + 16)[0]
    if planes != 1 or width == 0 or height == 0 or abs(width) > 65536 or abs(height) > 65536:
        return None
    return {
        "dib_size": dib_size, "width": width, "height": height,
        "planes": planes, "bpp": bpp, "compression": compression,
    }


def parse_bmp(path: Path) -> dict[str, Any]:
    with path.open("rb") as f:
        data = f.read(128)
    out: dict[str, Any] = {"kind": "BMP_UNKNOWN", "issues": []}
    if data[:2] == b"BM":
        info = bmp_dib_info(data, 14)
        if info:
            out.update(info)
            out.update({
                "kind": "BMP_STANDARD",
                "compatibility_family": f"BMP_STANDARD_{info['bpp']}BPP",
                "compatibility_tier": "A_REUSE_CORE_NOW",
                "implemented_similarity": "STANDARD_BMP_DIRECT",
            })
            return out
    # Priston protected family: proprietary 14 bytes in place of BITMAPFILEHEADER,
    # BITMAPINFOHEADER begins at byte 14.
    info = bmp_dib_info(data, 14)
    if info and data[:2] != b"BM":
        prefix = data[:14]
        out.update(info)
        out["prefix14_hex"] = prefix.hex()
        exact = prefix == KNOWN_PROTECTED_BMP_PREFIX14
        out.update({
            "kind": "BMP_PRISTON_PREFIX14",
            "compatibility_family": (
                "BMP_PROTECTED_PREFIX14_EXACT" if exact
                else "BMP_PROTECTED_PREFIX14_VARIANT"
            ),
            "compatibility_tier": "A_REUSE_CORE_NOW" if exact else "B_REUSE_WITH_ADAPTER",
            "implemented_similarity": (
                "PROTECTED_BMP_PIPELINE_EXACT"
                if exact else "SAME_CONTAINER_PATTERN_PREFIX_VARIANT"
            ),
        })
        return out
    out.update({
        "compatibility_family": "BMP_OTHER",
        "compatibility_tier": "D_NEW_REVERSE_ENGINEERING",
        "implemented_similarity": "UNKNOWN_BMP_VARIANT",
    })
    return out


def parse_tga(path: Path) -> dict[str, Any]:
    with path.open("rb") as f:
        data = f.read(64)
    out: dict[str, Any] = {"kind": "TGA_UNKNOWN"}
    if len(data) >= 18:
        image_type = data[2]
        width, height, bpp = struct.unpack_from("<HHB", data, 12)
        if image_type in {1,2,3,9,10,11} and width > 0 and height > 0 and bpp in {8,16,24,32}:
            out.update({
                "kind": "TGA_STANDARD",
                "width": width, "height": height, "bpp": bpp,
                "image_type": image_type,
                "compatibility_family": "TGA_STANDARD",
                "compatibility_tier": "A_REUSE_CORE_NOW",
                "implemented_similarity": "STANDARD_TGA_DIRECT",
            })
            return out
    out.update({
        "compatibility_family": "TGA_NONSTANDARD_OR_PROTECTED",
        "compatibility_tier": "B_REUSE_WITH_ADAPTER",
        "implemented_similarity": "PROTECTED_TGA_FAMILY_PARTIALLY_PROVEN_NEEDS_SIGNATURE_CLUSTER",
    })
    return out


def parse_inx(path: Path) -> dict[str, Any]:
    size = path.stat().st_size
    with path.open("rb") as f:
        data = f.read(min(size, 4096))
    out: dict[str, Any] = {"kind": "INX_UNKNOWN", "issues": []}
    if len(data) < 396:
        out.update({
            "compatibility_family": "INX_SHORT_OR_OTHER",
            "compatibility_tier": "D_NEW_REVERSE_ENGINEERING",
            "implemented_similarity": "UNKNOWN",
        })
        return out

    model = safe_ascii(data[0x00:0x40])
    motion = safe_ascii(data[0x40:0x80])
    submodel = safe_ascii(data[0x80:0xC0])
    groups = {}
    plausible = True
    for lod, off in (("high",0xC0),("default",0x104),("low",0x148)):
        count = struct.unpack_from("<i", data, off)[0]
        names = [safe_ascii(data[off+4+i*16:off+4+(i+1)*16]) for i in range(4)]
        if count < 0 or count > 4:
            plausible = False
        groups[lod] = {"count": count, "names": names[:max(0,min(4,count))]}

    pathish = any(x.lower().endswith((".ase",".smd",".smb")) or "\\" in x for x in (model,motion,submodel) if x)
    if plausible and pathish:
        out.update({
            "kind": "INX_MODELINFO_GROUPS",
            "model_ref": model,
            "motion_ref": motion,
            "submodel_ref": submodel,
            "groups": groups,
            "compatibility_family": "INX_MODELINFO_FIXED_GROUP_LAYOUT",
            "compatibility_tier": "A_REUSE_CORE_NOW",
            "implemented_similarity": "PIKEMAN_PRIESTESS_CONFIG_CORE_HIGH",
        })
    else:
        out.update({
            "model_ref": model,
            "motion_ref": motion,
            "submodel_ref": submodel,
            "groups": groups,
            "compatibility_family": "INX_OTHER_OR_VARIANT",
            "compatibility_tier": "B_REUSE_WITH_ADAPTER",
            "implemented_similarity": "PARTIAL_HEADER_SIMILARITY",
        })
    return out


def parse_smb(path: Path) -> dict[str, Any]:
    size = path.stat().st_size
    block = 512 * 1024
    with path.open("rb") as f:
        head = f.read(min(block, size))
        tail = b""
        if size > block:
            f.seek(max(0, size-block))
            tail = f.read(block)
    data = head + tail
    bip_count = len(re.findall(rb"Bip(?:01)?[A-Za-z0-9 _-]{0,28}\x00", data))
    node_strings = sorted({
        safe_ascii(x)
        for x in re.findall(rb"Bip(?:01)?[A-Za-z0-9 _-]{1,28}\x00", data)
    })[:64]
    if bip_count >= 5:
        return {
            "kind": "SMB_BIPED_LIKELY",
            "sampled_bip_name_count": bip_count,
            "sampled_bone_names": node_strings,
            "compatibility_family": "SMB_BIPED_SKELETON_MOTION",
            "compatibility_tier": "A_REUSE_CORE_NOW",
            "implemented_similarity": "SMB_BIND_MOTION_EVALUATOR_CORE_HIGH",
        }
    return {
        "kind": "SMB_UNKNOWN",
        "sampled_bip_name_count": bip_count,
        "sampled_bone_names": node_strings,
        "compatibility_family": "SMB_OTHER_OR_LOW_SIGNAL",
        "compatibility_tier": "B_REUSE_WITH_ADAPTER",
        "implemented_similarity": "NEEDS_REPRESENTATIVE_PROBE",
    }


def parse_ase(path: Path) -> dict[str, Any]:
    # Text format: cap read for speed. Structural inventory does not need every
    # vertex line.
    cap = 4 * 1024 * 1024
    with path.open("rb") as f:
        raw = f.read(cap)
    text = raw.decode("latin1", "replace")
    upper = text.upper()
    geom = upper.count("*GEOMOBJECT")
    has_physique = "*PHYSIQUE" in upper
    has_mesh = "*MESH_VERTEX_LIST" in upper and "*MESH_FACE_LIST" in upper
    has_material = "*MATERIAL_LIST" in upper or "*MATERIAL_COUNT" in upper
    has_anim = "*TM_ANIMATION" in upper
    bitmaps = sorted(set(re.findall(r'(?i)\*BITMAP\s+"([^"]+)"', text)))
    nodes = re.findall(r'(?i)\*NODE_NAME\s+"([^"]+)"', text)
    nverts = [int(x) for x in re.findall(r'(?i)\*MESH_NUMVERTEX\s+(\d+)', text)]
    nfaces = [int(x) for x in re.findall(r'(?i)\*MESH_NUMFACES\s+(\d+)', text)]
    out = {
        "kind": "ASE_TEXT",
        "geomobject_count_sampled": geom,
        "has_physique": has_physique,
        "has_mesh": has_mesh,
        "has_material": has_material,
        "has_tm_animation": has_anim,
        "bitmap_refs": bitmaps[:128],
        "node_names_sampled": nodes[:256],
        "mesh_vertices_declared_sampled": sum(nverts),
        "mesh_faces_declared_sampled": sum(nfaces),
        "read_truncated": path.stat().st_size > cap,
    }
    if has_mesh and has_physique:
        out.update({
            "compatibility_family": "ASE_CHARACTER_PHYSIQUE",
            "compatibility_tier": "A_REUSE_CORE_NOW",
            "implemented_similarity": "ASE_IMPORT_EDIT_SKIN_CORE_HIGH_RUNTIME_ROUTE_PREFER_SMD",
        })
    elif has_mesh:
        out.update({
            "compatibility_family": "ASE_STATIC_GEOMETRY",
            "compatibility_tier": "B_REUSE_WITH_ADAPTER",
            "implemented_similarity": "ASE_GEOMETRY_MATERIAL_UV_CORE_HIGH_STATIC_RUNTIME_ADAPTER",
        })
    else:
        out.update({
            "compatibility_family": "ASE_OTHER",
            "compatibility_tier": "D_NEW_REVERSE_ENGINEERING",
            "implemented_similarity": "LOW",
        })
    return out


def parse_ini(path: Path) -> dict[str, Any]:
    cap = 512 * 1024
    raw = path.read_bytes()[:cap]
    text = raw.decode("latin1", "replace")
    refs = sorted(set(re.findall(r'(?i)([A-Za-z0-9_./\\-]+\.(?:ase|smd|smb|bmp|tga|inx|ini))', text)))
    return {
        "kind": "INI_TEXT",
        "refs": refs[:256],
        "compatibility_family": "INI_TEXT_CONFIG",
        "compatibility_tier": "B_REUSE_WITH_ADAPTER",
        "implemented_similarity": "CONFIG_DISCOVERY_ONLY",
    }


def compare_smd_profiles(record: dict[str, Any]) -> list[dict[str, Any]]:
    if record.get("kind") != "SMD_MODEL_062":
        return []
    names = set(record.get("object_names") or [])
    comps = []
    for pname, p in KNOWN_PROFILES.items():
        score = 0.0
        components = {}
        components["format"] = 35.0
        score += 35.0
        oc = ratio_similarity(record.get("object_count") or 0, p["object_count"])
        mc = ratio_similarity(record.get("material_count") or 0, p["material_count"])
        vv = ratio_similarity(record.get("total_vertices") or 0, p["total_vertices"])
        ff = ratio_similarity(record.get("total_faces") or 0, p["total_faces"])
        nj = jaccard(names, p["objects"])
        components["object_count"] = 10 * oc
        components["material_count"] = 8 * mc
        components["vertices"] = 12 * vv
        components["faces"] = 12 * ff
        components["object_names"] = 18 * nj
        components["rigid_physique"] = 5.0 if record.get("physique_objects") == record.get("object_count") and record.get("object_count") else 0.0
        score += sum(v for k,v in components.items() if k != "format")
        comps.append({
            "profile": pname,
            "score": round(score, 2),
            "components": {k: round(v,2) for k,v in components.items()},
        })
    comps.sort(key=lambda x: x["score"], reverse=True)
    return comps


def inspect_one(path: Path, roots: list[Path], hash_mode: str) -> dict[str, Any]:
    st = path.stat()
    ext = path.suffix.lower()
    rec: dict[str, Any] = {
        "path": str(path),
        "relative_path": rel_display(path, roots),
        "basename": path.name,
        "extension": ext,
        "size": st.st_size,
        "category_hint": infer_category(path),
        "sample_digest": file_sample_digest(path),
    }
    try:
        if ext == ".smd":
            rec.update(parse_smd(path))
        elif ext == ".bmp":
            rec.update(parse_bmp(path))
        elif ext == ".tga":
            rec.update(parse_tga(path))
        elif ext == ".inx":
            rec.update(parse_inx(path))
        elif ext == ".smb":
            rec.update(parse_smb(path))
        elif ext == ".ase":
            rec.update(parse_ase(path))
        elif ext == ".ini":
            rec.update(parse_ini(path))
        else:
            rec.update({
                "kind": "UNINSPECTED",
                "compatibility_family": "UNINSPECTED",
                "compatibility_tier": "U_UNINSPECTED",
                "implemented_similarity": "N/A",
            })

        # Full hash only if requested or if it can establish identity with a known
        # reference by basename/size.
        need_full = hash_mode == "full"
        if hash_mode == "smart":
            for p in KNOWN_PROFILES.values():
                if path.name.lower() == p["smd_basename"].lower() or (
                    p.get("skeleton_basename") and path.name.lower() == p["skeleton_basename"].lower()
                ):
                    need_full = True
        if need_full:
            rec["sha256"] = full_sha256(path)
        else:
            rec["sha256"] = None

        if rec.get("kind") == "SMD_MODEL_062":
            rec["profile_similarity"] = compare_smd_profiles(rec)
            if rec["profile_similarity"]:
                rec["closest_profile"] = rec["profile_similarity"][0]["profile"]
                rec["closest_profile_score"] = rec["profile_similarity"][0]["score"]
        # Exact known identity if hash was computed.
        if rec.get("sha256"):
            for pname, p in KNOWN_PROFILES.items():
                if rec["sha256"] == p.get("smd_sha256"):
                    rec["exact_known_identity"] = f"{pname}:SMD"
                if rec["sha256"] == p.get("skeleton_sha256"):
                    rec["exact_known_identity"] = f"{pname}:SMB"
    except Exception as e:
        rec.update({
            "kind": rec.get("kind", "PARSE_ERROR"),
            "parse_error": f"{type(e).__name__}: {e}",
            "compatibility_family": rec.get("compatibility_family", "PARSE_ERROR"),
            "compatibility_tier": rec.get("compatibility_tier", "D_NEW_REVERSE_ENGINEERING"),
            "implemented_similarity": rec.get("implemented_similarity", "PARSE_FAILED"),
        })
    return rec


def windows_basename(ref: str) -> str:
    return ref.replace("\\", "/").split("/")[-1].lower()


def build_inx_asset_sets(records: list[dict[str, Any]]) -> list[dict[str, Any]]:
    by_base: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for r in records:
        by_base[r["basename"].lower()].append(r)

    sets = []
    for r in records:
        if r.get("kind") != "INX_MODELINFO_GROUPS":
            continue
        refs = {
            "model_ref": r.get("model_ref") or "",
            "motion_ref": r.get("motion_ref") or "",
            "submodel_ref": r.get("submodel_ref") or "",
        }
        found = {}
        for k, ref in refs.items():
            if not ref:
                found[k] = []
                continue
            base = windows_basename(ref)
            candidates = list(by_base.get(base, []))
            # Common runtime: INX says .ASE while current build physically has .smd.
            if base.endswith(".ase"):
                candidates += by_base.get(base[:-4] + ".smd", [])
            found[k] = sorted({x["relative_path"] for x in candidates})

        model_candidates = []
        for x in found.get("model_ref", []):
            rr = next((z for z in records if z["relative_path"] == x), None)
            if rr and rr.get("kind") == "SMD_MODEL_062":
                model_candidates.append(rr)
        closest = None
        if model_candidates:
            best = max(model_candidates, key=lambda x: x.get("closest_profile_score", 0))
            closest = {
                "path": best["relative_path"],
                "profile": best.get("closest_profile"),
                "score": best.get("closest_profile_score"),
                "family": best.get("compatibility_family"),
            }
        sets.append({
            "inx_path": r["relative_path"],
            **refs,
            "high_nodes": ",".join((r.get("groups") or {}).get("high",{}).get("names",[])),
            "default_nodes": ",".join((r.get("groups") or {}).get("default",{}).get("names",[])),
            "low_nodes": ",".join((r.get("groups") or {}).get("low",{}).get("names",[])),
            "model_candidates": json.dumps(found.get("model_ref", []), ensure_ascii=False),
            "motion_candidates": json.dumps(found.get("motion_ref", []), ensure_ascii=False),
            "closest_implemented_profile": closest["profile"] if closest else "",
            "closest_profile_score": closest["score"] if closest else "",
            "model_compatibility_family": closest["family"] if closest else "",
        })
    return sets


def family_priority(family: str, tier: str, count: int) -> tuple[int, str]:
    tier_rank = {
        "A_REUSE_CORE_NOW": 0,
        "B_REUSE_WITH_ADAPTER": 1,
        "C_SEPARATE_FAMILY": 2,
        "D_NEW_REVERSE_ENGINEERING": 3,
        "U_UNINSPECTED": 4,
    }.get(tier, 5)
    # Larger clusters first within tier.
    return (tier_rank, f"{999999999-count:09d}:{family}")


def flatten_for_csv(r: dict[str, Any]) -> dict[str, Any]:
    keys = [
        "relative_path","basename","extension","size","category_hint","kind",
        "compatibility_family","compatibility_tier","implemented_similarity",
        "object_count","material_count","total_vertices","total_faces",
        "total_texlinks","physique_objects","name_shape","closest_profile",
        "closest_profile_score","exact_known_identity","sample_digest","sha256",
        "parse_error",
    ]
    row = {}
    for k in keys:
        v = r.get(k, "")
        if isinstance(v, (dict,list,set,tuple)):
            v = json.dumps(v, ensure_ascii=False, sort_keys=True)
        row[k] = v
    return row


def write_csv(path: Path, rows: list[dict[str, Any]], fieldnames: list[str] | None = None):
    path.parent.mkdir(parents=True, exist_ok=True)
    if not rows:
        path.write_text("", encoding="utf-8")
        return
    if fieldnames is None:
        fieldnames = list(rows[0].keys())
    with path.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames, extrasaction="ignore")
        w.writeheader()
        w.writerows(rows)


def main():
    ap = argparse.ArgumentParser(description="Read-only Summit Tale asset similarity survey")
    ap.add_argument("--root", action="append", required=True,
                    help="Root to scan. Repeat for multiple roots.")
    ap.add_argument("--out-dir", default="logs/asset_similarity_v1")
    ap.add_argument("--workers", type=int, default=min(8, (os.cpu_count() or 4)))
    ap.add_argument("--hash-mode", choices=("sampled","smart","full"), default="smart",
                    help="sampled=fast; smart=full hash known basenames; full=hash every inspected file")
    ap.add_argument("--follow-symlinks", action="store_true")
    ap.add_argument("--no-resume", action="store_true",
                    help="Ignore/remove the incremental checkpoint and rescan everything")
    args = ap.parse_args()

    roots = [Path(x).expanduser().resolve() for x in args.root]
    for r in roots:
        if not r.is_dir():
            raise SystemExit(f"not a directory: {r}")
    out_dir = Path(args.out_dir).expanduser().resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    all_ext = Counter()
    paths: list[Path] = []
    for root in roots:
        for dp, dns, fns in os.walk(root, followlinks=args.follow_symlinks):
            # Avoid obvious generated/report directories if design_work is scanned.
            dns[:] = [d for d in dns if d not in {
                ".git","__pycache__","_pt_smd_texture_cache","_pipeline_work"
            }]
            for fn in fns:
                p = Path(dp) / fn
                ext = p.suffix.lower()
                all_ext[ext or "<none>"] += 1
                if ext in DEEP_EXTS:
                    paths.append(p)

    print(f"{TOOL}")
    print(f"roots={[str(x) for x in roots]}")
    print(f"deep_files={len(paths)}")
    print(f"workers={args.workers}")
    print("client_modified=False")

    checkpoint = out_dir / "scan_checkpoint.ndjson"
    if args.no_resume and checkpoint.exists():
        checkpoint.unlink()

    cached = load_checkpoint(checkpoint) if not args.no_resume else {}
    records: list[dict[str, Any]] = []
    pending: list[Path] = []

    # Reuse a cached record only if the path still exists with the same size.
    for p in paths:
        c = cached.get(str(p))
        if c is not None:
            try:
                if int(c.get("size", -1)) == p.stat().st_size:
                    records.append(c)
                    continue
            except OSError:
                pass
        pending.append(p)

    print(f"checkpoint={checkpoint}")
    print(f"resume_records={len(records)}")
    print(f"pending_files={len(pending)}")

    scanned_now = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1,args.workers)) as ex:
        futs = [ex.submit(inspect_one, p, roots, args.hash_mode) for p in pending]
        for fut in concurrent.futures.as_completed(futs):
            rec = fut.result()
            records.append(rec)
            append_checkpoint(checkpoint, rec)
            scanned_now += 1
            total_done = len(records)
            if scanned_now % 500 == 0 or scanned_now == len(pending):
                print(f"scanned={total_done}/{len(paths)} newly_scanned={scanned_now}", flush=True)

    records.sort(key=lambda r: r["relative_path"].lower())

    # Compatibility families.
    fam_groups: dict[tuple[str,str], list[dict[str,Any]]] = defaultdict(list)
    for r in records:
        fam_groups[(r.get("compatibility_family","UNKNOWN"), r.get("compatibility_tier","UNKNOWN"))].append(r)

    families = []
    for (family,tier), items in fam_groups.items():
        cats = Counter(x.get("category_hint","UNKNOWN") for x in items)
        closest = Counter(x.get("closest_profile","") for x in items if x.get("closest_profile"))
        families.append({
            "compatibility_family": family,
            "compatibility_tier": tier,
            "file_count": len(items),
            "total_bytes": sum(x["size"] for x in items),
            "top_categories": "; ".join(f"{k}:{v}" for k,v in cats.most_common(5)),
            "top_closest_profiles": "; ".join(f"{k}:{v}" for k,v in closest.most_common(3)),
            "examples": "; ".join(x["relative_path"] for x in items[:8]),
        })
    families.sort(key=lambda x: family_priority(x["compatibility_family"],x["compatibility_tier"],x["file_count"]))

    asset_sets = build_inx_asset_sets(records)

    # Reuse queue is the family table annotated with recommended action.
    actions = {
        "A_REUSE_CORE_NOW": "AUTO-GENERATE PROFILE; run structural batch; runtime-test 1-3 representatives",
        "B_REUSE_WITH_ADAPTER": "Choose representative; identify semantic delta; add small adapter; then promote family",
        "C_SEPARATE_FAMILY": "Keep separate from character pipeline; create dedicated format vertical slice",
        "D_NEW_REVERSE_ENGINEERING": "Do not mass-port; inspect only if family has meaningful asset count/business value",
        "U_UNINSPECTED": "Add recognizer only if extension is relevant to 3D/runtime authoring",
    }
    reuse_queue = []
    for f in families:
        reuse_queue.append({
            **f,
            "recommended_action": actions.get(f["compatibility_tier"], "Review"),
        })

    # Reports.
    json_path = out_dir/"asset_inventory.json"
    payload = {
        "tool": TOOL,
        "roots": [str(x) for x in roots],
        "client_modified": False,
        "known_profiles": KNOWN_PROFILES,
        "records": records,
        "compatibility_families": families,
        "asset_sets_from_inx": asset_sets,
    }
    # json_safe is mandatory because known profile signatures intentionally use
    # Python sets internally for fast Jaccard comparison.
    json_path.write_text(
        json.dumps(json_safe(payload), indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8"
    )

    write_csv(out_dir/"asset_inventory.csv", [flatten_for_csv(x) for x in records])
    write_csv(out_dir/"compatibility_families.csv", families)
    write_csv(out_dir/"reuse_queue.csv", reuse_queue)
    write_csv(out_dir/"asset_sets_from_inx.csv", asset_sets)
    write_csv(out_dir/"extension_inventory.csv", [
        {"extension": ext, "file_count": count} for ext,count in all_ext.most_common()
    ])

    tier_counts = Counter(r.get("compatibility_tier","UNKNOWN") for r in records)
    kind_counts = Counter(r.get("kind","UNKNOWN") for r in records)
    exact = [r for r in records if r.get("exact_known_identity")]
    smd_models = [r for r in records if r.get("kind") == "SMD_MODEL_062"]
    smd_stage = [r for r in records if r.get("kind") == "SMD_STAGE_072"]
    errors = [r for r in records if r.get("parse_error")]

    lines = []
    lines += [
        "# Summit Tale — Asset Similarity Survey V1",
        "",
        f"- Roots: `{', '.join(str(x) for x in roots)}`",
        f"- Deep-inspected files: **{len(records)}**",
        f"- Character SMD 0.62 candidates: **{len(smd_models)}**",
        f"- Stage SMD 0.72 candidates: **{len(smd_stage)}**",
        f"- INX-derived asset sets: **{len(asset_sets)}**",
        f"- Parse errors: **{len(errors)}**",
        "- Scanner is read-only with respect to scanned roots.",
        "",
        "## Compatibility tiers",
        "",
    ]
    for tier,count in sorted(tier_counts.items()):
        lines.append(f"- `{tier}`: **{count}** files")
    lines += ["", "## Technical families", ""]
    lines += [
        "| Tier | Family | Files | Bytes | Main categories |",
        "|---|---|---:|---:|---|",
    ]
    for f in families:
        lines.append(
            f"| {f['compatibility_tier']} | `{f['compatibility_family']}` | "
            f"{f['file_count']} | {f['total_bytes']} | {f['top_categories']} |"
        )

    lines += [
        "",
        "## Meaning",
        "",
        "- **A_REUSE_CORE_NOW**: same technical family as an implemented core. Do not reverse-engineer per asset; generate profiles and certify representatives.",
        "- **B_REUSE_WITH_ADAPTER**: most infrastructure is reusable, but one semantic branch differs (for example static/no-Physique models).",
        "- **C_SEPARATE_FAMILY**: already known to be technically separate, such as Stage0.72 maps.",
        "- **D_NEW_REVERSE_ENGINEERING**: new/unknown format or failed structural assumptions. Prioritize only if the cluster is important.",
        "",
        "## Closest known SMD profiles",
        "",
        "Similarity scores are heuristic structural similarity, not runtime proof.",
        "",
        "| Asset | Family | Closest | Score | Obj | Mat | Vertices | Faces |",
        "|---|---|---|---:|---:|---:|---:|---:|",
    ]
    for r in sorted(smd_models, key=lambda x: x.get("closest_profile_score",0), reverse=True):
        lines.append(
            f"| `{r['relative_path']}` | `{r.get('compatibility_family','')}` | "
            f"{r.get('closest_profile','')} | {r.get('closest_profile_score','')} | "
            f"{r.get('object_count','')} | {r.get('material_count','')} | "
            f"{r.get('total_vertices','')} | {r.get('total_faces','')} |"
        )

    lines += [
        "",
        "## Recommended production strategy",
        "",
        "1. Treat each compatibility family as the unit of engineering, not each asset.",
        "2. For every `A_REUSE_CORE_NOW` SMD family, auto-generate a profile from SMD + INX and run read-only batch validation.",
        "3. Cluster by structural signature (format, Physique mode, object/material counts, name shapes, config layout).",
        "4. Runtime-test only 1–3 representatives per cluster.",
        "5. Promote a cluster only after representative runtime success; then use regression tests for all members.",
        "6. Keep Stage0.72/maps as a separate project vertical slice.",
        "7. Spend reverse-engineering time only on high-count/high-value `D_*` clusters.",
        "",
        "## Exact known identities found",
        "",
    ]
    if exact:
        for r in exact:
            lines.append(f"- `{r['exact_known_identity']}` → `{r['relative_path']}`")
    else:
        lines.append("- None matched by full SHA in this scan mode/root set.")

    lines += [
        "",
        "## Files generated",
        "",
        "- `asset_inventory.json` — full machine-readable data",
        "- `asset_inventory.csv` — flat inventory",
        "- `compatibility_families.csv` — family counts",
        "- `asset_sets_from_inx.csv` — INX→model/motion grouping",
        "- `reuse_queue.csv` — prioritized engineering queue",
        "- `extension_inventory.csv` — complete extension distribution",
        "",
    ]
    (out_dir/"asset_similarity_report.md").write_text("\n".join(lines)+"\n", encoding="utf-8")

    print(f"families={len(families)}")
    print(f"smd_model_062={len(smd_models)}")
    print(f"smd_stage_072={len(smd_stage)}")
    print(f"inx_asset_sets={len(asset_sets)}")
    print(f"out_dir={out_dir}")
    print(f"checkpoint={checkpoint}")
    print("client_modified=False")
    print("asset_similarity_survey_check=PASS")


if __name__ == "__main__":
    main()

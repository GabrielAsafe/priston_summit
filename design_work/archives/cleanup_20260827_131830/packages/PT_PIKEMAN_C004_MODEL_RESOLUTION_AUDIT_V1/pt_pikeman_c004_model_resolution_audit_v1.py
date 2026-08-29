#!/usr/bin/env python3
"""
PT Pikeman C004/C005 Model Resolution Audit V1

Read-only.

Reads the real _MODELGROUP selections from C004.inx and C005.inx, then scans
every client .smd bytewise (case-insensitive) for those NodeNames.

Purpose:
- prove whether the INX-selected nodes live in the model named by szModelFile;
- discover alternate SMDs that contain missing nodes;
- detect complete per-LOD or complete all-LOD candidate SMDs;
- do not weaken Blender/importer missing-node validation.

No client file is modified.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import struct
from collections import defaultdict


CONFIGS = ["C004.inx", "C005.inx"]
GROUPS = [("high", 0x0C0), ("default", 0x104), ("low", 0x148)]


def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def cstr(data, off, size):
    raw = data[off:off+size].split(b"\0", 1)[0]
    return raw.decode("ascii", "strict").strip()


def group_at(data, off):
    if off + 68 > len(data):
        raise ValueError(f"INX too small at 0x{off:X}")
    count = struct.unpack_from("<i", data, off)[0]
    if not 0 <= count <= 4:
        raise ValueError(f"invalid ModelNameCnt={count} at 0x{off:X}")
    names = []
    for i in range(count):
        name = cstr(data, off + 4 + i*16, 16)
        if not name:
            raise ValueError(f"empty active node at 0x{off:X}")
        names.append(name)
    return names


def normalize_rel(s):
    s = s.replace("\\", "/")
    s = re.sub(r"/+", "/", s).strip("/")
    return pathlib.PurePosixPath(s).as_posix()


def expected_smd_target(model_logical):
    return pathlib.PurePosixPath(normalize_rel(model_logical)).with_suffix(".smd").as_posix()


def scan_smd_for_nodes(path, node_names):
    data = path.read_bytes().lower()
    return {
        n: (n.encode("ascii").lower() in data)
        for n in node_names
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--client-root", required=True)
    ap.add_argument("--config", action="append", default=[])
    ap.add_argument(
        "--report",
        default="logs/pikeman_c004_c005_model_resolution_audit_v1.json",
    )
    args = ap.parse_args()

    root = pathlib.Path(args.client_root).expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(root)

    configs = args.config or CONFIGS

    cfg_rows = []
    universe = []
    for cfg_name in configs:
        cfg = root / "char/tmABCD" / cfg_name
        if not cfg.is_file():
            raise FileNotFoundError(cfg)
        data = cfg.read_bytes()
        groups = {name: group_at(data, off) for name, off in GROUPS}
        all_nodes = []
        for lod in ("high", "default", "low"):
            for n in groups[lod]:
                if n not in all_nodes:
                    all_nodes.append(n)
                if n not in universe:
                    universe.append(n)

        model = cstr(data, 0x000, 64)
        motion = cstr(data, 0x040, 64)
        submodel = cstr(data, 0x080, 64)
        cfg_rows.append({
            "config": cfg_name,
            "config_sha256": sha256_file(cfg),
            "szModelFile": model,
            "szMotionFile": motion,
            "szSubModelFile": submodel,
            "expected_model_smd_target": expected_smd_target(model),
            "groups": groups,
            "all_selected_nodes": all_nodes,
        })

    smd_files = sorted(
        p for p in root.rglob("*")
        if p.is_file() and p.suffix.lower() == ".smd"
    )

    smd_rows = []
    refs_by_node = defaultdict(list)

    for p in smd_files:
        try:
            found = scan_smd_for_nodes(p, universe)
        except OSError:
            continue
        present = [n for n, yes in found.items() if yes]
        if not present:
            continue

        rel = str(p.relative_to(root)).replace("\\", "/")
        row = {
            "path": rel,
            "size": p.stat().st_size,
            "sha256": sha256_file(p),
            "present_nodes": present,
        }
        smd_rows.append(row)
        for n in present:
            refs_by_node[n].append(rel)

    for cfg in cfg_rows:
        expected_target_lower = cfg["expected_model_smd_target"].lower()
        expected_row = next(
            (r for r in smd_rows if r["path"].lower() == expected_target_lower),
            None,
        )

        expected_present = set(expected_row["present_nodes"]) if expected_row else set()
        cfg["expected_model_exists"] = bool(
            (root / pathlib.PurePosixPath(cfg["expected_model_smd_target"])).is_file()
        )
        cfg["expected_model_missing_nodes_by_lod"] = {
            lod: [n for n in names if n not in expected_present]
            for lod, names in cfg["groups"].items()
        }

        candidates = []
        for row in smd_rows:
            present = set(row["present_nodes"])
            lod_complete = {
                lod: all(n in present for n in names)
                for lod, names in cfg["groups"].items()
            }
            all_complete = all(lod_complete.values())
            matched_count = sum(n in present for n in cfg["all_selected_nodes"])
            if matched_count:
                candidates.append({
                    "path": row["path"],
                    "matched_selected_node_count": matched_count,
                    "selected_node_count": len(cfg["all_selected_nodes"]),
                    "lod_complete": lod_complete,
                    "all_lods_complete": all_complete,
                })

        candidates.sort(
            key=lambda x: (
                not x["all_lods_complete"],
                -x["matched_selected_node_count"],
                x["path"].lower(),
            )
        )
        cfg["candidate_smds"] = candidates

    external_locations = {
        n: refs_by_node.get(n, [])
        for n in universe
    }

    result = {
        "tool": "PT_PIKEMAN_C004_MODEL_RESOLUTION_AUDIT_V1",
        "client_root": str(root),
        "smd_scanned": len(smd_files),
        "selected_node_universe": universe,
        "configs": cfg_rows,
        "node_locations": external_locations,
        "client_modified": False,
        "model_resolution_audit_check": "PASS",
    }

    report = pathlib.Path(args.report).expanduser()
    if not report.is_absolute():
        report = pathlib.Path.cwd() / report
    report = report.resolve()
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print("PT_PIKEMAN_C004_MODEL_RESOLUTION_AUDIT_V1")
    print(f"smd_scanned={len(smd_files)}")

    for cfg in cfg_rows:
        print(f"CONFIG {cfg['config']}")
        print(f"  model={cfg['szModelFile']!r}")
        print(f"  submodel={cfg['szSubModelFile']!r}")
        print(f"  expected_smd={cfg['expected_model_smd_target']}")
        print(f"  expected_model_exists={cfg['expected_model_exists']}")
        print(
            "  expected_model_missing_nodes_by_lod="
            + repr(cfg["expected_model_missing_nodes_by_lod"])
        )

        full = [x for x in cfg["candidate_smds"] if x["all_lods_complete"]]
        print(f"  complete_all_lod_candidate_count={len(full)}")
        for cand in cfg["candidate_smds"][:12]:
            print(
                f"  CANDIDATE {cand['path']} "
                f"matched={cand['matched_selected_node_count']}/"
                f"{cand['selected_node_count']} "
                f"lod_complete={cand['lod_complete']} "
                f"all_lods_complete={cand['all_lods_complete']}"
            )

    print("NODE_LOCATIONS")
    for n in universe:
        print(f"  {n}: {external_locations[n]}")

    print(f"report={report}")
    print("client_modified=False")
    print("model_resolution_audit_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"model_resolution_audit_check=FAIL: {type(exc).__name__}: {exc}")
        raise

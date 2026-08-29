#!/usr/bin/env python3
"""
PT Free Mesh Multi-Object Validator V1

Runs the proven generic V2 validator for multiple Blender mesh objects in one
Blender session and emits one aggregate report.

No mesh is saved or modified on disk.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import sys


def load_core():
    path = pathlib.Path(__file__).resolve().with_name("pt_free_mesh_validate_v2.py")
    if not path.is_file():
        raise FileNotFoundError(path)
    spec = importlib.util.spec_from_file_location("pt_free_mesh_validate_v2_core", path)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)
    return mod


def parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--smd", required=True)
    p.add_argument("--skeleton", required=True)
    p.add_argument("--object", action="append", required=True)
    p.add_argument("--report", required=True)
    p.add_argument("--report-dir", required=True)
    p.add_argument("--max-fit-error", type=float, default=0.02)
    p.add_argument("--expected-source-sha256")
    return p.parse_args(argv)


def main(argv):
    a = parse_args(argv)
    targets = list(dict.fromkeys(a.object))
    if len(targets) != len(a.object):
        raise ValueError("duplicate --object values are not allowed")

    core = load_core()
    report_dir = pathlib.Path(a.report_dir).expanduser().resolve()
    report_dir.mkdir(parents=True, exist_ok=True)

    rows = []
    for target in targets:
        rp = report_dir / f"{target}_validation.json"
        args = [
            "--smd", a.smd,
            "--skeleton", a.skeleton,
            "--object", target,
            "--report", str(rp),
            "--max-fit-error", str(a.max_fit_error),
        ]
        if a.expected_source_sha256:
            args += ["--expected-source-sha256", a.expected_source_sha256]

        print(f"\n--- VALIDATE {target} ---")
        core.main(args)

        row = json.loads(rp.read_text(encoding="utf-8"))
        if row.get("validation_check") != "PASS":
            raise RuntimeError(f"{target}: validator did not return PASS")
        rows.append(row)

    aggregate = {
        "tool": "PT_FREE_MESH_MULTI_VALIDATE_V1",
        "objects": targets,
        "object_count": len(targets),
        "object_reports": rows,
        "all_objects_passed": True,
        "validation_check": "PASS",
    }

    out = pathlib.Path(a.report).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(aggregate, ensure_ascii=False, indent=2), encoding="utf-8")

    print("\nPT_FREE_MESH_MULTI_VALIDATE_V1")
    print(f"objects={targets}")
    print(f"object_count={len(targets)}")
    for row in rows:
        print(
            f"OBJECT {row['object']}: "
            f"V={row['vertices']} "
            f"F={row['export_triangles']} "
            f"materials={row['used_materials']} "
            f"bones={row['rigid_bone_count']} "
            f"check=PASS"
        )
    print(f"report={out}")
    print("multi_object_validation_check=PASS")


if __name__ == "__main__":
    try:
        argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"multi_object_validation_check=FAIL: {type(exc).__name__}: {exc}")
        raise

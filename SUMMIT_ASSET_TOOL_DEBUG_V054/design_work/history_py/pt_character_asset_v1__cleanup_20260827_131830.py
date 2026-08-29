#!/usr/bin/env python3
"""
PT Character Asset V1

Treat a Summit Tale character as one operational asset set.

High-level commands:
  inspect         resolve model / skeleton / INX / textures and write an asset manifest
  sync-workspace  register/copy resolved components into the round-trip workspace
  build           run the runtime-proven all-LODs geometry pipeline from the profile
  deploy-plan     dry-run deployment for the character model
  deploy          verified deployment for the character model

This tool does not silently use --force-test. Production remains REVIEW_REQUIRED.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import subprocess
import sys
from datetime import datetime, timezone


def sha256_file(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def stable_asset_id(roundtrip_target: str) -> str:
    norm = roundtrip_target.replace("\\", "/").lstrip("./").lower()
    return "AST-" + hashlib.sha1(norm.encode("utf-8")).hexdigest()[:12].upper()


def load_json(path: pathlib.Path):
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: pathlib.Path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")


def resolve(root: pathlib.Path, value: str | pathlib.Path) -> pathlib.Path:
    p = pathlib.Path(value).expanduser()
    return p.resolve() if p.is_absolute() else (root / p).resolve()


def norm_rel(value: str) -> str:
    return value.replace("\\", "/").lstrip("./")


def safe_client_path(client_root: pathlib.Path, rel: str) -> pathlib.Path:
    dst = (client_root / norm_rel(rel)).resolve()
    try:
        dst.relative_to(client_root)
    except ValueError as exc:
        raise ValueError(f"unsafe client path outside root: {rel}") from exc
    return dst


def client_basename_index(client_root: pathlib.Path):
    index = {}
    for p in client_root.rglob("*"):
        if not p.is_file():
            continue
        index.setdefault(p.name.lower(), []).append(p)
    return index


def discover_component(root, client_root, basename_index, comp):
    local = None
    if comp.get("local_source"):
        candidate = resolve(root, comp["local_source"])
        if candidate.is_file():
            local = candidate

    target = comp.get("roundtrip_target")
    physical = None
    resolution = None

    if target:
        candidate = safe_client_path(client_root, target)
        if candidate.is_file():
            physical = candidate
            resolution = "EXACT_ROUNDTRIP_TARGET"
    elif comp.get("discover_client_by_basename"):
        name = comp["discover_client_by_basename"].lower()
        matches = basename_index.get(name, [])
        if len(matches) == 1:
            physical = matches[0].resolve()
            target = physical.relative_to(client_root).as_posix()
            resolution = "UNIQUE_BASENAME"
        elif len(matches) > 1:
            resolution = "AMBIGUOUS_BASENAME"
        else:
            resolution = "BASENAME_NOT_FOUND"
    else:
        resolution = "NO_CLIENT_DISCOVERY_RULE"

    required = bool(comp.get("required", False))
    # Build dependencies can be satisfied by local source even if the client
    # copy is not uniquely located yet.
    available = physical is not None or local is not None
    status = "PASS" if available else ("MISSING_REQUIRED" if required else "OPTIONAL_MISSING")

    chosen = physical or local
    row = dict(comp)
    row.update({
        "roundtrip_target": target,
        "asset_id": stable_asset_id(target) if target else None,
        "client_path": str(physical) if physical else None,
        "local_source_path": str(local) if local else None,
        "resolution": resolution,
        "available": available,
        "status": status,
        "size": chosen.stat().st_size if chosen else None,
        "sha256": sha256_file(chosen) if chosen else None,
    })
    return row


def inspect_asset(root, profile_path, profile, client_root, workspace=None):
    basename_index = client_basename_index(client_root)
    components = [
        discover_component(root, client_root, basename_index, c)
        for c in profile.get("components", [])
    ]

    workspace_rows = {}
    workspace_manifest = None
    if workspace:
        mp = workspace / "workspace_manifest.json"
        if mp.is_file():
            workspace_manifest = load_json(mp)
            for row in workspace_manifest.get("files", []):
                target = norm_rel(row.get("roundtrip_target", "")).lower()
                if target:
                    workspace_rows[target] = row

    for c in components:
        target = c.get("roundtrip_target")
        wrow = workspace_rows.get(norm_rel(target).lower()) if target else None
        c["workspace_registered"] = bool(wrow)
        c["workspace_asset_id"] = wrow.get("asset_id") if wrow else None
        c["workspace_registered_path"] = wrow.get("workspace_path") if wrow else None

    missing_required = [c["role"] for c in components if c["status"] == "MISSING_REQUIRED"]
    model = next((c for c in components if c["role"] == "model"), None)

    # Protect the geometry baseline used by the profile.
    model_baseline_ok = True
    if model and profile.get("model_smd_sha256"):
        baseline = profile["model_smd_sha256"].lower()
        local_model = resolve(root, profile["model_smd"])
        if not local_model.is_file() or sha256_file(local_model) != baseline:
            model_baseline_ok = False

    report = {
        "tool": "PT_CHARACTER_ASSET_V1",
        "character_asset_check": "PASS" if not missing_required and model_baseline_ok else "FAIL",
        "profile": str(profile_path),
        "profile_id": profile["profile_id"],
        "asset_set_id": profile.get("asset_set_id"),
        "display_name": profile.get("display_name"),
        "owner_name": profile.get("owner_name"),
        "class_code": profile.get("class_code"),
        "client_root": str(client_root),
        "workspace": str(workspace) if workspace else None,
        "model_baseline_ok": model_baseline_ok,
        "lod_groups": profile.get("lod_groups", {}),
        "geometry_status": profile.get("geometry_status", {}),
        "components": components,
        "missing_required_roles": missing_required,
        "timestamp": datetime.now(timezone.utc).isoformat(),
    }
    return report


def default_asset_manifest(root, profile):
    return root / "logs/characters" / profile["profile_id"] / "character_asset_manifest.json"


def run_checked(cmd):
    print("COMMAND=" + " ".join(str(x) for x in cmd))
    subprocess.run(cmd, check=True)


def cmd_inspect(args):
    root = args.root.resolve()
    pp = resolve(root, args.profile)
    profile = load_json(pp)
    client = resolve(root, args.client_root)
    workspace = resolve(root, args.workspace) if args.workspace else None

    report = inspect_asset(root, pp, profile, client, workspace)
    out = resolve(root, args.out) if args.out else default_asset_manifest(root, profile)
    write_json(out, report)

    print("PT_CHARACTER_ASSET_V1")
    print("action=inspect")
    print(f"profile_id={profile['profile_id']}")
    print(f"asset_set_id={profile.get('asset_set_id')}")
    print(f"component_count={len(report['components'])}")
    for c in report["components"]:
        print(
            f"COMPONENT role={c['role']} status={c['status']} "
            f"resolution={c['resolution']} "
            f"target={c.get('roundtrip_target')} "
            f"asset_id={c.get('asset_id')} "
            f"workspace_registered={c['workspace_registered']}"
        )
    print(f"model_baseline_ok={report['model_baseline_ok']}")
    print(f"manifest={out}")
    print(f"character_asset_check={report['character_asset_check']}")
    if report["character_asset_check"] != "PASS":
        raise SystemExit(2)


def cmd_sync_workspace(args):
    root = args.root.resolve()
    pp = resolve(root, args.profile)
    profile = load_json(pp)
    client = resolve(root, args.client_root)
    workspace = resolve(root, args.workspace)

    report = inspect_asset(root, pp, profile, client, workspace)
    if report["character_asset_check"] != "PASS":
        raise RuntimeError("character asset inspect did not PASS")

    bridge = root / "summit_workspace_asset_bridge_v1_1.py"
    if not bridge.is_file():
        raise FileNotFoundError(bridge)

    registered = 0
    skipped = 0
    for c in report["components"]:
        target = c.get("roundtrip_target")
        if not target:
            print(f"SKIP role={c['role']} reason=NO_RESOLVED_CLIENT_TARGET")
            skipped += 1
            continue
        if c.get("workspace_registered"):
            print(
                f"SKIP role={c['role']} target={target} "
                "reason=ALREADY_REGISTERED"
            )
            skipped += 1
            continue
        if not c.get("client_path"):
            print(
                f"SKIP role={c['role']} target={target} "
                "reason=CLIENT_FILE_NOT_FOUND"
            )
            skipped += 1
            continue

        cmd = [
            sys.executable, str(bridge), "register",
            str(workspace),
            "--game-root", str(client),
            "--target", target,
            "--workspace-path", c["workspace_path"],
            "--category", c["category"],
            "--subcategory", c["subcategory"],
            "--representation", c["representation"],
            "--owner", profile.get("owner_name", ""),
            "--class-code", profile.get("class_code", ""),
        ]
        run_checked(cmd)
        registered += 1

    final = inspect_asset(root, pp, profile, client, workspace)
    out = default_asset_manifest(root, profile)
    write_json(out, final)

    print("PT_CHARACTER_ASSET_V1")
    print("action=sync-workspace")
    print(f"registered={registered}")
    print(f"skipped={skipped}")
    print(f"manifest={out}")
    print(f"character_asset_check={final['character_asset_check']}")


def cmd_build(args):
    root = args.root.resolve()
    pp = resolve(root, args.profile)
    profile = load_json(pp)
    pipeline = root / "pt_character_multi_object_lod_pipeline_v2.py"
    if not pipeline.is_file():
        raise FileNotFoundError(pipeline)

    cmd = [
        sys.executable, str(pipeline),
        "--root", str(root),
        "--profile", str(pp),
        "--artist", args.artist,
        "--all-lods",
    ]
    if args.name:
        cmd += ["--name", args.name]
    if args.overwrite:
        cmd.append("--overwrite")
    if args.force_test:
        print(
            "WARNING: --force-test is engineering-only; visual deformation "
            "is the artist's responsibility."
        )
        cmd.append("--force-test")
    if args.client_root:
        cmd += ["--client-root", args.client_root]
    if args.publish:
        if not args.workspace:
            raise ValueError("--publish requires --workspace")
        cmd += [
            "--roundtrip-workspace", args.workspace,
            "--roundtrip-target", profile["roundtrip_target"],
        ]
        if args.replace_published:
            cmd.append("--replace-published")

    run_checked(cmd)


def cmd_deploy(args, apply):
    root = args.root.resolve()
    pp = resolve(root, args.profile)
    profile = load_json(pp)
    restore = root / "summit_roundtrip_restore_v3.py"
    if not restore.is_file():
        raise FileNotFoundError(restore)
    workspace = resolve(root, args.workspace)

    cmd = [
        sys.executable, str(restore),
        str(workspace),
        "--target", profile["roundtrip_target"],
    ]
    if apply:
        cmd.append("--apply")
    run_checked(cmd)


def parser():
    default_root = pathlib.Path(__file__).resolve().parent
    p = argparse.ArgumentParser()
    p.add_argument("--root", type=pathlib.Path, default=default_root)
    p.add_argument(
        "--profile",
        default="profiles/pt_character_profile_pikeman_c01_m5_v2.json",
    )
    sub = p.add_subparsers(dest="command", required=True)

    i = sub.add_parser("inspect")
    i.add_argument("--client-root", required=True)
    i.add_argument("--workspace")
    i.add_argument("--out")
    i.set_defaults(func=cmd_inspect)

    s = sub.add_parser("sync-workspace")
    s.add_argument("--client-root", required=True)
    s.add_argument("--workspace", required=True)
    s.set_defaults(func=cmd_sync_workspace)

    b = sub.add_parser("build")
    b.add_argument("--artist", required=True)
    b.add_argument("--name")
    b.add_argument("--client-root")
    b.add_argument("--workspace")
    b.add_argument("--publish", action="store_true")
    b.add_argument("--replace-published", action="store_true")
    b.add_argument("--force-test", action="store_true")
    b.add_argument("--overwrite", action="store_true")
    b.set_defaults(func=cmd_build)

    dp = sub.add_parser("deploy-plan")
    dp.add_argument("--workspace", required=True)
    dp.set_defaults(func=lambda a: cmd_deploy(a, False))

    d = sub.add_parser("deploy")
    d.add_argument("--workspace", required=True)
    d.set_defaults(func=lambda a: cmd_deploy(a, True))

    return p


def main():
    args = parser().parse_args()
    args.func(args)


if __name__ == "__main__":
    try:
        main()
    except SystemExit:
        raise
    except Exception as exc:
        print(f"character_asset_check=FAIL: {type(exc).__name__}: {exc}")
        raise

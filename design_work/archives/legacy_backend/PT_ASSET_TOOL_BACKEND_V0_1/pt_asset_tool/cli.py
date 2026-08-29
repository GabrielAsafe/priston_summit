from __future__ import annotations
import argparse, json, os, pathlib, shutil, subprocess, sys
from . import __version__
from .certification import CertificationRegistry
from .config import Settings
from .registry import AssetRegistry
from .resolver import ClientResolver
from .validation import ValidationContext, ValidationRunner


def make_context(a):
    s = Settings.discover(getattr(a, "root", None), getattr(a, "client_root", None))
    reg = AssetRegistry(s.inventory, s.pairing)
    cert = CertificationRegistry(s.certifications_dir)
    resolver = ClientResolver(s.client_root)
    return s, reg, cert, resolver


def table(rows, headers):
    widths = [len(h) for h in headers]
    for row in rows:
        for i, val in enumerate(row):
            widths[i] = min(80, max(widths[i], len(str(val))))
    print("  ".join(h.ljust(widths[i]) for i, h in enumerate(headers)))
    print("  ".join("-" * widths[i] for i in range(len(headers))))
    for row in rows:
        vals = []
        for i, val in enumerate(row):
            s = str(val)
            if len(s) > widths[i]:
                s = s[:max(1, widths[i]-1)] + "…"
            vals.append(s.ljust(widths[i]))
        print("  ".join(vals))


def cmd_doctor(a):
    s, reg, cert, resolver = make_context(a)
    checks = []
    def add(name, ok, detail): checks.append((name, "PASS" if ok else "FAIL", detail))
    add("project_root", s.project_root.is_dir(), s.project_root)
    add("inventory", s.inventory.is_file(), s.inventory)
    add("pairing", s.pairing.is_file(), s.pairing)
    add("canonical_records", len(reg.records) > 0, len(reg.records))
    required = [
        "pt_character_structural_gate_v2.py", "pt_inx_motion_core_v2.py",
        "pt_smd_static_capability_probe_v1.py", "pt_smd_blender_import_v4_generic.py",
        "history_py/pt_smd_blender_import_v3_static_armature.py",
        "pt_asset_tool/blender/witness_geometry.py",
        "summit_workspace_asset_bridge_v1_2.py", "summit_roundtrip_restore_v4.py",
    ]
    for rel in required:
        add(rel, (s.project_root / rel).is_file(), s.project_root / rel)
    add("client_root", s.client_root.is_dir(), s.client_root)
    add("python", bool(shutil.which("python3")), shutil.which("python3") or "not found")
    bc = s.blender_command()
    if bc and bc[0] == "flatpak":
        ok = bool(shutil.which("flatpak"))
        detail = " ".join(bc)
        if ok and "org.blender.Blender" in bc:
            probe = subprocess.run(["flatpak", "info", "org.blender.Blender"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            ok = probe.returncode == 0
    else:
        ok = bool(bc and shutil.which(bc[0]))
        detail = " ".join(bc)
    add("blender_command", ok, detail)
    table(checks, ["CHECK", "STATUS", "DETAIL"])
    failed = [x for x in checks if x[1] == "FAIL"]
    print(f"\nassets={len(reg.assets)} canonical_records={len(reg.records)} certifications={len(cert.records())}")
    if reg.duplicate_relpaths:
        print(f"doctor_note=canonical case-only relative-path pairs: {len(reg.duplicate_relpaths)} (exact-case resolver enabled)")
    print(f"doctor_check={'PASS' if not failed else 'FAIL'}")
    return 0 if not failed else 2


def cmd_assets_list(a):
    s, reg, cert, resolver = make_context(a)
    assets = reg.list_assets(category=a.category, family=a.family, extension=a.ext, visual_only=a.visual_only)
    latest = cert.latest()
    rows = []
    for asset in assets[:a.limit]:
        visual = latest.get((asset.asset_id, "VISUAL_GEOMETRY", "HUMAN_VISUAL"), {}).get("result", "PENDING")
        structural = latest.get((asset.asset_id, "STRUCTURAL", "AUTOMATIC"), {}).get("result", "PENDING")
        rows.append((asset.asset_id, asset.category, asset.family, asset.binding_mode, structural, visual, asset.primary_path))
    table(rows, ["ID", "CATEGORY", "FAMILY", "BINDING", "STRUCT", "VISUAL", "PATH"])
    print(f"\nshown={len(rows)} matching={len(assets)}")
    return 0


def cmd_families(a):
    s, reg, cert, resolver = make_context(a)
    table([(fam, n) for fam, n in reg.family_counts()], ["FAMILY", "ASSETS"])
    return 0


def cmd_asset_inspect(a):
    s, reg, cert, resolver = make_context(a)
    asset = reg.resolve(a.target)
    data = asset.to_dict()
    data["certifications"] = list(cert.latest(asset.asset_id).values())
    if a.physical:
        if not s.client_root.is_dir():
            data["physical_error"] = f"client root missing: {s.client_root}"
        else:
            try:
                data["physical_path"] = str(resolver.resolve(asset.primary_path))
                data["config_physical"] = [str(resolver.resolve(x["inx"])) for x in asset.configs]
                data["motion_physical"] = [str(resolver.resolve(x)) for x in asset.motions]
            except Exception as exc:
                data["physical_error"] = f"{type(exc).__name__}: {exc}"
    print(json.dumps(data, ensure_ascii=False, indent=2))
    return 0


def choose_next(reg, cert, category=None):
    priority = {"MOB_MONSTER": 0, "NPC": 1, "CHARACTER": 2, "WEAPON_ITEM": 3, "EFFECT": 4, "MAP_STAGE": 5, "UNKNOWN": 9}
    candidates = reg.list_assets(category=category, visual_only=True)
    candidates.sort(key=lambda x: (priority.get(x.category, 8), x.family, x.primary_path.lower()))
    for asset in candidates:
        if cert.latest_result(asset.asset_id, "VISUAL_GEOMETRY", "HUMAN_VISUAL") != "PASS":
            return asset
    raise RuntimeError("no pending visual-compatible assets match the selection")


def cmd_validate_structural(a):
    s, reg, cert, resolver = make_context(a)
    asset = reg.resolve(a.target)
    runner = ValidationRunner(ValidationContext(s, resolver, cert))
    result = runner.structural(asset)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0


def cmd_validate_visual(a):
    s, reg, cert, resolver = make_context(a)
    asset = choose_next(reg, cert, a.category) if a.target.lower() == "next" else reg.resolve(a.target)
    print(f"ASSET={asset.asset_id} {asset.primary_path}")
    print(f"FAMILY={asset.family} CATEGORY={asset.category} MODE={asset.visual_mode}")
    runner = ValidationRunner(ValidationContext(s, resolver, cert))
    result = runner.visual_geometry(asset, lod=a.lod)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result["human_visual"] != "FAIL" else 3


def cmd_cert_list(a):
    s, reg, cert, resolver = make_context(a)
    rows = cert.records()
    if a.asset:
        asset = reg.resolve(a.asset)
        rows = [x for x in rows if x.get("asset_id") == asset.asset_id]
    if a.latest:
        latest = {}
        for r in rows:
            latest[(r.get("asset_id"), r.get("test"), r.get("layer"))] = r
        rows = list(latest.values())
    rows = rows[-a.limit:]
    table([(r.get("timestamp",""), r.get("asset_id",""), r.get("test",""), r.get("layer",""), r.get("result",""), r.get("asset_path","")) for r in rows], ["TIME", "ID", "TEST", "LAYER", "RESULT", "PATH"])
    return 0


def build_parser():
    p = argparse.ArgumentParser(prog="pt_asset", description="Summit Tale asset registry and validation backend")
    p.add_argument("--version", action="version", version=__version__)
    p.add_argument("--root", help="design_work root (default: package parent or PT_PROJECT_ROOT)")
    p.add_argument("--client-root", help="client root (default: PT_CLIENT_ROOT or standard Summit path)")
    sp = p.add_subparsers(dest="command", required=True)

    x = sp.add_parser("doctor"); x.set_defaults(func=cmd_doctor)
    x = sp.add_parser("families"); x.set_defaults(func=cmd_families)

    assets = sp.add_parser("assets"); asp = assets.add_subparsers(dest="assets_cmd", required=True)
    x = asp.add_parser("list"); x.add_argument("--category"); x.add_argument("--family"); x.add_argument("--ext"); x.add_argument("--visual-only", action="store_true"); x.add_argument("--limit", type=int, default=50); x.set_defaults(func=cmd_assets_list)

    asset = sp.add_parser("asset"); ap = asset.add_subparsers(dest="asset_cmd", required=True)
    x = ap.add_parser("inspect"); x.add_argument("target"); x.add_argument("--physical", action="store_true"); x.set_defaults(func=cmd_asset_inspect)

    val = sp.add_parser("validate"); vp = val.add_subparsers(dest="val_cmd", required=True)
    x = vp.add_parser("structural"); x.add_argument("target"); x.set_defaults(func=cmd_validate_structural)
    x = vp.add_parser("visual"); x.add_argument("target", help="asset id/path, or 'next'"); x.add_argument("--category"); x.add_argument("--lod", choices=("high","default","low"), default="default"); x.set_defaults(func=cmd_validate_visual)

    cert = sp.add_parser("certification"); cp = cert.add_subparsers(dest="cert_cmd", required=True)
    x = cp.add_parser("list"); x.add_argument("--asset"); x.add_argument("--latest", action="store_true"); x.add_argument("--limit", type=int, default=100); x.set_defaults(func=cmd_cert_list)
    return p


def main(argv=None):
    p = build_parser()
    a = p.parse_args(argv)
    try:
        return int(a.func(a) or 0)
    except KeyboardInterrupt:
        print("cancelled", file=sys.stderr)
        return 130
    except Exception as exc:
        print(f"pt_asset_error={type(exc).__name__}: {exc}", file=sys.stderr)
        return 2

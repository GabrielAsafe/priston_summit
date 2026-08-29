from __future__ import annotations
import argparse, collections, json, os, pathlib, shutil, subprocess, sys
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
        "pt_asset_tool/blender/bake_actions.py", "pt_asset_tool/motion_core.py",
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
        geometry = latest.get((asset.asset_id, "VISUAL_GEOMETRY", "HUMAN_VISUAL"), {}).get("result", "PENDING")
        texture = latest.get((asset.asset_id, "VISUAL_TEXTURE_MATERIAL", "HUMAN_VISUAL"), {}).get("result", "PENDING") if asset.textures else "N/A"
        visual = latest.get((asset.asset_id, "VISUAL_ASSET", "HUMAN_VISUAL"), {}).get("result", "PENDING")
        animation = latest.get((asset.asset_id, "VISUAL_ANIMATION_ASSET", "HUMAN_VISUAL"), {}).get("result", "PENDING") if asset.visual_mode == "RIGID" else "N/A"
        structural = latest.get((asset.asset_id, "STRUCTURAL", "AUTOMATIC"), {}).get("result", "PENDING")
        rows.append((asset.asset_id, asset.category, asset.family, asset.binding_mode, structural, geometry, texture, visual, animation, asset.primary_path))
    table(rows, ["ID", "CATEGORY", "FAMILY", "BINDING", "STRUCT", "GEOM", "TEX/MAT", "VISUAL", "ANIM", "PATH"])
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
                data["binding_root_model_physical"] = [str(resolver.resolve(x)) for x in asset.binding_root_models]
            except Exception as exc:
                data["physical_error"] = f"{type(exc).__name__}: {exc}"
    print(json.dumps(data, ensure_ascii=False, indent=2))
    return 0


def choose_next(reg, cert, category=None):
    priority = {"MOB_MONSTER": 0, "NPC": 1, "CHARACTER": 2, "WEAPON_ITEM": 3, "EFFECT": 4, "MAP_STAGE": 5, "UNKNOWN": 9}
    candidates = reg.list_assets(category=category, visual_only=True)
    candidates.sort(key=lambda x: (priority.get(x.category, 8), x.family, x.primary_path.lower()))
    for asset in candidates:
        if cert.latest_result(asset.asset_id, "VISUAL_ASSET", "HUMAN_VISUAL") != "PASS":
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
    return 0 if result["visual_asset"] != "FAIL" else 3


def cmd_asset_motions(a):
    s, reg, cert, resolver = make_context(a)
    asset = reg.resolve(a.target)
    if asset.visual_mode != "RIGID":
        raise RuntimeError(f"motion listing requires an unambiguous rigid skeleton binding; got {asset.family}/{asset.binding_mode}")
    from .motion_core import load_motion_system, enrich_motions
    cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
    inx = resolver.resolve(cfg["inx"]); smb_path = resolver.resolve(cfg["motion_smb"])
    evaluator, inxcore = load_motion_system(s.project_root)
    smb = evaluator.parse_smb(smb_path); parsed = inxcore.parse_inx(inx); motions = enrich_motions(parsed, smb)
    rows=[]
    for m in motions:
        if not m.get("active_container"): continue
        rows.append((m["index"],m["state_name"],m["start"],m["end"],m["repeat"],m["motion_frame"]))
    table(rows,["INDEX","STATE","START","END","REPEAT","TMFRAME"])
    print(f"\nactive={len(rows)} motion_count={parsed['motion_count']}")
    return 0

def cmd_validate_animation(a):
    s, reg, cert, resolver = make_context(a)
    asset = choose_next_animation(reg, cert, a.category) if a.target.lower() == "next" else reg.resolve(a.target)
    print(f"ASSET={asset.asset_id} {asset.primary_path}")
    print(f"FAMILY={asset.family} CATEGORY={asset.category} MODE={asset.visual_mode}")
    runner = ValidationRunner(ValidationContext(s, resolver, cert))
    result = runner.visual_animation(asset, lod=a.lod, motion=a.motion, load_actions=a.load_actions, fps=a.fps)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result["visual_animation_asset"] != "FAIL" else 3

def choose_next_animation(reg, cert, category=None):
    candidates=[x for x in reg.list_assets(category=category, visual_only=True) if x.visual_mode=="RIGID"]
    candidates.sort(key=lambda x:(x.category,x.primary_path.lower()))
    for asset in candidates:
        if cert.latest_result(asset.asset_id,"VISUAL_ANIMATION_ASSET","HUMAN_VISUAL") != "PASS":
            return asset
    raise RuntimeError("no pending animation-compatible assets match the selection")


def cmd_diagnose_animation(a):
    s, reg, cert, resolver = make_context(a)
    asset = reg.resolve(a.target)
    print(f"ASSET={asset.asset_id} {asset.primary_path}")
    print(f"FAMILY={asset.family} CATEGORY={asset.category} MODE={asset.visual_mode}")
    runner = ValidationRunner(ValidationContext(s, resolver, cert))
    result = runner.animation_diagnostic(asset, lod=a.lod, motion=a.motion, load_actions=a.load_actions, fps=a.fps)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0

def cmd_diagnose_motion_source(a):
    s, reg, cert, resolver = make_context(a)
    asset = reg.resolve(a.target)
    if asset.visual_mode != "RIGID":
        raise RuntimeError(f"source motion audit requires rigid asset; got {asset.family}/{asset.binding_mode}")
    from .motion_core import audit_motion_source
    cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
    inx = resolver.resolve(cfg["inx"]); smb_path = resolver.resolve(cfg["motion_smb"])
    print(f"ASSET={asset.asset_id} {asset.primary_path}")
    print(f"FAMILY={asset.family} CATEGORY={asset.category} MODE={asset.visual_mode}")
    result = audit_motion_source(s.project_root, inx, smb_path, selector=a.motion)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0

def cmd_diagnose_motion_segment(a):
    s, reg, cert, resolver = make_context(a)
    asset = reg.resolve(a.target)
    if asset.visual_mode != "RIGID":
        raise RuntimeError(f"motion segment audit requires rigid asset; got {asset.family}/{asset.binding_mode}")
    from .motion_core import audit_motion_segments
    cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
    inx = resolver.resolve(cfg["inx"]); smb_path = resolver.resolve(cfg["motion_smb"])
    print(f"ASSET={asset.asset_id} {asset.primary_path}")
    print(f"FAMILY={asset.family} CATEGORY={asset.category} MODE={asset.visual_mode}")
    result = audit_motion_segments(s.project_root, inx, smb_path, selector=a.motion)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0


def cmd_bindings(a):
    s, reg, cert, resolver = make_context(a)
    assets = [x for x in reg.assets if x.extension == ".smd" and x.family == "SMD_MODEL_062_RIGID_PHYSIQUE"]
    if a.category:
        assets = [x for x in assets if x.category.lower() == a.category.lower()]
    if a.mode:
        assets = [x for x in assets if x.binding_mode.lower() == a.mode.lower()]
    interesting = [x for x in assets if x.binding_mode in {"ROOT_SKELETON_BOUND", "CONFIG_ONLY", "AMBIGUOUS_MULTIPLE_MOTIONS"}]
    rows=[]
    for asset in interesting[:a.limit]:
        roots = ",".join(asset.binding_root_models[:3])
        if len(asset.binding_root_models) > 3:
            roots += f" (+{len(asset.binding_root_models)-3})"
        motion = asset.motions[0] if len(asset.motions) == 1 else (f"{len(asset.motions)} motions" if asset.motions else "-")
        rows.append((asset.asset_id, asset.category, asset.binding_mode, motion, roots or "-", asset.primary_path))
    table(rows, ["ID","CATEGORY","BINDING","MOTION","ROOT MODEL(S)","PATH"])
    counts = collections.Counter(x.binding_mode for x in assets)
    print("\n" + " ".join(f"{k}={v}" for k,v in sorted(counts.items())))
    print(f"shown={len(rows)} interesting={len(interesting)} rigid_assets={len(assets)}")
    return 0

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


def cmd_cert_record(a):
    s, reg, cert, resolver = make_context(a)
    asset = reg.resolve(a.target)
    src = resolver.resolve(asset.primary_path, required=False) if s.client_root.is_dir() else None
    source_sha = None
    if src and src.is_file():
        from .utils import sha256_file
        source_sha = sha256_file(src)
    row = cert.add(
        asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
        test=a.test, layer=a.layer, result=a.result, source_sha256=source_sha, notes=a.notes or "",
        metadata={"manual_record": True},
    )
    print(json.dumps(row, ensure_ascii=False, indent=2))
    return 0


def build_parser():
    p = argparse.ArgumentParser(prog="pt_asset", description="Summit Tale asset registry and validation backend")
    p.add_argument("--version", action="version", version=__version__)
    p.add_argument("--root", help="design_work root (default: package parent or PT_PROJECT_ROOT)")
    p.add_argument("--client-root", help="client root (default: PT_CLIENT_ROOT or standard Summit path)")
    sp = p.add_subparsers(dest="command", required=True)

    x = sp.add_parser("doctor"); x.set_defaults(func=cmd_doctor)
    x = sp.add_parser("families"); x.set_defaults(func=cmd_families)
    x = sp.add_parser("bindings"); x.add_argument("--category"); x.add_argument("--mode"); x.add_argument("--limit", type=int, default=100); x.set_defaults(func=cmd_bindings)

    assets = sp.add_parser("assets"); asp = assets.add_subparsers(dest="assets_cmd", required=True)
    x = asp.add_parser("list"); x.add_argument("--category"); x.add_argument("--family"); x.add_argument("--ext"); x.add_argument("--visual-only", action="store_true"); x.add_argument("--limit", type=int, default=50); x.set_defaults(func=cmd_assets_list)

    asset = sp.add_parser("asset"); ap = asset.add_subparsers(dest="asset_cmd", required=True)
    x = ap.add_parser("inspect"); x.add_argument("target"); x.add_argument("--physical", action="store_true"); x.set_defaults(func=cmd_asset_inspect)
    x = ap.add_parser("motions"); x.add_argument("target"); x.set_defaults(func=cmd_asset_motions)

    val = sp.add_parser("validate"); vp = val.add_subparsers(dest="val_cmd", required=True)
    x = vp.add_parser("structural"); x.add_argument("target"); x.set_defaults(func=cmd_validate_structural)
    x = vp.add_parser("visual"); x.add_argument("target", help="asset id/path, or 'next'"); x.add_argument("--category"); x.add_argument("--lod", choices=("auto","high","default","low"), default="auto"); x.set_defaults(func=cmd_validate_visual)
    x = vp.add_parser("animation"); x.add_argument("target", help="asset id/path, or 'next'"); x.add_argument("--category"); x.add_argument("--lod", choices=("auto","high","default","low"), default="auto"); x.add_argument("--motion", default="auto", help="auto, state name (STAND/WALK/...), or motion index"); x.add_argument("--load-actions", choices=("selected","core","all"), default="core"); x.add_argument("--fps", type=int, default=30); x.set_defaults(func=cmd_validate_animation)

    diag = sp.add_parser("diagnose"); dp = diag.add_subparsers(dest="diag_cmd", required=True)
    x = dp.add_parser("animation"); x.add_argument("target", help="asset id/path"); x.add_argument("--lod", choices=("auto","high","default","low"), default="auto"); x.add_argument("--motion", default="ATTACK", help="state name or motion index"); x.add_argument("--load-actions", choices=("selected","core","all"), default="selected"); x.add_argument("--fps", type=int, default=30); x.set_defaults(func=cmd_diagnose_animation)
    x = dp.add_parser("motion-source"); x.add_argument("target", help="asset id/path"); x.add_argument("--motion", default="STAND", help="state name or motion index"); x.set_defaults(func=cmd_diagnose_motion_source)
    x = dp.add_parser("motion-segment"); x.add_argument("target", help="asset id/path"); x.add_argument("--motion", default="STAND", help="state name or motion index"); x.set_defaults(func=cmd_diagnose_motion_segment)

    cert = sp.add_parser("certification"); cp = cert.add_subparsers(dest="cert_cmd", required=True)
    x = cp.add_parser("list"); x.add_argument("--asset"); x.add_argument("--latest", action="store_true"); x.add_argument("--limit", type=int, default=100); x.set_defaults(func=cmd_cert_list)
    x = cp.add_parser("record"); x.add_argument("target"); x.add_argument("--test", required=True); x.add_argument("--layer", default="HUMAN_VISUAL"); x.add_argument("--result", required=True, choices=("PASS","FAIL","SKIP")); x.add_argument("--notes"); x.set_defaults(func=cmd_cert_record)
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

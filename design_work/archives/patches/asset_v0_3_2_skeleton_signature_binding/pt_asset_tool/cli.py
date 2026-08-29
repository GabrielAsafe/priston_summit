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
    motion_inx = cfg.get("motion_inx") or (cfg.get("inx") if asset.binding_source not in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE"} else "")
    if not motion_inx:
        raise RuntimeError("root skeleton is structurally bound, but root motion INX is not uniquely resolved")
    inx = resolver.resolve(motion_inx); smb_path = resolver.resolve(cfg["motion_smb"])
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


def _load_root_binding_cache(path: pathlib.Path) -> dict:
    if path.is_file():
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
            if isinstance(data, dict):
                data.setdefault("format", "PT_ROOT_SKELETON_BINDINGS_V2")
                data.setdefault("entries", {})
                return data
        except Exception:
            pass
    return {"format": "PT_ROOT_SKELETON_BINDINGS_V2", "entries": {}}


def _atomic_json(path: pathlib.Path, data: dict):
    from .utils import atomic_write_json
    atomic_write_json(path, data)


def _load_smb_signature_index(path: pathlib.Path) -> dict:
    if path.is_file():
        try:
            data=json.loads(path.read_text(encoding="utf-8"))
            if isinstance(data, dict):
                data.setdefault("format", "PT_SMB_SKELETON_SIGNATURE_INDEX_V1")
                data.setdefault("entries", {})
                return data
        except Exception:
            pass
    return {"format":"PT_SMB_SKELETON_SIGNATURE_INDEX_V1","entries":{}}


def _build_smb_skeleton_groups(s, reg, resolver, *, candidate_limit=0):
    """Group physical SMB files by embedded bone hierarchy + bind pose.

    This is deliberately independent of SMB file SHA: multiple animation SMBs
    may carry the same skeleton with different clip payloads.
    """
    from .utils import low_path, norm_path, utc_now
    from .motion_core import load_smb_evaluator, smb_skeleton_signature

    candidates=[]; seen=set()
    for r in reg.records:
        if str(r.get("extension") or "").lower() != ".smb":
            continue
        rel=norm_path(r.get("relative_path")); k=low_path(rel)
        if rel and k not in seen:
            seen.add(k); candidates.append(rel)
    candidates.sort(key=str.lower)

    index_path = reg.pairing_path.parent / "smb_skeleton_signatures_v1.json"
    index = _load_smb_signature_index(index_path)
    entries=index.setdefault("entries", {})
    evaluator=None
    cache_hits=0; parsed=0; parse_failures=0
    groups={}

    for pos, rel in enumerate(candidates, 1):
        physical=resolver.resolve(rel, required=False)
        if physical is None:
            continue
        st=physical.stat(); key=low_path(rel)
        row=entries.get(key) if isinstance(entries.get(key), dict) else None
        valid=bool(row and row.get("size") == st.st_size and row.get("mtime_ns") == st.st_mtime_ns and row.get("skeleton_signature"))
        if valid:
            info={k:row.get(k) for k in ("skeleton_signature","topology_signature","bone_count","root_bones")}
            cache_hits += 1
        else:
            if evaluator is None:
                evaluator=load_smb_evaluator(s.project_root)
            try:
                smb=evaluator.parse_smb(physical)
                info=smb_skeleton_signature(smb)
                entries[key]={"path":rel,"size":st.st_size,"mtime_ns":st.st_mtime_ns,**info}
                parsed += 1
            except Exception as exc:
                entries[key]={"path":rel,"size":st.st_size,"mtime_ns":st.st_mtime_ns,"error":f"{type(exc).__name__}: {exc}"}
                parse_failures += 1
                continue
        sig=info.get("skeleton_signature")
        if not sig:
            continue
        g=groups.setdefault(sig,{"skeleton_signature":sig,
                                "topology_signature":info.get("topology_signature"),
                                "bone_count":int(info.get("bone_count") or 0),
                                "root_bones":info.get("root_bones") or [],
                                "paths":[]})
        g["paths"].append(rel)

    index["updated_at"]=utc_now(); _atomic_json(index_path,index)
    ordered=[]
    for g in groups.values():
        g["paths"]=sorted(set(g["paths"]), key=str.lower)
        g["representative"]=g["paths"][0]
        ordered.append(g)
    ordered.sort(key=lambda g:g["representative"].lower())
    if candidate_limit and candidate_limit > 0:
        ordered=ordered[:candidate_limit]
    return {
        "smb_files":len(candidates), "skeleton_groups":ordered,
        "signature_index":str(index_path), "signature_cache_hits":cache_hits,
        "signature_parsed":parsed, "signature_parse_failures":parse_failures,
    }


def cmd_binding_discover(a, s, reg, resolver):
    from .utils import low_path, norm_path, sha256_file, stable_id, utc_now
    asset = reg.resolve(a.discover)
    if asset.family != "SMD_MODEL_062_RIGID_PHYSIQUE":
        raise RuntimeError(f"binding discovery requires rigid Physique SMD; got {asset.family}")
    if asset.binding_mode == "SELF_CONTAINED":
        raise RuntimeError("asset is already SELF_CONTAINED; discovery is only for config-only/root-bound assets")

    gate = s.project_root / "pt_character_structural_gate_v2.py"
    if not gate.is_file():
        raise FileNotFoundError(gate)
    src = resolver.resolve(asset.primary_path)
    source_sha = sha256_file(src)
    work = s.work_dir / asset.asset_id / "root_skeleton_discovery"
    reports = work / "reports_v2"
    reports.mkdir(parents=True, exist_ok=True)
    state_path = work / "probe_state_v2.json"
    state = {"format":"PT_ROOT_SKELETON_PROBE_STATE_V2", "asset":asset.primary_path,
             "source_sha256":source_sha, "tested_skeletons":{}, "passes":[]}
    if not a.no_resume and state_path.is_file():
        try:
            old=json.loads(state_path.read_text(encoding="utf-8"))
            if old.get("source_sha256") == source_sha and old.get("format") == "PT_ROOT_SKELETON_PROBE_STATE_V2":
                state=old
        except Exception:
            pass

    universe=_build_smb_skeleton_groups(s, reg, resolver, candidate_limit=a.candidate_limit)
    groups=universe["skeleton_groups"]
    print(f"smb_files={universe['smb_files']} skeleton_groups={len(groups)} "
          f"signature_cache_hits={universe['signature_cache_hits']} parsed={universe['signature_parsed']} "
          f"parse_failures={universe['signature_parse_failures']}")

    tested=state.setdefault("tested_skeletons", {})
    pass_by_sig={}
    for row in state.get("passes", []):
        if isinstance(row, dict) and row.get("skeleton_signature"):
            pass_by_sig[row["skeleton_signature"]]=row

    fresh=0; total=len(groups)
    for pos,g in enumerate(groups,1):
        sig=g["skeleton_signature"]
        if sig in tested:
            continue
        rel=g["representative"]
        smb=resolver.resolve(rel, required=False)
        if smb is None:
            tested[sig]={"representative":rel,"result":"MISSING"}
            continue
        rid=stable_id("skeleton-probe", sig).replace("AST-", "")
        rp=reports / f"{rid}.json"; lp=reports / f"{rid}.log"
        proc=subprocess.run(["python3", str(gate), "--smd", str(src), "--smb", str(smb), "--report", str(rp)],
                            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        lp.write_text(proc.stdout or "", encoding="utf-8")
        fresh += 1
        if proc.returncode == 0:
            row={"skeleton_signature":sig,"topology_signature":g.get("topology_signature"),
                 "bone_count":g.get("bone_count"),"root_bones":g.get("root_bones") or [],
                 "representative":rel,"smb_aliases":g["paths"],"report":str(rp)}
            tested[sig]={"representative":rel,"result":"PASS","report":str(rp),"smb_aliases":g["paths"]}
            pass_by_sig[sig]=row
            print(f"PASS skeleton={sig[:12]} bones={g.get('bone_count')} smb_aliases={len(g['paths'])} representative={rel}")
        else:
            tested[sig]={"representative":rel,"result":"FAIL"}
        if fresh % 25 == 0:
            state["passes"]=list(pass_by_sig.values()); _atomic_json(state_path,state)
            print(f"progress={pos}/{total} distinct_skeleton_passes={len(pass_by_sig)}")
        if len(pass_by_sig) >= 2 and not a.full_scan:
            break

    state["passes"]=sorted(pass_by_sig.values(), key=lambda r:r["representative"].lower())
    _atomic_json(state_path,state)

    cache_path = reg.binding_cache_path
    cache = _load_root_binding_cache(cache_path)
    entry={"asset":asset.primary_path,"source_sha256":source_sha,"updated_at":utc_now(),
           "tested_skeletons":len(tested),"distinct_skeleton_passes":len(pass_by_sig),
           "smb_files":universe["smb_files"],"skeleton_group_universe":total,
           "signature_index":universe["signature_index"],"probe_state":str(state_path)}
    if len(pass_by_sig) == 1 and len(tested) >= total:
        sig, winner = next(iter(pass_by_sig.items()))
        aliases=winner["smb_aliases"]
        alias_keys={low_path(x) for x in aliases}
        root_models=set(); root_inxs=set(); referenced_aliases=[]
        for row in reg.pairing:
            motion=norm_path(row.get("motion_smb"))
            if low_path(motion) not in alias_keys:
                continue
            if motion: referenced_aliases.append(motion)
            model=norm_path(row.get("model_smd")); inx=norm_path(row.get("inx"))
            if model and str(row.get("model_resolved") or "").lower() in {"1","true","yes","y"}:
                root_models.add(model)
            if inx: root_inxs.add(inx)
        canonical_motion=sorted(set(referenced_aliases), key=str.lower)[0] if referenced_aliases else winner["representative"]
        entry.update({"status":"UNIQUE_SKELETON","motion_smb":canonical_motion,
                      "motion_aliases":aliases,"skeleton_signature":sig,
                      "topology_signature":winner.get("topology_signature"),"bone_count":winner.get("bone_count"),
                      "root_bones":winner.get("root_bones") or [],
                      "root_models":sorted(root_models,key=str.lower),"root_inxs":sorted(root_inxs,key=str.lower),
                      "source":"SKELETON_SIGNATURE_UNIQUE"})
    elif len(pass_by_sig) >= 2:
        entry.update({"status":"AMBIGUOUS_SKELETON_MATCH","passes":state["passes"],"source":"SKELETON_SIGNATURE_GATE"})
    else:
        entry.update({"status":"NO_MATCH" if len(tested) >= total else "INCOMPLETE_SCAN",
                      "passes":state["passes"],"source":"SKELETON_SIGNATURE_GATE"})
    cache.setdefault("entries", {})[low_path(asset.primary_path)] = entry
    _atomic_json(cache_path, cache)
    print(json.dumps({"asset_id":asset.asset_id,"asset":asset.primary_path,**entry}, ensure_ascii=False, indent=2))
    if entry["status"] == "UNIQUE_SKELETON":
        print("binding_discovery=UNIQUE_SKELETON; rerun `python3 pt_asset.py asset inspect <asset> --physical`")
        return 0
    if entry["status"] == "AMBIGUOUS_SKELETON_MATCH":
        print("binding_discovery=AMBIGUOUS_SKELETON_MATCH (fail-closed)")
        return 4
    print(f"binding_discovery={entry['status']} (fail-closed)")
    return 3

def cmd_bindings(a):
    s, reg, cert, resolver = make_context(a)
    if getattr(a, "discover", None):
        return cmd_binding_discover(a, s, reg, resolver)
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
    x = sp.add_parser("bindings"); x.add_argument("--category"); x.add_argument("--mode"); x.add_argument("--limit", type=int, default=100); x.add_argument("--discover", metavar="ASSET", help="group physical SMBs by embedded skeleton signature, then probe one representative per skeleton"); x.add_argument("--candidate-limit", type=int, default=0, help="debug only: cap skeleton-signature groups; zero means all"); x.add_argument("--full-scan", action="store_true", help="continue even after two distinct skeleton signatures pass"); x.add_argument("--no-resume", action="store_true", help="ignore previous per-candidate probe state"); x.set_defaults(func=cmd_bindings)

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

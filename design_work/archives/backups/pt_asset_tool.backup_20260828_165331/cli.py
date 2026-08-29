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
        animation = latest.get((asset.asset_id, "VISUAL_ANIMATION_ASSET", "HUMAN_VISUAL"), {}).get("result", "PENDING") if asset.animation_binding_ready else ("CONTEXT" if asset.visual_mode == "RIGID" else "N/A")
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
                data["binding_skeleton_physical"] = str(resolver.resolve(asset.binding_skeleton_smb)) if asset.binding_skeleton_smb else ""
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
    if asset.visual_mode != "RIGID" or not asset.animation_binding_ready:
        raise RuntimeError(f"motion listing requires resolved motion context; skeleton binding={asset.binding_mode}/{asset.binding_source} animation_binding_ready={asset.animation_binding_ready}")
    from .motion_core import load_motion_system, enrich_motions
    cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
    motion_inx = cfg.get("motion_inx") or (cfg.get("inx") if asset.binding_source not in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE", "GATE_FIRST_SKELETON_UNIQUE", "PHYSIQUE_SUBSET_EQUIVALENT"} else "")
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
    candidates=[x for x in reg.list_assets(category=category, visual_only=True) if x.visual_mode=="RIGID" and x.animation_binding_ready]
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
    if asset.visual_mode != "RIGID" or not asset.animation_binding_ready:
        raise RuntimeError(f"source motion audit requires resolved motion context; binding={asset.binding_mode}/{asset.binding_source}")
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
    if asset.visual_mode != "RIGID" or not asset.animation_binding_ready:
        raise RuntimeError(f"motion segment audit requires resolved motion context; binding={asset.binding_mode}/{asset.binding_source}")
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


def _signature_for_passing_smb(s, smb_path: pathlib.Path) -> dict:
    """Best-effort skeleton identity for an SMB that already passed the structural gate.

    The crucial ordering is gate-first: parser coverage over unrelated SMB dialects
    must never shrink the structural candidate universe. A PASS that cannot be
    signature-parsed is kept as unresolved evidence and forces fail-closed.
    """
    from .motion_core import load_smb_evaluator, smb_skeleton_signature
    evaluator = load_smb_evaluator(s.project_root)
    smb = evaluator.parse_smb(smb_path)
    return smb_skeleton_signature(smb)


def _error_bucket(exc: Exception) -> str:
    msg=f"{type(exc).__name__}: {exc}"
    low=msg.casefold()
    if 'eof' in low or 'unexpected end' in low or 'unpack' in low:
        return 'TRUNCATED_OR_LAYOUT'
    if 'magic' in low or 'header' in low or 'signature' in low:
        return 'HEADER_OR_DIALECT'
    if 'decode' in low or 'unicode' in low:
        return 'TEXT_DECODE'
    if 'assert' in low:
        return 'PARSER_ASSERTION'
    return type(exc).__name__



def _physique_subset_resolution(a, s, reg, resolver, asset, src, tested, work):
    """Resolve full-skeleton ambiguity by the mesh's actually weighted bone closure.

    The legacy Blender importer is used once to recover the rigid Physique bone
    indices as vertex-group names. Every structural PASS SMB is then compared
    only on those bones + their ancestors. This does not use directory/name
    similarity as binding evidence.
    """
    from .motion_core import load_smb_evaluator, smb_physique_closure_signature
    from .texture_overlay import prepare_texture_overlay
    from .utils import low_path, norm_path

    pass_rows=[r for r in tested.values() if isinstance(r,dict) and r.get('result')=='PASS' and r.get('signature_status')=='PASS' and r.get('path')]
    if not pass_rows:
        return {'status':'PHYSIQUE_PROBE_NO_PASSES','source':'PHYSIQUE_SUBSET'}

    asset_dir=str(pathlib.PurePosixPath(asset.primary_path).parent).casefold()
    pass_rows.sort(key=lambda r:(0 if str(pathlib.PurePosixPath(r['path']).parent).casefold()==asset_dir else 1, str(r['path']).casefold()))
    representative=pass_rows[0]['path']
    rep_smb_path=resolver.resolve(representative)

    cfg=asset.configs[0] if asset.configs else None
    if not cfg or not cfg.get('inx'):
        return {'status':'PHYSIQUE_PROBE_NO_INX','source':'PHYSIQUE_SUBSET'}
    inx=resolver.resolve(cfg['inx'])
    groups=cfg.get('lod_groups') or {}
    lod=next((x for x in ('default','high','low') if groups.get(x)), None)
    if not lod:
        return {'status':'PHYSIQUE_PROBE_NO_LOD','source':'PHYSIQUE_SUBSET'}

    probe=work/'physique_subset_v1'
    probe.mkdir(parents=True,exist_ok=True)
    overlay, texture_report=prepare_texture_overlay(asset=asset,resolver=resolver,root=s.project_root,work=probe)
    blend=probe/'physique_probe.blend'
    import_log=probe/'01_import.log'
    cmd=s.blender_command()+[
        '--background','--python-exit-code','1',
        '--python',str(s.project_root/'history_py/pt_smd_blender_import_v3_static_armature.py'),'--',
        '--smd',str(src),'--skeleton',str(rep_smb_path),'--model-info',str(inx),
        '--lod',lod,'--client-root',str(overlay),'--out',str(blend),'--no-pack',
    ]
    proc=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    import_log.write_text(proc.stdout or '',encoding='utf-8')
    if proc.returncode or not blend.is_file():
        return {'status':'PHYSIQUE_PROBE_IMPORT_FAIL','source':'PHYSIQUE_SUBSET','representative_smb':representative,'log':str(import_log),'returncode':proc.returncode}

    usage_report=probe/'physique_usage.json'
    usage_log=probe/'02_usage.log'
    cmd=s.blender_command()+[
        '--background',str(blend),'--python-exit-code','1',
        '--python',str(s.project_root/'pt_asset_tool/blender/extract_physique_usage.py'),'--',
        '--report',str(usage_report),
    ]
    proc=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    usage_log.write_text(proc.stdout or '',encoding='utf-8')
    if proc.returncode or not usage_report.is_file():
        return {'status':'PHYSIQUE_USAGE_EXTRACT_FAIL','source':'PHYSIQUE_SUBSET','representative_smb':representative,'log':str(usage_log),'returncode':proc.returncode}

    usage=json.loads(usage_report.read_text(encoding='utf-8'))
    weighted=usage.get('weighted_groups') or []
    evaluator=load_smb_evaluator(s.project_root)
    rep_smb=evaluator.parse_smb(rep_smb_path)
    rep_names=[str(b.get('name') or '') for b in (rep_smb.get('bones') or [])]
    name_to_indices=collections.defaultdict(list)
    for i,n in enumerate(rep_names):
        if n: name_to_indices[n.casefold()].append(i)

    used_indices=[]; unmapped=[]; ambiguous_names=[]
    for row in weighted:
        name=str(row.get('name') or '')
        hits=name_to_indices.get(name.casefold(),[])
        if len(hits)==1:
            used_indices.append(hits[0])
        elif not hits:
            unmapped.append(name)
        else:
            ambiguous_names.append({'name':name,'indices':hits})
    used_indices=sorted(set(used_indices))
    if unmapped or ambiguous_names or not used_indices:
        return {'status':'PHYSIQUE_USAGE_UNMAPPED','source':'PHYSIQUE_SUBSET','representative_smb':representative,
                'weighted_groups':[r.get('name') for r in weighted],'used_bone_indices':used_indices,
                'unmapped_groups':unmapped,'ambiguous_group_names':ambiguous_names,'usage_report':str(usage_report)}

    subset_groups={}; topology_groups=collections.defaultdict(list); subset_failures=[]
    for row in pass_rows:
        rel=row['path']; physical=resolver.resolve(rel,required=False)
        if physical is None:
            subset_failures.append({'path':rel,'error':'MISSING'})
            continue
        try:
            smb=evaluator.parse_smb(physical)
            info=smb_physique_closure_signature(smb,used_indices)
            sig=info['physique_subset_signature']
            topo_sig=info['physique_topology_signature']
            topology_groups[topo_sig].append(rel)
            g=subset_groups.setdefault(sig,{'physique_subset_signature':sig,'physique_topology_signature':topo_sig,
                'used_bone_indices':info['used_bone_indices'],'used_bone_names':info['used_bone_names'],
                'closure_indices':info['closure_indices'],'closure_bone_names':info['closure_bone_names'],
                'closure_bone_count':info['closure_bone_count'],'smb_aliases':[]})
            g['smb_aliases'].append(rel)
        except Exception as exc:
            subset_failures.append({'path':rel,'error':f'{type(exc).__name__}: {exc}'})

    for g in subset_groups.values():
        g['smb_aliases']=sorted(set(g['smb_aliases']),key=str.lower)
    ordered=sorted(subset_groups.values(),key=lambda g:(-len(g['smb_aliases']),g['smb_aliases'][0].lower()))
    groups_report=probe/'physique_subset_groups.json'
    groups_report.write_text(json.dumps({'format':'PT_PHYSIQUE_SUBSET_GROUPS_V1','groups':ordered},ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    base={'source':'PHYSIQUE_SUBSET','representative_probe_smb':representative,'lod':lod,
          'weighted_group_count':len(weighted),'weighted_groups':[r.get('name') for r in weighted],
          'used_bone_indices':used_indices,'physique_usage_report':str(usage_report),
          'physique_probe_blend':str(blend),'physique_subset_count':len(ordered),'physique_topology_subset_count':len(topology_groups),
          'subset_parse_failures':subset_failures,'physique_subset_groups_report':str(groups_report),
          'physique_subset_group_sample':ordered[:12]}
    if subset_failures:
        return {'status':'PHYSIQUE_SUBSET_INCOMPLETE',**base}
    if len(ordered)==1:
        winner=ordered[0]
        aliases=winner['smb_aliases']
        # Representative is deterministic, but animation remains contextual.
        preferred=sorted(aliases,key=lambda rel:(0 if str(pathlib.PurePosixPath(rel).parent).casefold()==asset_dir else 1,rel.casefold()))[0]
        return {'status':'PHYSIQUE_SUBSET_EQUIVALENT','representative_smb':preferred,
                'equivalent_smb_count':len(aliases),'equivalent_smb_alias_sample':aliases[:20],
                'physique_subset_signature':winner['physique_subset_signature'],
                'closure_bone_names':winner['closure_bone_names'],'closure_bone_count':winner['closure_bone_count'],**base}
    return {'status':'AMBIGUOUS_PHYSIQUE_SUBSET',**base}

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
    reports = work / "reports_v5"
    reports.mkdir(parents=True, exist_ok=True)
    state_path = work / "probe_state_v5.json"
    state = {"format":"PT_ROOT_SKELETON_PROBE_STATE_V5", "asset":asset.primary_path,
             "source_sha256":source_sha, "tested":{}, "passes":[]}
    if not a.no_resume and state_path.is_file():
        try:
            old=json.loads(state_path.read_text(encoding="utf-8"))
            if old.get("source_sha256") == source_sha and old.get("format") == "PT_ROOT_SKELETON_PROBE_STATE_V5":
                state=old
        except Exception:
            pass

    # V0.3.5 migration: reuse the complete V0.3.4 structural+signature scan.
    # The new work is only the Physique weighted-bone closure probe.
    migrated_from_v4 = False
    if not state.get("tested") and not a.no_resume:
        v4_path = work / "probe_state_v4.json"
        if v4_path.is_file():
            try:
                v4=json.loads(v4_path.read_text(encoding="utf-8"))
                if v4.get("source_sha256") == source_sha and v4.get("format") == "PT_ROOT_SKELETON_PROBE_STATE_V4":
                    state["tested"]=v4.get("tested") or {}
                    migrated_from_v4=True
                    print(f"reused_v4_scan={len(state['tested'])}; physique_subset_will_be_computed")
            except Exception as exc:
                print(f"v4_state_reuse=SKIP ({type(exc).__name__})")
    # Older V3 can still be recovered if V4 was never produced. Its broken
    # signatures are stripped and recomputed below.
    migrated_from_v3 = False
    if not state.get("tested") and not a.no_resume:
        v3_path = work / "probe_state_v3.json"
        if v3_path.is_file():
            try:
                v3=json.loads(v3_path.read_text(encoding="utf-8"))
                if v3.get("source_sha256") == source_sha and v3.get("format") == "PT_ROOT_SKELETON_PROBE_STATE_V3":
                    imported={}
                    for k,row in (v3.get("tested") or {}).items():
                        if not isinstance(row,dict):
                            continue
                        keep={x:row.get(x) for x in ("path","result","sha256","report") if row.get(x) is not None}
                        imported[k]=keep
                    state["tested"]=imported
                    migrated_from_v3=True
                    print(f"reused_v3_structural_results={len(imported)}; signatures_will_be_recomputed")
            except Exception as exc:
                print(f"v3_state_reuse=SKIP ({type(exc).__name__})")

    # IMPORTANT: this is the complete physical SMB universe. We intentionally
    # do NOT pre-parse it with the animation evaluator. Structural gate decides
    # relevance first; skeleton signature is only requested for actual PASSes.
    candidates=[]; seen=set()
    for r in reg.records:
        if str(r.get("extension") or "").lower() != ".smb":
            continue
        rel=norm_path(r.get("relative_path")); k=low_path(rel)
        if rel and k not in seen:
            seen.add(k); candidates.append(rel)
    candidates.sort(key=str.lower)
    if a.candidate_limit and a.candidate_limit > 0:
        candidates=candidates[:a.candidate_limit]

    tested=state.setdefault("tested", {})
    total=len(candidates); fresh=0

    # Repair/re-sign already-proven structural PASSes. This is deliberately
    # independent from the gate loop: a parser/signature bug must never force
    # 958 expensive structural gates to run again.
    resign_total=sum(1 for row in tested.values() if isinstance(row,dict) and row.get("result")=="PASS" and not row.get("skeleton_signature"))
    resign_done=0; resign_fail=0
    if resign_total:
        print(f"resign_structural_passes={resign_total}")
    for k,row in tested.items():
        if not isinstance(row,dict) or row.get("result") != "PASS" or row.get("skeleton_signature"):
            continue
        rel=row.get("path")
        smb=resolver.resolve(rel, required=False) if rel else None
        if smb is None:
            row.update({"signature_status":"FAIL","signature_error_bucket":"MISSING","signature_error":"SMB missing during re-sign"})
            resign_fail += 1
            continue
        try:
            info=_signature_for_passing_smb(s, smb)
            row.update({"signature_status":"PASS", **info})
            for dead in ("signature_error_bucket","signature_error"):
                row.pop(dead,None)
            resign_done += 1
        except Exception as exc:
            bucket=_error_bucket(exc); msg=f"{type(exc).__name__}: {exc}"
            row.update({"signature_status":"FAIL","signature_error_bucket":bucket,"signature_error":msg})
            resign_fail += 1
        if (resign_done + resign_fail) % 50 == 0:
            _atomic_json(state_path,state)
            print(f"resign_progress={resign_done + resign_fail}/{resign_total} ok={resign_done} fail={resign_fail}")
    if resign_total:
        _atomic_json(state_path,state)
        print(f"resign_complete={resign_done + resign_fail}/{resign_total} ok={resign_done} fail={resign_fail}")

    # Reconstruct pass groups from the per-candidate resumable state. This is
    # stronger than relying on the periodic summary: an interruption after a
    # PASS cannot silently lose its skeleton identity.
    pass_groups={}
    unsigned_passes=[]
    parse_error_counts=collections.Counter()
    parse_error_samples={}
    structural_pass_count=0
    for row in tested.values():
        if not isinstance(row, dict) or row.get("result") != "PASS":
            continue
        structural_pass_count += 1
        sig=row.get("skeleton_signature")
        if row.get("signature_status") == "PASS" and sig:
            g=pass_groups.setdefault(sig,{"skeleton_signature":sig,
                "topology_signature":row.get("topology_signature"),
                "bone_count":row.get("bone_count"),"root_bones":row.get("root_bones") or [],
                "smb_aliases":[],"reports":[]})
            if row.get("path"): g["smb_aliases"].append(row["path"])
            if row.get("report"): g["reports"].append(row["report"])
        else:
            unsigned={k:row.get(k) for k in ("path","sha256","report","signature_error_bucket","signature_error")}
            unsigned_passes.append(unsigned)
            bucket=row.get("signature_error_bucket") or "UNKNOWN"
            parse_error_counts[bucket]+=1
            if row.get("path"):
                parse_error_samples.setdefault(bucket,{"path":row.get("path"),"error":row.get("signature_error") or ""})

    for pos, rel in enumerate(candidates, 1):
        k=low_path(rel)
        if k in tested:
            continue
        smb=resolver.resolve(rel, required=False)
        if smb is None:
            tested[k]={"path":rel,"result":"MISSING"}
            continue
        rid=stable_id("smb-gate-first", rel).replace("AST-", "")
        rp=reports / f"{rid}.json"; lp=reports / f"{rid}.log"
        proc=subprocess.run(["python3", str(gate), "--smd", str(src), "--smb", str(smb), "--report", str(rp)],
                            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        lp.write_text(proc.stdout or "", encoding="utf-8")
        fresh += 1
        if proc.returncode == 0:
            structural_pass_count += 1
            file_sha=sha256_file(smb)
            base={"path":rel,"result":"PASS","sha256":file_sha,"report":str(rp)}
            try:
                info=_signature_for_passing_smb(s, smb)
                sig=info["skeleton_signature"]
                base.update({"signature_status":"PASS", **info})
                g=pass_groups.setdefault(sig,{"skeleton_signature":sig,
                    "topology_signature":info.get("topology_signature"),
                    "bone_count":info.get("bone_count"),"root_bones":info.get("root_bones") or [],
                    "smb_aliases":[],"reports":[]})
                g["smb_aliases"].append(rel); g["reports"].append(str(rp))
                print(f"PASS {rel} skeleton={sig[:12]} bones={info.get('bone_count')}")
            except Exception as exc:
                bucket=_error_bucket(exc); msg=f"{type(exc).__name__}: {exc}"
                base.update({"signature_status":"FAIL","signature_error_bucket":bucket,"signature_error":msg})
                unsigned={"path":rel,"sha256":file_sha,"report":str(rp),
                          "signature_error_bucket":bucket,"signature_error":msg}
                unsigned_passes.append(unsigned)
                parse_error_counts[bucket]+=1
                parse_error_samples.setdefault(bucket,{"path":rel,"error":msg})
                print(f"PASS-UNSIGNED {rel} parser={bucket}")
            tested[k]=base
        else:
            tested[k]={"path":rel,"result":"FAIL"}

        if fresh % 25 == 0:
            state["passes"]=[]
            for g in pass_groups.values():
                row=dict(g); row["smb_aliases"]=sorted(set(row["smb_aliases"]),key=str.lower); row["reports"]=sorted(set(row["reports"]))
                state["passes"].append(row)
            state["passes"].extend(unsigned_passes)
            _atomic_json(state_path,state)
            print(f"progress={pos}/{total} structural_passes={structural_pass_count} "
                  f"signed_skeletons={len(pass_groups)} unsigned_passes={len(unsigned_passes)}")

        # V0.3.5 needs the complete structural PASS universe before a Physique
        # subset equivalence claim can be made, so discovery no longer early-
        # stops at two full-skeleton identities.

    # Canonicalize state.
    state["passes"]=[]
    for g in sorted(pass_groups.values(), key=lambda x:(x.get("smb_aliases") or [""])[0].lower()):
        row=dict(g); row["smb_aliases"]=sorted(set(row["smb_aliases"]),key=str.lower); row["reports"]=sorted(set(row["reports"]))
        state["passes"].append(row)
    state["passes"].extend(sorted(unsigned_passes,key=lambda x:x.get("path","").lower()))
    _atomic_json(state_path,state)

    tested_count=len(tested)
    scan_complete=(tested_count >= total)
    cache_path = reg.binding_cache_path
    cache = _load_root_binding_cache(cache_path)
    entry={"asset":asset.primary_path,"source_sha256":source_sha,"updated_at":utc_now(),
           "candidate_universe":total,"tested_candidates":tested_count,"scan_complete":scan_complete,
           "structural_passes":structural_pass_count,"distinct_signed_skeletons":len(pass_groups),
           "unsigned_structural_passes":len(unsigned_passes),"probe_state":str(state_path),
           "reused_v4_scan":bool(migrated_from_v4),
           "reused_v3_structural_results":bool(migrated_from_v3),
           "resigned_structural_passes":resign_total,"resign_ok":resign_done,"resign_fail":resign_fail,
           "pass_signature_error_counts":dict(sorted(parse_error_counts.items())),
           "pass_signature_error_samples":parse_error_samples}

    if unsigned_passes:
        entry.update({"status":"PASS_SIGNATURE_INCOMPLETE",
                      "source":"GATE_FIRST_SIGNATURE_INCOMPLETE",
                      "full_skeleton_group_sample":state["passes"][:10]})
    elif len(pass_groups) >= 2 and scan_complete:
        # Full skeleton identity is intentionally not required for modular
        # Physique pieces. Resolve whether all candidates are equivalent on the
        # exact weighted-bone ancestor closure that can deform this mesh.
        subset=_physique_subset_resolution(a,s,reg,resolver,asset,src,tested,work)
        entry.update(subset)
        entry["full_skeleton_group_sample"]=state["passes"][:10]
        entry["full_skeleton_groups"]=len(pass_groups)
        if subset.get("status") == "PHYSIQUE_SUBSET_EQUIVALENT":
            entry["source"]="PHYSIQUE_SUBSET_EQUIVALENT"
    elif len(pass_groups) >= 2:
        entry.update({"status":"AMBIGUOUS_SKELETON_MATCH","source":"GATE_FIRST_SKELETON_SIGNATURE",
                      "full_skeleton_group_sample":state["passes"][:10]})
    elif len(pass_groups) == 1 and scan_complete:
        sig,winner=next(iter(pass_groups.items()))
        aliases=sorted(set(winner["smb_aliases"]), key=str.lower)
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
        canonical_motion=sorted(set(referenced_aliases),key=str.lower)[0] if referenced_aliases else aliases[0]
        entry.update({"status":"UNIQUE_SKELETON","motion_smb":canonical_motion,
                      "motion_aliases":aliases,"skeleton_signature":sig,
                      "topology_signature":winner.get("topology_signature"),"bone_count":winner.get("bone_count"),
                      "root_bones":winner.get("root_bones") or [],
                      "root_models":sorted(root_models,key=str.lower),"root_inxs":sorted(root_inxs,key=str.lower),
                      "source":"GATE_FIRST_SKELETON_UNIQUE"})
    elif scan_complete:
        entry.update({"status":"NO_MATCH","source":"GATE_FIRST_STRUCTURAL"})
    else:
        entry.update({"status":"INCOMPLETE_SCAN","source":"GATE_FIRST_STRUCTURAL",
                      "full_skeleton_group_sample":state["passes"][:10]})

    cache.setdefault("entries", {})[low_path(asset.primary_path)] = entry
    _atomic_json(cache_path, cache)
    print(json.dumps({"asset_id":asset.asset_id,"asset":asset.primary_path,**entry}, ensure_ascii=False, indent=2))
    if entry["status"] in {"UNIQUE_SKELETON","PHYSIQUE_SUBSET_EQUIVALENT"}:
        print(f"binding_discovery={entry['status']}; rerun `python3 pt_asset.py asset inspect <asset> --physical`")
        return 0
    if entry["status"] in {"AMBIGUOUS_SKELETON_MATCH","AMBIGUOUS_PHYSIQUE_SUBSET"}:
        print(f"binding_discovery={entry['status']} (fail-closed)")
        return 4
    if entry["status"] in {"PASS_SIGNATURE_INCOMPLETE","PHYSIQUE_SUBSET_INCOMPLETE","PHYSIQUE_USAGE_UNMAPPED","PHYSIQUE_PROBE_IMPORT_FAIL","PHYSIQUE_USAGE_EXTRACT_FAIL"}:
        print(f"binding_discovery={entry['status']} (diagnostic evidence incomplete; fail-closed)")
        return 5
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
    x = sp.add_parser("bindings"); x.add_argument("--category"); x.add_argument("--mode"); x.add_argument("--limit", type=int, default=100); x.add_argument("--discover", metavar="ASSET", help="gate all physical SMBs first, then group only structural PASSes by embedded skeleton signature"); x.add_argument("--candidate-limit", type=int, default=0, help="debug only: cap skeleton-signature groups; zero means all"); x.add_argument("--full-scan", action="store_true", help="continue even after two distinct skeleton signatures pass"); x.add_argument("--no-resume", action="store_true", help="ignore previous per-candidate probe state"); x.set_defaults(func=cmd_bindings)

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

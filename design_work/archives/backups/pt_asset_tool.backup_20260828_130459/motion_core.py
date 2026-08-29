from __future__ import annotations
import importlib.util, json, pathlib, types


def _load(path: pathlib.Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot import {path}")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def _load_evaluator_without_cli(path: pathlib.Path):
    source=path.read_text(encoding="utf-8")
    stripped=source.rstrip()
    if not stripped.endswith("main()"):
        raise RuntimeError(f"unexpected evaluator footer; refusing unsafe import: {path}")
    source=stripped[:-len("main()")]+"\n"
    mod=types.ModuleType("pt_legacy_motion_eval_safe")
    mod.__file__=str(path)
    exec(compile(source,str(path),"exec"),mod.__dict__)
    return mod

def load_motion_system(root: pathlib.Path):
    root = pathlib.Path(root)
    evaluator = _load_evaluator_without_cli(root / "history_py/pt_smb_motion_evaluator_v1.py")

    # The historical evaluator represented the source call
    # D3DMath_QuaternionSlerp(..., NULL, delta, alpha) by passing an
    # all-zero quaternion as A.  In the legacy D3D helper, a NULL A means
    # the identity quaternion, not (0,0,0,0).  Treating it as the latter
    # makes intermediate matrices non-orthonormal and visibly collapses /
    # explodes skinned meshes.  Keep the historical evaluator file intact
    # and correct only this call semantic inside the backend.
    _legacy_slerp = evaluator.quaternion_slerp_source

    def _slerp_null_is_identity(ax, ay, az, aw, bx, by, bz, bw, alpha):
        if ax == 0.0 and ay == 0.0 and az == 0.0 and aw == 0.0:
            aw = 1.0
        return _legacy_slerp(ax, ay, az, aw, bx, by, bz, bw, alpha)

    evaluator.quaternion_slerp_source = _slerp_null_is_identity
    evaluator.pt_null_quaternion_semantics = "IDENTITY"

    inxcore = _load(root / "pt_inx_motion_core_v2.py", "pt_inx_motion_core_v2_runtime")
    return evaluator, inxcore


def enrich_motions(inx_parsed: dict, smb: dict) -> list[dict]:
    out=[]
    for src in inx_parsed["motions"]:
        m=dict(src)
        mf=int(m.get("motion_frame") or 0)
        if mf <= 0:
            m["active_container"] = False
            out.append(m); continue
        slot=mf-1
        if not 0 <= slot < len(smb["header_frames"]):
            raise RuntimeError(f"motion[{m['index']}] slot={slot} outside SMB TmFrameCounter={len(smb['header_frames'])}")
        container=smb["header_frames"][slot]
        base=container["start"]//160
        m.update({
            "active_container": True,
            "slot": slot,
            "global_start": int(m["start"])+base,
            "global_end": int(m["end"])+base,
        })
        m["engine_start"]=m["global_start"]*160
        m["engine_end"]=m["global_end"]*160
        if m["engine_end"] > container["end"]:
            raise RuntimeError(f"motion[{m['index']}] engine_end={m['engine_end']} > container_end={container['end']}")
        if m["global_end"] <= m["global_start"]:
            m["active_container"] = False
        out.append(m)
    return out


def action_name(m: dict) -> str:
    return f"PT_{m['state_name']}_{int(m['index']):03d}"


def select_motion(motions: list[dict], selector: str = "auto") -> dict:
    active=[m for m in motions if m.get("active_container")]
    if not active:
        raise RuntimeError("INX has no active motion with SMB container")
    q=str(selector or "auto").strip()
    if q.lower()=="auto":
        for state in ("STAND","WALK","RUN","ATTACK","DAMAGE","DEAD"):
            hit=next((m for m in active if m.get("state_name")==state),None)
            if hit: return hit
        return active[0]
    if q.isdigit():
        idx=int(q); hit=next((m for m in active if int(m["index"])==idx),None)
        if not hit: raise RuntimeError(f"active motion index not found: {idx}")
        return hit
    uq=q.upper()
    exact=[m for m in active if str(m.get("state_name","")).upper()==uq]
    if exact: return exact[0]
    contains=[m for m in active if uq in str(m.get("state_name","")).upper()]
    if len(contains)==1: return contains[0]
    raise RuntimeError(f"motion selector not found/ambiguous: {selector}")


def core_motion_subset(motions: list[dict]) -> list[dict]:
    active=[m for m in motions if m.get("active_container")]
    picked=[]; seen=set()
    for state in ("STAND","WALK","RUN","ATTACK","DAMAGE","DEAD","FALLDOWN","SKILL"):
        m=next((x for x in active if x.get("state_name")==state),None)
        if m and m["index"] not in seen:
            picked.append(m); seen.add(m["index"])
    return picked or active[:1]


def build_bake_payload(root: pathlib.Path, inx: pathlib.Path, smb_path: pathlib.Path, *, selector="auto", load_actions="core", fps=30) -> dict:
    evaluator, inxcore=load_motion_system(root)
    smb=evaluator.parse_smb(pathlib.Path(smb_path))
    parsed=inxcore.parse_inx(pathlib.Path(inx))
    validation=inxcore.validate_with_smb(parsed, pathlib.Path(smb_path))
    if validation["check"] != "PASS":
        raise RuntimeError(f"INX/SMB validation failed: {validation['errors']}")
    motions=enrich_motions(parsed, smb)
    selected=select_motion(motions, selector)
    active=[m for m in motions if m.get("active_container")]
    if load_actions == "selected": chosen=[selected]
    elif load_actions == "core": chosen=core_motion_subset(motions)
    elif load_actions == "all": chosen=active
    else: raise ValueError("load_actions must be selected/core/all")
    if selected["index"] not in {m["index"] for m in chosen}:
        chosen=[selected]+chosen
    actions=[]
    for m in chosen:
        frames=[]
        for bf, logical in enumerate(range(m["global_start"], m["global_end"]), start=1):
            engine=logical*160
            results, _, _, stats=evaluator.evaluate_pose(smb, engine, m["engine_end"])
            frames.append({"frame":bf,"engine_frame":engine,"matrices":results})
        actions.append({
            "name": action_name(m), "index":m["index"], "state":m["state"], "state_name":m["state_name"],
            "repeat":m["repeat"], "motion_frame":m["motion_frame"], "global_start":m["global_start"],
            "global_end":m["global_end"], "engine_start":m["engine_start"], "engine_end":m["engine_end"],
            "frame_count":len(frames), "frames":frames,
        })
    bones=[{"name":b["name"],"parent":b["parent_name"],"bind_tm":b["tm"]} for b in smb["bones"]]
    return {
        "format":"PT_ASSET_MOTION_BAKE_V1", "inx":str(inx), "smb":str(smb_path), "fps":int(fps),
        "selected_action":action_name(selected), "selected_motion_index":selected["index"],
        "load_actions":load_actions, "motion_count":parsed["motion_count"], "active_motion_count":len(active),
        "actions":actions, "bones":bones, "tm_sort":list(smb["tm_sort"]),
        "quaternion_null_semantics":"IDENTITY", "client_modified":False,
    }


def write_payload(path: pathlib.Path, data: dict):
    path=pathlib.Path(path); path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(data,ensure_ascii=False,separators=(",",":"))+"\n",encoding="utf-8")

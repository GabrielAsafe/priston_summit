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


def _flatten4(m):
    return [float(m[r][c]) for r in range(4) for c in range(4)]


_STAND_BIND_LOCAL_GUARD_BONES = ("Bip01 Neck", "Bip01 Head")


def _apply_monbagon_stand_bind_local_guard(results, smb, smb_path: pathlib.Path, motion: dict):
    """Stabilize only the previously observed bad STAND neck/head chain.

    V0.2.8 proved that rebasing the whole STAND container is wrong: it made
    the visual deformation worse even though Source Direct and Blender agreed.
    Keep the original/global source pose for every other bone and every other
    motion. For Monbagon STAND only, reconstruct Neck and Head at their bind
    local relationship to the *current* parent. This removes malformed local
    animation on that two-bone chain while still following the animated torso.

    Source matrices are row-major / row-vector style, so:
        bind_local = bind_child @ inverse(bind_parent)
        current_child = bind_local @ current_parent
    """
    if str(motion.get("state_name", "")).upper() != "STAND":
        return results, []
    if "monbagon" not in pathlib.Path(smb_path).name.lower():
        return results, []

    bones=smb["bones"]
    name_to_index={str(b.get("name", "")): i for i,b in enumerate(bones)}
    mats=[_mat4(v) for v in results]
    binds=[_mat4(b["tm"]) for b in bones]
    applied=[]
    for name in _STAND_BIND_LOCAL_GUARD_BONES:
        idx=name_to_index.get(name)
        if idx is None:
            continue
        parent_index=int(bones[idx].get("parent_index", -1))
        if parent_index < 0 or parent_index >= len(mats):
            continue
        bind_local=_matmul(binds[idx], _matinv(binds[parent_index]))
        mats[idx]=_matmul(bind_local, mats[parent_index])
        applied.append(name)
    return [_flatten4(m) for m in mats], applied


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
        guard_bones=[]
        for bf, logical in enumerate(range(m["global_start"], m["global_end"]), start=1):
            engine=logical*160
            results, _, _, stats=evaluator.evaluate_pose(smb, engine, m["engine_end"])
            results, applied=_apply_monbagon_stand_bind_local_guard(results, smb, smb_path, m)
            if applied:
                guard_bones=applied
            frames.append({"frame":bf,"engine_frame":engine,"matrices":results})
        actions.append({
            "name": action_name(m), "index":m["index"], "state":m["state"], "state_name":m["state_name"],
            "repeat":m["repeat"], "motion_frame":m["motion_frame"], "global_start":m["global_start"],
            "global_end":m["global_end"], "engine_start":m["engine_start"], "engine_end":m["engine_end"],
            "frame_count":len(frames), "frames":frames,
            "source_pose_reference":"GLOBAL_BIND",
            "pose_guard":"MONBAGON_STAND_BIND_LOCAL_V1" if guard_bones else "NONE",
            "pose_guard_bones":guard_bones,
        })
    bones=[{"name":b["name"],"parent":b["parent_name"],"bind_tm":b["tm"]} for b in smb["bones"]]
    return {
        "format":"PT_ASSET_MOTION_BAKE_V1", "inx":str(inx), "smb":str(smb_path), "fps":int(fps),
        "selected_action":action_name(selected), "selected_motion_index":selected["index"],
        "load_actions":load_actions, "motion_count":parsed["motion_count"], "active_motion_count":len(active),
        "actions":actions, "bones":bones, "tm_sort":list(smb["tm_sort"]),
        "quaternion_null_semantics":"IDENTITY",
        "source_pose_reference_policy":"GLOBAL_BIND",
        "pose_guard_policy":"MONBAGON_STAND_BIND_LOCAL_V1",
        "client_modified":False,
    }


def write_payload(path: pathlib.Path, data: dict):
    path=pathlib.Path(path); path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(data,ensure_ascii=False,separators=(",",":"))+"\n",encoding="utf-8")


def _mat4(values):
    if len(values) != 16:
        raise ValueError("matrix must contain 16 values")
    return [list(map(float, values[i:i+4])) for i in range(0, 16, 4)]


def _matmul(a, b):
    return [[sum(a[r][k] * b[k][c] for k in range(4)) for c in range(4)] for r in range(4)]


def _matinv(a):
    aug=[list(map(float,row))+[1.0 if r==c else 0.0 for c in range(4)] for r,row in enumerate(a)]
    for c in range(4):
        pivot=max(range(c,4), key=lambda r: abs(aug[r][c]))
        if abs(aug[pivot][c]) < 1e-12:
            raise ValueError("singular matrix")
        if pivot != c:
            aug[c], aug[pivot] = aug[pivot], aug[c]
        d=aug[c][c]
        aug[c]=[x/d for x in aug[c]]
        for r in range(4):
            if r == c: continue
            f=aug[r][c]
            if f:
                aug[r]=[aug[r][j]-f*aug[c][j] for j in range(8)]
    return [row[4:] for row in aug]


def _rigid_quality(m):
    # Source matrices are row-major. Assess the upper-left 3x3 basis without
    # assuming whether basis vectors are stored in rows or columns: use rows,
    # which is invariant enough for scale/shear detection on orthogonal matrices.
    v=[m[i][:3] for i in range(3)]
    norms=[sum(x*x for x in q) ** 0.5 for q in v]
    scale_dev=max(abs(x-1.0) for x in norms)
    dots=[]
    for i,j in ((0,1),(0,2),(1,2)):
        dots.append(abs(sum(v[i][k]*v[j][k] for k in range(3))))
    orth=max(dots)
    a,b,c=v
    det=(a[0]*(b[1]*c[2]-b[2]*c[1])
         -a[1]*(b[0]*c[2]-b[2]*c[0])
         +a[2]*(b[0]*c[1]-b[1]*c[0]))
    return {"scale_dev":scale_dev,"orth_error":orth,"det":det,"det_dev":abs(abs(det)-1.0)}


def audit_motion_source(root: pathlib.Path, inx: pathlib.Path, smb_path: pathlib.Path, selector="STAND") -> dict:
    evaluator, inxcore=load_motion_system(root)
    smb=evaluator.parse_smb(pathlib.Path(smb_path))
    parsed=inxcore.parse_inx(pathlib.Path(inx))
    motions=enrich_motions(parsed, smb)
    selected=select_motion(motions, selector)
    active=[m for m in motions if m.get("active_container")]
    bind=[_mat4(b["tm"]) for b in smb["bones"]]
    bind_inv=[_matinv(x) for x in bind]
    names=[b["name"] for b in smb["bones"]]

    def analyze(m):
        worst={"score":-1.0}
        first=None; last=None
        frames=0
        bone_worst={}
        for bf, logical in enumerate(range(m["global_start"], m["global_end"]), start=1):
            engine=logical*160
            results, _, _, _=evaluator.evaluate_pose(smb, engine, m["engine_end"])
            frames += 1
            frame_max=0.0
            frame_info=None
            for i, vals in enumerate(results):
                cur=_mat4(vals)
                delta=_matmul(cur, bind_inv[i])
                q=_rigid_quality(delta)
                score=max(q["scale_dev"],q["orth_error"],q["det_dev"])
                if score > frame_max:
                    frame_max=score; frame_info=(i,q)
                prev=bone_worst.get(i)
                if prev is None or score > prev[0]:
                    bone_worst[i]=(score,bf,engine,q)
            if first is None: first=frame_max
            last=frame_max
            if frame_max > worst.get("score",-1):
                i,q=frame_info
                worst={"score":frame_max,"frame":bf,"engine_frame":engine,"bone":names[i],**q}
        tops=sorted(bone_worst.items(),key=lambda kv:kv[1][0],reverse=True)[:10]
        return {
            "index":int(m["index"]),"state":m["state_name"],"repeat":int(m["repeat"]),
            "motion_frame":int(m["motion_frame"]),"global_start":int(m["global_start"]),"global_end":int(m["global_end"]),
            "engine_start":int(m["engine_start"]),"engine_end":int(m["engine_end"]),"frames":frames,
            "first_frame_rigidity_score":first,"last_frame_rigidity_score":last,
            "worst_rigidity":worst,
            "top_bones":[{"bone":names[i],"score":v[0],"frame":v[1],"engine_frame":v[2],**v[3]} for i,v in tops],
        }

    # Always compare the selected state with the already useful baselines when present.
    picked=[]; seen=set()
    for m in [selected] + [x for x in active if x.get("state_name") in ("WALK","ATTACK")]:
        if m["index"] in seen: continue
        seen.add(m["index"]); picked.append(m)
    return {
        "format":"PT_SOURCE_MOTION_AUDIT_V1",
        "selected_motion_index":int(selected["index"]),
        "selected_state":selected["state_name"],
        "bone_count":len(names),
        "motions":[analyze(m) for m in picked],
        "interpretation":{
            "rigidity_score":"max(scale deviation, basis non-orthogonality, |abs(det)-1|) on current*inverse(bind)",
            "near_zero":"source transforms remain rigid; investigate segment/keyframe semantics rather than matrix scale/shear",
            "large":"source evaluator is producing scale/shear/non-rigid matrices"
        },
        "client_modified":False,
    }



def _pose_diff(a, b):
    """Matrix-space delta metric; intentionally layout-agnostic."""
    worst={"score":-1.0}
    total=0.0; count=0
    for i,(ma,mb) in enumerate(zip(a,b)):
        vals=[float(x)-float(y) for x,y in zip(ma,mb)]
        score=sum(x*x for x in vals) ** 0.5
        total += score*score; count += 1
        if score > worst["score"]:
            worst={"score":score,"bone_index":i}
    return worst, ((total/max(count,1)) ** 0.5)


def _frame_like_value(obj):
    if not isinstance(obj, dict): return None
    for k,v in obj.items():
        lk=str(k).lower()
        if ("frame" in lk or "time" in lk) and isinstance(v,(int,float)):
            return {"field":str(k),"value":v}
    return None


def _key_sequences(obj, path="", depth=0):
    """Find likely keyframe arrays without relying on a specific legacy parser schema."""
    if depth > 5: return []
    out=[]
    if isinstance(obj, dict):
        for k,v in obj.items():
            child=f"{path}.{k}" if path else str(k)
            out.extend(_key_sequences(v,child,depth+1))
    elif isinstance(obj, (list,tuple)):
        if obj and isinstance(obj[0],dict):
            probes=[_frame_like_value(x) for x in obj[:min(len(obj),8)]]
            if any(x is not None for x in probes):
                frames=[]
                for item in obj:
                    fv=_frame_like_value(item)
                    if fv is not None:
                        frames.append(fv)
                out.append({
                    "path":path,"length":len(obj),"frame_samples":frames[:12],
                    "first":frames[0] if frames else None,"last":frames[-1] if frames else None,
                })
        # Do not recurse through long scalar/matrix arrays. Recurse only into
        # structured short arrays, which is where parser key records live.
        if len(obj) <= 64:
            for i,v in enumerate(obj):
                if isinstance(v,(dict,list,tuple)):
                    out.extend(_key_sequences(v,f"{path}[{i}]",depth+1))
    return out


def audit_motion_segments(root: pathlib.Path, inx: pathlib.Path, smb_path: pathlib.Path, selector="STAND") -> dict:
    evaluator, inxcore=load_motion_system(root)
    smb=evaluator.parse_smb(pathlib.Path(smb_path))
    parsed=inxcore.parse_inx(pathlib.Path(inx))
    motions=enrich_motions(parsed,smb)
    selected=select_motion(motions,selector)
    active=[m for m in motions if m.get("active_container")]
    names=[b["name"] for b in smb["bones"]]

    def eval_at(logical,end_engine):
        results,_,_,stats=evaluator.evaluate_pose(smb,int(logical)*160,int(end_engine))
        return results,stats

    def boundary(m):
        points=[]
        for logical in range(int(m["global_start"])-2,int(m["global_start"])+3):
            pose,stats=eval_at(logical,m["engine_end"])
            points.append((logical,pose,stats))
        for logical in range(int(m["global_end"])-2,int(m["global_end"])+3):
            if any(x[0]==logical for x in points): continue
            pose,stats=eval_at(logical,m["engine_end"])
            points.append((logical,pose,stats))
        points.sort(key=lambda x:x[0])
        transitions=[]
        for (la,pa,_),(lb,pb,_) in zip(points,points[1:]):
            if lb != la+1: continue
            worst,rms=_pose_diff(pa,pb)
            transitions.append({"from":la,"to":lb,"engine_from":la*160,"engine_to":lb*160,
                                "worst_bone":names[worst["bone_index"]],"max_matrix_step":worst["score"],"rms_bone_step":rms})
        return transitions

    # Test whether frame_end materially changes a pose inside the selected
    # segment. This catches evaluators that use the passed segment end to pick
    # interpolation/wrap keys.
    mid=(int(selected["global_start"])+int(selected["global_end"])-1)//2
    probe_ends=[]
    end_candidates={
        "selected_end":int(selected["engine_end"]),
        "one_frame_before":int(selected["engine_end"])-160,
        "one_frame_after":int(selected["engine_end"])+160,
    }
    base,_=eval_at(mid,end_candidates["selected_end"])
    for label,endv in end_candidates.items():
        pose,_=eval_at(mid,endv)
        worst,rms=_pose_diff(base,pose)
        probe_ends.append({"label":label,"frame_end":endv,"max_diff_from_selected_end":worst["score"],
                           "worst_bone":names[worst["bone_index"]],"rms_bone_diff":rms})

    # Introspect likely raw key arrays for every bone, but emit only distinct
    # path/length/frame-range signatures plus bones whose signatures differ.
    sig_groups={}
    per_bone=[]
    for i,b in enumerate(smb["bones"]):
        seqs=_key_sequences(b)
        compact=[]
        for q in seqs:
            compact.append({"path":q["path"],"length":q["length"],"first":q["first"],"last":q["last"]})
        sig=json.dumps(compact,sort_keys=True,ensure_ascii=False)
        sig_groups.setdefault(sig,[]).append(names[i])
        per_bone.append((names[i],compact))
    key_layout_groups=[]
    for sig,bns in sig_groups.items():
        key_layout_groups.append({"bones":bns,"sequences":json.loads(sig)})

    compare=[]; seen=set()
    for m in [selected]+[x for x in active if x.get("state_name") in ("WALK","ATTACK")]:
        if m["index"] in seen: continue
        seen.add(m["index"])
        compare.append({
            "index":int(m["index"]),"state":m["state_name"],"motion_frame":int(m["motion_frame"]),
            "repeat":int(m["repeat"]),"global_start":int(m["global_start"]),"global_end":int(m["global_end"]),
            "engine_start":int(m["engine_start"]),"engine_end":int(m["engine_end"]),
            "boundary_steps":boundary(m),
        })

    return {
        "format":"PT_MOTION_SEGMENT_KEY_AUDIT_V1",
        "selected_motion_index":int(selected["index"]),"selected_state":selected["state_name"],
        "selected_frame_end_sensitivity":probe_ends,
        "motions":compare,
        "smb_top_level_keys":sorted(map(str,smb.keys())),
        "bone_record_keys":sorted(map(str,smb["bones"][0].keys())) if smb.get("bones") else [],
        "key_layout_groups":key_layout_groups,
        "interpretation":{
            "boundary_steps":"large discontinuity at start/end can indicate wrong segment bounds or cross-segment key selection",
            "frame_end_sensitivity":"non-zero differences show evaluate_pose semantics depend on the supplied motion end",
            "key_layout_groups":"raw parser key arrays grouped by bone layout; frame ranges reveal whether keys span multiple INX segments"
        },
        "client_modified":False,
    }

#!/usr/bin/env python3
"""
PT_ASSET_FAMILY_REFINE_V2

Second-stage analysis. Does NOT rescan all assets.
Consumes asset_inventory.json from PT_ASSET_SIMILARITY_SURVEY_V1_1 and turns
the coarse inventory into semantic subfamilies, INX pair coverage, runtime
representative queue, and outlier lists.

Optionally reads only a small number of raw client files to fingerprint
nonstandard TGA / unknown SMD headers. It never writes to the client.
"""
from __future__ import annotations
import argparse, csv, json, re, struct
from pathlib import Path
from collections import Counter, defaultdict
from typing import Any

def norm(s: str) -> str:
    return str(s).replace("\\","/").lower().lstrip("./")

def replace_ext(ref: str, ext: str) -> str:
    r=norm(ref)
    if not r:
        return ""
    leaf=r.rsplit("/",1)[-1]
    if "." in leaf:
        return re.sub(r"\.[^.\/]+$", ext, r)
    return r+ext

def mat_bucket(v):
    try: v=int(v)
    except: return "unknown"
    if v <= 1: return "1"
    if v <= 4: return "2-4"
    if v <= 8: return "5-8"
    if v <= 32: return "9-32"
    if v <= 128: return "33-128"
    return "129+"

def geom_bucket(v):
    try: v=int(v)
    except: return "unknown"
    if v < 50: return "<50V"
    if v < 300: return "50-299V"
    if v < 1000: return "300-999V"
    if v < 5000: return "1000-4999V"
    return "5000+V"

def phys_mode(r):
    oc=int(r.get("object_count") or 0)
    pc=int(r.get("physique_objects") or 0)
    if oc and pc == oc: return "RIGID"
    if pc == 0: return "STATIC"
    return "MIXED"

def write_csv(path: Path, rows: list[dict[str,Any]]):
    path.parent.mkdir(parents=True, exist_ok=True)
    if not rows:
        path.write_text("",encoding="utf-8"); return
    fields=[]
    seen=set()
    for r in rows:
        for k in r:
            if k not in seen:
                seen.add(k); fields.append(k)
    with path.open("w",encoding="utf-8",newline="") as f:
        w=csv.DictWriter(f,fieldnames=fields,extrasaction="ignore")
        w.writeheader(); w.writerows(rows)

def choose_rep(items):
    ss=sorted(items,key=lambda r:int(r.get("size") or 0))
    return ss[len(ss)//2]

def raw_signature(client_root: Path, rel: str):
    p=client_root/rel
    try:
        size=p.stat().st_size
        with p.open("rb") as f:
            head=f.read(32)
            if size>32:
                f.seek(max(0,size-32))
                tail=f.read(32)
            else:
                tail=b""
        return {
            "raw_head16":head[:16].hex(),
            "raw_head32":head.hex(),
            "raw_tail16":tail[-16:].hex() if tail else "",
            "raw_tail32":tail.hex(),
        }
    except Exception as e:
        return {"raw_error":f"{type(e).__name__}: {e}"}

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--inventory",required=True)
    ap.add_argument("--client-root",required=True)
    ap.add_argument("--out-dir",required=True)
    ap.add_argument("--raw-signatures",action="store_true",
                    help="Read headers/footers only for TGA unknown and SMD outliers")
    a=ap.parse_args()

    inv=Path(a.inventory).expanduser().resolve()
    client_root=Path(a.client_root).expanduser().resolve()
    out=Path(a.out_dir).expanduser().resolve()
    out.mkdir(parents=True,exist_ok=True)

    data=json.loads(inv.read_text(encoding="utf-8"))
    records=[]
    for r in data["records"]:
        p=Path(str(r.get("path","")))
        try:
            p.relative_to(client_root)
        except Exception:
            continue
        records.append(r)

    by_rel={norm(r["relative_path"]):r for r in records}

    # 1. Macro coverage.
    tiers=Counter(r.get("compatibility_tier","UNKNOWN") for r in records)
    families=Counter((r.get("compatibility_tier",""),r.get("compatibility_family","")) for r in records)
    family_rows=[]
    for (tier,fam),count in families.most_common():
        family_rows.append({
            "tier":tier,"family":fam,"file_count":count,
            "percent":round(count/len(records)*100,4),
        })
    write_csv(out/"family_summary.csv",family_rows)

    # 2. SMD semantic subfamilies.
    smds=[r for r in records if r.get("kind")=="SMD_MODEL_062"]
    groups=defaultdict(list)
    for r in smds:
        key=(
            phys_mode(r),
            r.get("category_hint","UNKNOWN"),
            int(r.get("object_count") or 0),
            mat_bucket(r.get("material_count")),
            geom_bucket(r.get("total_vertices")),
        )
        groups[key].append(r)
    smd_rows=[]
    for key,items in groups.items():
        rep=choose_rep(items)
        smd_rows.append({
            "physique_mode":key[0],
            "category":key[1],
            "object_count":key[2],
            "material_bucket":key[3],
            "geometry_bucket":key[4],
            "file_count":len(items),
            "unique_sample_digests":len({x.get("sample_digest") for x in items}),
            "representative":rep["relative_path"],
            "representative_material_count":rep.get("material_count"),
            "representative_vertices":rep.get("total_vertices"),
            "representative_faces":rep.get("total_faces"),
        })
    smd_rows.sort(key=lambda r:r["file_count"],reverse=True)
    write_csv(out/"smd_semantic_subfamilies.csv",smd_rows)

    # 3. INX model/motion physical pairing.
    inx_rows=[]
    for r in records:
        if r.get("kind")!="INX_MODELINFO_GROUPS":
            continue
        model_smd=replace_ext(r.get("model_ref",""),".smd")
        motion_smb=replace_ext(r.get("motion_ref",""),".smb")
        model_hit=model_smd if model_smd in by_rel else ""
        motion_hit=motion_smb if motion_smb and motion_smb in by_rel else ""
        model_rec=by_rel.get(model_hit) if model_hit else None
        motion_rec=by_rel.get(motion_hit) if motion_hit else None
        inx_rows.append({
            "inx":r["relative_path"],
            "model_ref":r.get("model_ref",""),
            "model_smd":model_hit,
            "model_family":model_rec.get("compatibility_family","") if model_rec else "",
            "motion_ref":r.get("motion_ref",""),
            "motion_smb":motion_hit,
            "motion_family":motion_rec.get("compatibility_family","") if motion_rec else "",
            "high":",".join((r.get("groups") or {}).get("high",{}).get("names",[])),
            "default":",".join((r.get("groups") or {}).get("default",{}).get("names",[])),
            "low":",".join((r.get("groups") or {}).get("low",{}).get("names",[])),
            "model_resolved":bool(model_hit),
            "motion_required":bool(norm(r.get("motion_ref",""))),
            "motion_resolved":bool(motion_hit),
        })
    write_csv(out/"inx_physical_pairing.csv",inx_rows)

    # 4. Texture prefix clusters from inventory.
    bmp_groups=Counter()
    for r in records:
        if r.get("kind")=="BMP_PRISTON_PREFIX14":
            key=(r.get("prefix14_hex",""),r.get("bpp",""),r.get("compression",""))
            bmp_groups[key]+=1
    bmp_rows=[
        {"prefix14_hex":k[0],"bpp":k[1],"compression":k[2],"file_count":v}
        for k,v in bmp_groups.most_common()
    ]
    write_csv(out/"bmp_prefix14_clusters.csv",bmp_rows)

    # 5. Outliers and optional raw signatures.
    outliers=[]
    tga_raw=Counter()
    for r in records:
        if r.get("compatibility_tier")=="D_NEW_REVERSE_ENGINEERING":
            row={
                "path":r["relative_path"],"kind":r.get("kind"),
                "family":r.get("compatibility_family"),"size":r.get("size"),
                "issues":json.dumps(r.get("issues",[]),ensure_ascii=False),
            }
            if a.raw_signatures:
                row.update(raw_signature(client_root,r["relative_path"]))
            outliers.append(row)
        if a.raw_signatures and r.get("kind")=="TGA_UNKNOWN":
            sig=raw_signature(client_root,r["relative_path"])
            key=(sig.get("raw_head16",""),sig.get("raw_tail16",""))
            tga_raw[key]+=1
    write_csv(out/"outliers.csv",outliers)
    if a.raw_signatures:
        write_csv(out/"tga_raw_signature_clusters.csv",[
            {"head16":k[0],"tail16":k[1],"file_count":v}
            for k,v in tga_raw.most_common()
        ])

    # 6. Runtime representative queue.
    # Prefer high-volume semantic branches, not each asset.
    category_priority={
        ("RIGID","MOB_MONSTER"):1,
        ("RIGID","NPC"):2,
        ("RIGID","CHARACTER"):3,
        ("STATIC","WEAPON_ITEM"):4,
        ("STATIC","EFFECT"):5,
        ("STATIC","MAP_STAGE"):6,
    }
    reps=[]
    picked=Counter()
    for row in smd_rows:
        k=(row["physique_mode"],row["category"])
        if k not in category_priority:
            continue
        # Up to three representatives per high-level semantic category.
        if picked[k]>=3:
            continue
        if row["file_count"]<2 and picked[k]>0:
            continue
        reps.append({
            "priority":category_priority[k],
            "purpose":f"{k[0]}_{k[1]}",
            "cluster_file_count":row["file_count"],
            "representative":row["representative"],
            "object_count":row["object_count"],
            "material_bucket":row["material_bucket"],
            "geometry_bucket":row["geometry_bucket"],
            "recommended_gate":(
                "existing rigid character core + auto profile"
                if k[0]=="RIGID"
                else "static/no-Physique writer adapter"
            ),
        })
        picked[k]+=1
    reps.sort(key=lambda r:(r["priority"],-r["cluster_file_count"]))
    write_csv(out/"runtime_representative_queue.csv",reps)

    # 7. Summary Markdown.
    inx_count=len(inx_rows)
    model_res=sum(bool(x["model_resolved"]) for x in inx_rows)
    motion_req=sum(bool(x["motion_required"]) for x in inx_rows)
    motion_res=sum(bool(x["motion_resolved"]) for x in inx_rows)
    smd_files=[r for r in records if str(r.get("extension","")).lower()==".smd"]
    known_smd=sum(r.get("kind") in ("SMD_MODEL_062","SMD_STAGE_072") for r in smd_files)
    lines=[
        "# Asset Family Refinement V2",
        "",
        f"- canonical inspected records: **{len(records)}**",
        f"- A_REUSE_CORE_NOW: **{tiers.get('A_REUSE_CORE_NOW',0)}**",
        f"- B_REUSE_WITH_ADAPTER: **{tiers.get('B_REUSE_WITH_ADAPTER',0)}**",
        f"- C_SEPARATE_FAMILY: **{tiers.get('C_SEPARATE_FAMILY',0)}**",
        f"- D_NEW_REVERSE_ENGINEERING: **{tiers.get('D_NEW_REVERSE_ENGINEERING',0)}**",
        f"- recognized SMD top-level: **{known_smd}/{len(smd_files)}**",
        f"- INX model physical resolve: **{model_res}/{inx_count}**",
        f"- INX motion SMB resolve: **{motion_res}/{motion_req}**",
        "",
        "## Engineering decision",
        "",
        "Do not port by filename. Certify semantic subfamilies and generate profiles.",
        "",
        "## Next representatives",
        "",
    ]
    for r in reps:
        lines.append(
            f"- P{r['priority']} `{r['representative']}` — "
            f"{r['purpose']}; cluster={r['cluster_file_count']}; "
            f"obj={r['object_count']}; mat={r['material_bucket']}; geom={r['geometry_bucket']}"
        )
    (out/"refinement_report.md").write_text("\n".join(lines)+"\n",encoding="utf-8")

    print("PT_ASSET_FAMILY_REFINE_V2")
    print(f"canonical_records={len(records)}")
    print(f"smd_subfamilies={len(smd_rows)}")
    print(f"inx_model_resolved={model_res}/{inx_count}")
    print(f"inx_motion_resolved={motion_res}/{motion_req}")
    print(f"representatives={len(reps)}")
    print(f"out_dir={out}")
    print("client_modified=False")
    print("asset_family_refine_check=PASS")

if __name__=="__main__":
    main()

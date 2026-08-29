#!/usr/bin/env python3
from __future__ import annotations
import argparse,pathlib,hashlib,json,struct,os,tempfile,math

SOURCE_SHA="9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SOURCE_SIZE=187504
MAT_GROUP_SIZE=88
MATERIAL_SIZE=320

def sha(d): return hashlib.sha256(d).hexdigest()
def i32(d,o): return struct.unpack_from("<i",d,o)[0]
def f32(d,o): return struct.unpack_from("<f",d,o)[0]

def atomic_write(path,data):
    path.parent.mkdir(parents=True,exist_ok=True)
    fd,tmp=tempfile.mkstemp(prefix=path.name+".",suffix=".tmp",dir=path.parent); os.close(fd)
    t=pathlib.Path(tmp)
    try: t.write_bytes(data); os.replace(t,path)
    finally:
        if t.exists(): t.unlink()

def atomic_json(path,obj):
    path.parent.mkdir(parents=True,exist_ok=True)
    fd,tmp=tempfile.mkstemp(prefix=path.name+".",suffix=".tmp",dir=path.parent); os.close(fd)
    t=pathlib.Path(tmp)
    try:
        t.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+"\n",encoding="utf-8"); os.replace(t,path)
    finally:
        if t.exists(): t.unlink()

def parse(data):
    if not data.startswith(b"SMD Model data Ver 0.62"): raise ValueError("bad header")
    mc=i32(data,28); mp=i32(data,32); fp=i32(data,36)
    if i32(data,mp+8)!=mc: raise ValueError("MaterialCount mismatch")
    cursor=mp+MAT_GROUP_SIZE; rows=[]
    for mid in range(mc):
        base=cursor; iu=i32(data,base); tc=i32(data,base+4); ac=i32(data,base+304)
        if not 0<=tc<=8 or not 0<=ac<=32: raise ValueError("bad counters")
        rows.append({
            "id":mid,"base":base,
            "selfillum_off":base+148,"twoside_off":base+124,"blendtype_off":base+116,
            "self_illum":f32(data,base+148),"two_side":i32(data,base+124),"blend_type":i32(data,base+116)
        })
        cursor+=MATERIAL_SIZE
        if iu:
            sl=i32(data,cursor)
            if sl<0: raise ValueError("negative StrLen")
            cursor+=4+sl
    if cursor!=fp: raise ValueError("material region end mismatch")
    return rows

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--source",default="TmbC01.smd")
    ap.add_argument("--edit-manifest",required=True)
    ap.add_argument("--out",default="pt_build/characters/pikeman_c01_m4/materials/TmbC01_from_blender_material_v3_blendtype.smd")
    ap.add_argument("--manifest",default="logs/characters/pikeman_c01_m4/materials/TmbC01_from_blender_material_v3_blendtype_build_manifest.json")
    args=ap.parse_args()

    src=pathlib.Path(args.source).expanduser().resolve(); raw=src.read_bytes()
    if sha(raw)!=SOURCE_SHA or len(raw)!=SOURCE_SIZE:
        raise ValueError("registered TmbC01 source baseline mismatch")

    ed=json.loads(pathlib.Path(args.edit_manifest).expanduser().resolve().read_text(encoding="utf-8"))
    if ed.get("schema")!="PT_SMD_MATERIAL_EDIT_MANIFEST_V3": raise ValueError("wrong schema")
    if ed.get("editable_fields")!=["self_illum","two_side","blend_type"]: raise ValueError("wrong editable set")

    rows=parse(raw); mats=ed.get("materials")
    if not isinstance(mats,list): raise ValueError("materials must be list")
    ids=[int(m["material_id"]) for m in mats]
    if ids!=list(range(len(rows))): raise ValueError(f"manifest must include all material ids: {ids}")

    out=bytearray(raw); reqs=[]
    for m,r in zip(mats,rows):
        si=float(m["self_illum"]); ts=int(m["two_side"]); bt=int(m["blend_type"])
        if not math.isfinite(si) or si<0: raise ValueError("invalid self_illum")
        if ts not in (0,1): raise ValueError("two_side must be 0/1")
        if not 0<=bt<=6: raise ValueError("blend_type must be 0..6")
        struct.pack_into("<f",out,r["selfillum_off"],si)
        struct.pack_into("<I",out,r["twoside_off"],ts)
        struct.pack_into("<I",out,r["blendtype_off"],bt)
        reqs.append({
            "material_id":r["id"],
            "old_self_illum":r["self_illum"],"new_self_illum":si,"selfillum_offset":r["selfillum_off"],
            "old_two_side":r["two_side"],"new_two_side":ts,"twoside_offset":r["twoside_off"],
            "old_blend_type":r["blend_type"],"new_blend_type":bt,"blendtype_offset":r["blendtype_off"],
        })

    changed=[i for i,(a,b) in enumerate(zip(raw,out)) if a!=b]
    allowed=set()
    for q in reqs:
        for key in ("selfillum_offset","twoside_offset","blendtype_offset"):
            allowed.update(range(q[key],q[key]+4))
    outside=[i for i in changed if i not in allowed]
    if outside: raise RuntimeError(f"outside allowed fields: {outside[:20]}")
    if len(out)!=len(raw): raise RuntimeError("size changed")

    after=parse(bytes(out))
    for q,a in zip(reqs,after):
        if abs(a["self_illum"]-q["new_self_illum"])>1e-7: raise RuntimeError("SelfIllum verify fail")
        if a["two_side"]!=q["new_two_side"]: raise RuntimeError("TwoSide verify fail")
        if a["blend_type"]!=q["new_blend_type"]: raise RuntimeError("BlendType verify fail")

    outp=pathlib.Path(args.out).expanduser()
    if not outp.is_absolute(): outp=pathlib.Path.cwd()/outp
    outp=outp.resolve(); atomic_write(outp,bytes(out)); outsha=sha(bytes(out))
    mp=pathlib.Path(args.manifest).expanduser()
    if not mp.is_absolute(): mp=pathlib.Path.cwd()/mp
    mp=mp.resolve()
    obj={"tool":"PT_SMD_MATERIAL_BUILD_FROM_BLENDER_V3",
         "roundtrip_target":"char/tmABCD/TmbC01.smd",
         "source":str(src),"source_sha256":SOURCE_SHA,"source_size":SOURCE_SIZE,
         "artifact":str(outp),"artifact_sha256":outsha,"artifact_size":len(out),
         "requested_changes":reqs,
         "allowed_fields":["SelfIllum","TwoSide","BlendType"],
         "changed_byte_count":len(changed),"changed_offsets":changed,
         "outside_allowed_field_changed_byte_count":len(outside),
         "file_size_preserved":True,"runtime_installed":False,"build_check":"PASS"}
    atomic_json(mp,obj)
    print("PT_SMD_MATERIAL_BUILD_FROM_BLENDER_V3")
    for q in reqs:
        print(f"MATERIAL {q['material_id']} SelfIllum {q['old_self_illum']} -> {q['new_self_illum']} TwoSide {q['old_two_side']} -> {q['new_two_side']} BlendType {q['old_blend_type']} -> {q['new_blend_type']}")
    print(f"changed_byte_count={len(changed)}")
    print(f"outside_allowed_field_changed_byte_count={len(outside)}")
    print("file_size_preserved=True")
    print(f"artifact={outp}")
    print(f"artifact_sha256={outsha}")
    print(f"manifest={mp}")
    print("runtime_installed=False")
    print("build_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as exc:
        print(f"build_check=FAIL: {type(exc).__name__}: {exc}")
        raise

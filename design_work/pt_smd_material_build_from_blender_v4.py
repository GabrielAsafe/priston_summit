#!/usr/bin/env python3
from __future__ import annotations
import argparse,pathlib,hashlib,json,struct,os,tempfile,math

SOURCE_SHA="9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SOURCE_SIZE=187504
MAT_GROUP_SIZE=88
MATERIAL_SIZE=320
BLINK_BIT=0x4000

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
            "blendtype_off":base+116,
            "twoside_off":base+124,
            "diffuse_off":base+132,
            "selfillum_off":base+148,
            "usestate_off":base+164,
            "wind_off":base+172,
            "blend_type":i32(data,base+116),
            "two_side":i32(data,base+124),
            "diffuse":[f32(data,base+132),f32(data,base+136),f32(data,base+140)],
            "self_illum":f32(data,base+148),
            "use_state":i32(data,base+164),
            "wind_mesh_bottom":i32(data,base+172),
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
    ap.add_argument("--out",default="pt_build/characters/pikeman_c01_m4/materials/TmbC01_from_blender_material_v4_blink_color.smd")
    ap.add_argument("--manifest",default="logs/characters/pikeman_c01_m4/materials/TmbC01_from_blender_material_v4_blink_color_build_manifest.json")
    args=ap.parse_args()

    src=pathlib.Path(args.source).expanduser().resolve(); raw=src.read_bytes()
    if sha(raw)!=SOURCE_SHA or len(raw)!=SOURCE_SIZE:
        raise ValueError("registered TmbC01 source baseline mismatch")

    ed=json.loads(pathlib.Path(args.edit_manifest).expanduser().resolve().read_text(encoding="utf-8"))
    if ed.get("schema")!="PT_SMD_MATERIAL_EDIT_MANIFEST_V4": raise ValueError("wrong schema")
    expected=["self_illum","two_side","blend_type","blink_color_enabled","blink_rgb","blink_time"]
    if ed.get("editable_fields")!=expected: raise ValueError("wrong editable set")

    rows=parse(raw); mats=ed.get("materials")
    if not isinstance(mats,list): raise ValueError("materials must be list")
    ids=[int(m["material_id"]) for m in mats]
    if ids!=list(range(len(rows))): raise ValueError(f"manifest must include all material ids: {ids}")

    out=bytearray(raw); reqs=[]
    for m,r in zip(mats,rows):
        si=float(m["self_illum"]); ts=int(m["two_side"]); bt=int(m["blend_type"])
        enabled=bool(m["blink_color_enabled"])
        rgb=[float(x) for x in m["blink_rgb"]]
        blink_time=int(m["blink_time"])
        if not math.isfinite(si) or si<0: raise ValueError("invalid self_illum")
        if ts not in (0,1): raise ValueError("two_side must be 0/1")
        if not 0<=bt<=6: raise ValueError("blend_type must be 0..6")
        if len(rgb)!=3 or not all(math.isfinite(x) and 0.0<=x<=1.0 for x in rgb):
            raise ValueError("blink_rgb must be three finite floats in 0..1")
        if not 1<=blink_time<=8: raise ValueError("blink_time must be 1..8")

        new_use=(r["use_state"] & ~BLINK_BIT) | (BLINK_BIT if enabled else 0)
        # For this registered Pike baseline, disabled blink must restore zero.
        # Otherwise WindMeshBottom is interpreted by the wind-mesh path.
        new_wind=(8+blink_time) if enabled else 0

        struct.pack_into("<I",out,r["blendtype_off"],bt)
        struct.pack_into("<I",out,r["twoside_off"],ts)
        for j,x in enumerate(rgb): struct.pack_into("<f",out,r["diffuse_off"]+4*j,x)
        struct.pack_into("<f",out,r["selfillum_off"],si)
        struct.pack_into("<I",out,r["usestate_off"],new_use)
        struct.pack_into("<i",out,r["wind_off"],new_wind)

        reqs.append({
            "material_id":r["id"],
            "old_self_illum":r["self_illum"],"new_self_illum":si,"selfillum_offset":r["selfillum_off"],
            "old_two_side":r["two_side"],"new_two_side":ts,"twoside_offset":r["twoside_off"],
            "old_blend_type":r["blend_type"],"new_blend_type":bt,"blendtype_offset":r["blendtype_off"],
            "old_diffuse":r["diffuse"],"new_diffuse":rgb,"diffuse_offset":r["diffuse_off"],
            "old_use_state":r["use_state"],"new_use_state":new_use,"usestate_offset":r["usestate_off"],
            "old_wind_mesh_bottom":r["wind_mesh_bottom"],"new_wind_mesh_bottom":new_wind,"wind_offset":r["wind_off"],
            "blink_color_enabled":enabled,"blink_time":blink_time,
        })

    changed=[i for i,(a,b) in enumerate(zip(raw,out)) if a!=b]
    allowed=set()
    for q in reqs:
        allowed.update(range(q["blendtype_offset"],q["blendtype_offset"]+4))
        allowed.update(range(q["twoside_offset"],q["twoside_offset"]+4))
        allowed.update(range(q["diffuse_offset"],q["diffuse_offset"]+12))
        allowed.update(range(q["selfillum_offset"],q["selfillum_offset"]+4))
        allowed.update(range(q["usestate_offset"],q["usestate_offset"]+4))
        allowed.update(range(q["wind_offset"],q["wind_offset"]+4))
    outside=[i for i in changed if i not in allowed]
    if outside: raise RuntimeError(f"outside allowed fields: {outside[:20]}")
    if len(out)!=len(raw): raise RuntimeError("size changed")

    after=parse(bytes(out))
    for q,a in zip(reqs,after):
        if abs(a["self_illum"]-q["new_self_illum"])>1e-7: raise RuntimeError("SelfIllum verify fail")
        if a["two_side"]!=q["new_two_side"]: raise RuntimeError("TwoSide verify fail")
        if a["blend_type"]!=q["new_blend_type"]: raise RuntimeError("BlendType verify fail")
        if any(abs(x-y)>1e-7 for x,y in zip(a["diffuse"],q["new_diffuse"])): raise RuntimeError("Diffuse verify fail")
        if a["use_state"]!=q["new_use_state"]: raise RuntimeError("UseState verify fail")
        if a["wind_mesh_bottom"]!=q["new_wind_mesh_bottom"]: raise RuntimeError("WindMeshBottom verify fail")

    outp=pathlib.Path(args.out).expanduser()
    if not outp.is_absolute(): outp=pathlib.Path.cwd()/outp
    outp=outp.resolve(); atomic_write(outp,bytes(out)); outsha=sha(bytes(out))
    mp=pathlib.Path(args.manifest).expanduser()
    if not mp.is_absolute(): mp=pathlib.Path.cwd()/mp
    mp=mp.resolve()
    obj={"tool":"PT_SMD_MATERIAL_BUILD_FROM_BLENDER_V4",
         "roundtrip_target":"char/tmABCD/TmbC01.smd",
         "source":str(src),"source_sha256":SOURCE_SHA,"source_size":SOURCE_SIZE,
         "artifact":str(outp),"artifact_sha256":outsha,"artifact_size":len(out),
         "requested_changes":reqs,
         "allowed_fields":["SelfIllum","TwoSide","BlendType","Diffuse","UseState.BLINK_COLOR_BIT","WindMeshBottom"],
         "blink_bit_hex":"0x4000",
         "blink_time_mapping":{"1":9,"2":10,"3":11,"4":12,"5":13,"6":14,"7":15,"8":16},
         "changed_byte_count":len(changed),"changed_offsets":changed,
         "outside_allowed_field_changed_byte_count":len(outside),
         "file_size_preserved":True,"runtime_installed":False,"build_check":"PASS"}
    atomic_json(mp,obj)

    print("PT_SMD_MATERIAL_BUILD_FROM_BLENDER_V4")
    for q in reqs:
        print(f"MATERIAL {q['material_id']} SelfIllum {q['old_self_illum']} -> {q['new_self_illum']} TwoSide {q['old_two_side']} -> {q['new_two_side']} BlendType {q['old_blend_type']} -> {q['new_blend_type']}")
        print(f"  Diffuse {q['old_diffuse']} -> {q['new_diffuse']}")
        print(f"  UseState {q['old_use_state']} -> {q['new_use_state']} (blink_enabled={q['blink_color_enabled']})")
        print(f"  WindMeshBottom {q['old_wind_mesh_bottom']} -> {q['new_wind_mesh_bottom']} (BLINK_TIME{q['blink_time']})")
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

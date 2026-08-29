#!/usr/bin/env python3
import bpy, pathlib, struct, sys, json, re

MAT_GROUP_SIZE=88
MATERIAL_SIZE=320
BLINK_BIT=0x4000

def i32(d,o): return struct.unpack_from("<i",d,o)[0]
def f32(d,o): return struct.unpack_from("<f",d,o)[0]

def parse_materials(path):
    p=pathlib.Path(path).expanduser().resolve()
    data=p.read_bytes()
    if not data.startswith(b"SMD Model data Ver 0.62"):
        raise RuntimeError("unexpected SMD header")
    mc=i32(data,28); mp=i32(data,32); fp=i32(data,36)
    if i32(data,mp+8)!=mc: raise RuntimeError("MaterialCount mismatch")
    cursor=mp+MAT_GROUP_SIZE
    mats=[]
    for mid in range(mc):
        raw=data[cursor:cursor+MATERIAL_SIZE]
        if len(raw)!=MATERIAL_SIZE: raise RuntimeError(f"material {mid} truncated")
        tc=i32(raw,4); ac=i32(raw,304)
        if not 0<=tc<=8 or not 0<=ac<=32: raise RuntimeError("bad counters")
        use=i32(raw,164)
        wind=i32(raw,172)
        blink=bool(use & BLINK_BIT)
        # Source-proven BLINK_TIME1..8 map to 9..16.
        blink_time = (wind-8) if blink and 9 <= wind <= 16 else 1
        m={
            "id":mid,
            "InUse":i32(raw,0),
            "TextureCounter":tc,
            "TextureStageState":[i32(raw,40+4*i) for i in range(8)],
            "TextureFormState":[i32(raw,72+4*i) for i in range(8)],
            "ReformTexture":i32(raw,104),
            "MapOpacity":f32(raw,108),
            "TextureType":i32(raw,112),
            "BlendType":i32(raw,116),
            "Shade":i32(raw,120),
            "TwoSide":i32(raw,124),
            "Diffuse":[f32(raw,132),f32(raw,136),f32(raw,140)],
            "Transparency":f32(raw,144),
            "SelfIllum":f32(raw,148),
            "TextureClip":i32(raw,160),
            "UseState":use,
            "MeshState":i32(raw,168),
            "WindMeshBottom":wind,
            "blink_enabled":blink,
            "blink_time":blink_time,
            "AnimTexCounter":ac,
            "textures":[],
        }
        cursor+=MATERIAL_SIZE
        if m["InUse"]:
            sl=i32(data,cursor); cursor+=4
            if sl<0 or cursor+sl>fp: raise RuntimeError("invalid StrLen")
            block=data[cursor:cursor+sl]; cursor+=sl
            parts=block.split(b"\0")
            if parts and parts[-1]==b"": parts.pop()
            strings=[x.decode("latin1","replace") for x in parts]
            if len(strings)!=2*(tc+ac): raise RuntimeError(f"material {mid}: string mismatch")
            k=0
            for _ in range(tc):
                m["textures"].append({"Name":strings[k],"NameA":strings[k+1]}); k+=2
        mats.append(m)
    if cursor!=fp: raise RuntimeError("material region end mismatch")
    return mats

def mat_id(name):
    m=re.match(r"^SMD_MAT_(\d+)_",name)
    return int(m.group(1)) if m else None

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=2: raise SystemExit("usage: -- <source.smd> <out.blend>")
    smd=pathlib.Path(argv[0]).expanduser().resolve()
    out=pathlib.Path(argv[1]).expanduser().resolve()
    mats=parse_materials(smd)
    seen=set()
    for bm in bpy.data.materials:
        mid=mat_id(bm.name)
        if mid is None or mid>=len(mats): continue
        m=mats[mid]
        bm["pt_material_schema"]="PT_SMD_MATERIAL_V4"
        bm["pt_material_id"]=mid
        bm["pt_self_illum"]=float(m["SelfIllum"])
        bm["pt_two_side"]=int(m["TwoSide"])
        bm["pt_blend_type"]=int(m["BlendType"])
        bm["pt_blink_color_enabled"]=bool(m["blink_enabled"])
        bm["pt_blink_r"]=float(m["Diffuse"][0])
        bm["pt_blink_g"]=float(m["Diffuse"][1])
        bm["pt_blink_b"]=float(m["Diffuse"][2])
        bm["pt_blink_time"]=int(m["blink_time"])

        # Preserved/diagnostic raw fields.
        bm["pt_transparency"]=float(m["Transparency"])
        bm["pt_shade"]=int(m["Shade"])
        bm["pt_texture_type"]=int(m["TextureType"])
        bm["pt_map_opacity"]=float(m["MapOpacity"])
        bm["pt_texture_clip"]=int(m["TextureClip"])
        bm["pt_source_use_state"]=int(m["UseState"])
        bm["pt_mesh_state"]=int(m["MeshState"])
        bm["pt_source_wind_mesh_bottom"]=int(m["WindMeshBottom"])
        bm["pt_source_texture_names"]=json.dumps(m["textures"],ensure_ascii=False)
        bm["pt_editable_fields"]="pt_self_illum,pt_two_side,pt_blend_type,pt_blink_color_enabled,pt_blink_r,pt_blink_g,pt_blink_b,pt_blink_time"

        try:
            bm.id_properties_ui("pt_blink_r").update(
                description="Blink red amplitude (smMATERIAL::Diffuse.r)", min=0.0, max=1.0)
            bm.id_properties_ui("pt_blink_g").update(
                description="Blink green amplitude (smMATERIAL::Diffuse.g)", min=0.0, max=1.0)
            bm.id_properties_ui("pt_blink_b").update(
                description="Blink blue amplitude (smMATERIAL::Diffuse.b)", min=0.0, max=1.0)
            bm.id_properties_ui("pt_blink_time").update(
                description="BLINK_TIME1..8; source maps to WindMeshBottom 9..16", min=1, max=8)
        except Exception:
            pass
        seen.add(mid)

    missing=sorted(set(range(len(mats)))-seen)
    if missing: raise RuntimeError(f"missing Blender material ids {missing}")
    out.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))
    print("PT_BLENDER_SUMMIT_MATERIAL_BIND_V4")
    print(f"smd={smd}")
    print(f"materials_bound={len(seen)}")
    print("editable_fields=['pt_self_illum', 'pt_two_side', 'pt_blend_type', 'pt_blink_color_enabled', 'pt_blink_r', 'pt_blink_g', 'pt_blink_b', 'pt_blink_time']")
    print(f"output_blend={out}")
    print("material_bind_check=PASS")

if __name__=="__main__":
    main()

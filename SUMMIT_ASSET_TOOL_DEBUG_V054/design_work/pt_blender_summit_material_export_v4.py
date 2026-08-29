#!/usr/bin/env python3
import bpy, pathlib, json, sys, re, math

def mat_id(name):
    m=re.match(r"^SMD_MAT_(\d+)_",name)
    return int(m.group(1)) if m else None

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=1: raise SystemExit("usage: -- <out.json>")
    out=pathlib.Path(argv[0]).expanduser().resolve()
    rows=[]
    for bm in bpy.data.materials:
        mid=bm.get("pt_material_id",mat_id(bm.name))
        if mid is None: continue
        if bm.get("pt_material_schema")!="PT_SMD_MATERIAL_V4":
            raise RuntimeError(f"{bm.name}: invalid schema")
        si=float(bm["pt_self_illum"]); ts=int(bm["pt_two_side"]); blend=int(bm["pt_blend_type"])
        enabled=bool(bm["pt_blink_color_enabled"])
        rgb=[float(bm["pt_blink_r"]),float(bm["pt_blink_g"]),float(bm["pt_blink_b"])]
        bt=int(bm["pt_blink_time"])
        if not math.isfinite(si) or si<0: raise RuntimeError("invalid self illum")
        if ts not in (0,1): raise RuntimeError("two_side must be 0/1")
        if not 0<=blend<=6: raise RuntimeError("blend_type must be 0..6")
        if not all(math.isfinite(x) and 0.0<=x<=1.0 for x in rgb):
            raise RuntimeError("blink RGB must be finite within 0..1")
        if not 1<=bt<=8: raise RuntimeError("blink_time must be 1..8")
        rows.append({
            "material_id":int(mid),"material_name":bm.name,
            "self_illum":si,"two_side":ts,"blend_type":blend,
            "blink_color_enabled":enabled,
            "blink_rgb":rgb,
            "blink_time":bt,
            "editable_fields":["self_illum","two_side","blend_type","blink_color_enabled","blink_rgb","blink_time"],
        })
    rows.sort(key=lambda x:x["material_id"])
    ids=[r["material_id"] for r in rows]
    if ids!=list(range(len(rows))): raise RuntimeError(f"noncontiguous material ids: {ids}")
    obj={"tool":"PT_BLENDER_SUMMIT_MATERIAL_EXPORT_V4",
         "schema":"PT_SMD_MATERIAL_EDIT_MANIFEST_V4",
         "editable_fields":["self_illum","two_side","blend_type","blink_color_enabled","blink_rgb","blink_time"],
         "materials":rows,"export_check":"PASS"}
    out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    print("PT_BLENDER_SUMMIT_MATERIAL_EXPORT_V4")
    print(f"material_count={len(rows)}")
    for r in rows:
        print(f"MATERIAL {r['material_id']} self_illum={r['self_illum']} two_side={r['two_side']} blend_type={r['blend_type']} blink_enabled={r['blink_color_enabled']} blink_rgb={r['blink_rgb']} blink_time={r['blink_time']}")
    print(f"manifest={out}")
    print("export_check=PASS")

if __name__=="__main__":
    main()

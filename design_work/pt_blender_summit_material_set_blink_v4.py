#!/usr/bin/env python3
import bpy, sys, pathlib, re

def mat_id(name):
    m=re.match(r"^SMD_MAT_(\d+)_",name)
    return int(m.group(1)) if m else None

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=7:
        raise SystemExit("usage: -- <material_id> <enabled:0|1> <r> <g> <b> <blink_time:1..8> <out.blend>")
    target=int(argv[0]); enabled=int(argv[1]); r=float(argv[2]); g=float(argv[3]); b=float(argv[4]); bt=int(argv[5])
    out=pathlib.Path(argv[6]).expanduser().resolve()
    if enabled not in (0,1): raise RuntimeError("enabled must be 0 or 1")
    if not all(0.0 <= x <= 1.0 for x in (r,g,b)): raise RuntimeError("blink RGB must be within 0..1")
    if not 1 <= bt <= 8: raise RuntimeError("blink_time must be 1..8")
    found=None
    for bm in bpy.data.materials:
        mid=bm.get("pt_material_id",mat_id(bm.name))
        if mid==target:
            found=bm; break
    if found is None: raise RuntimeError(f"material id {target} not found")
    if found.get("pt_material_schema")!="PT_SMD_MATERIAL_V4":
        raise RuntimeError("wrong material schema")
    old={
        "enabled":bool(found["pt_blink_color_enabled"]),
        "r":float(found["pt_blink_r"]),
        "g":float(found["pt_blink_g"]),
        "b":float(found["pt_blink_b"]),
        "blink_time":int(found["pt_blink_time"]),
    }
    found["pt_blink_color_enabled"]=bool(enabled)
    found["pt_blink_r"]=r; found["pt_blink_g"]=g; found["pt_blink_b"]=b
    found["pt_blink_time"]=bt
    out.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))
    print("PT_BLENDER_SUMMIT_MATERIAL_SET_BLINK_V4")
    print(f"material_id={target}")
    print(f"old={old}")
    print(f"new={{'enabled': {bool(enabled)}, 'r': {r}, 'g': {g}, 'b': {b}, 'blink_time': {bt}}}")
    print(f"output_blend={out}")
    print("material_set_check=PASS")

if __name__=="__main__":
    main()

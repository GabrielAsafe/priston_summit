#!/usr/bin/env python3
import bpy, sys, pathlib, re

def mat_id(name):
    m=re.match(r"^SMD_MAT_(\d+)_",name)
    return int(m.group(1)) if m else None

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=4:
        raise SystemExit("usage: -- <material_id> <property> <value> <out.blend>")
    target=int(argv[0]); prop=argv[1]; val=argv[2]; out=pathlib.Path(argv[3]).expanduser().resolve()
    allowed={"pt_self_illum":"float","pt_two_side":"int","pt_blend_type":"int"}
    if prop not in allowed: raise RuntimeError(f"property not editable in V3: {prop}")
    found=None
    for bm in bpy.data.materials:
        mid=bm.get("pt_material_id",mat_id(bm.name))
        if mid==target:
            found=bm; break
    if found is None: raise RuntimeError(f"material id {target} not found")
    if found.get("pt_material_schema")!="PT_SMD_MATERIAL_V3":
        raise RuntimeError("wrong material schema")
    if allowed[prop]=="float":
        new=float(val)
    else:
        new=int(val)
    if prop=="pt_two_side" and new not in (0,1):
        raise RuntimeError("pt_two_side must be 0 or 1")
    if prop=="pt_blend_type" and not 0<=new<=6:
        raise RuntimeError("pt_blend_type must be 0..6")
    old=found[prop]
    found[prop]=new
    out.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))
    print("PT_BLENDER_SUMMIT_MATERIAL_SET_V3")
    print(f"material_id={target}")
    print(f"property={prop}")
    print(f"old_value={old}")
    print(f"new_value={new}")
    print(f"output_blend={out}")
    print("material_set_check=PASS")

if __name__=="__main__":
    main()

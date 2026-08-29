#!/usr/bin/env python3
import bpy, pathlib, sys, math, json

TARGET="TbhC03"
EXPECTED_VERTS=92
EXPECTED_FACES=152
SCALE_XY=1.20
SCHEMA="PT_PIKEMAN_TBHC03_HIGH_WITNESS_V1"

def identity(obj,eps=1e-8):
    I=((1,0,0,0),(0,1,0,0),(0,0,1,0),(0,0,0,1))
    return all(abs(float(obj.matrix_world[r][c])-I[r][c])<=eps for r in range(4) for c in range(4))

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=1: raise SystemExit("usage: -- <out.blend>")
    out=pathlib.Path(argv[0]).expanduser().resolve()
    obj=bpy.data.objects.get(TARGET)
    if obj is None or obj.type!="MESH": raise RuntimeError(f"missing {TARGET}")
    if not identity(obj): raise RuntimeError("matrix_world must be identity")
    me=obj.data
    if len(me.vertices)!=EXPECTED_VERTS or len(me.polygons)!=EXPECTED_FACES:
        raise RuntimeError(f"unexpected counts {len(me.vertices)}/{len(me.polygons)}")
    if any(len(p.vertices)!=3 for p in me.polygons): raise RuntimeError("triangles required")

    before=[(float(v.co.x),float(v.co.y),float(v.co.z)) for v in me.vertices]
    cx=sum(p[0] for p in before)/len(before)
    cy=sum(p[1] for p in before)/len(before)
    cz=sum(p[2] for p in before)/len(before)
    for v in me.vertices:
        v.co.x=cx+(float(v.co.x)-cx)*SCALE_XY
        v.co.y=cy+(float(v.co.y)-cy)*SCALE_XY
    after=[(float(v.co.x),float(v.co.y),float(v.co.z)) for v in me.vertices]
    moved=[math.dist(a,b) for a,b in zip(before,after)]
    if not any(d>1e-9 for d in moved): raise RuntimeError("no movement")

    obj["pt_high_witness_schema"]=SCHEMA
    obj["pt_high_witness_original_coords_json"]=json.dumps(before,separators=(",",":"))
    obj["pt_high_witness_center_x"]=cx
    obj["pt_high_witness_center_y"]=cy
    obj["pt_high_witness_center_z"]=cz
    obj["pt_high_witness_scale_xy"]=SCALE_XY

    out.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))
    print("PT_PIKEMAN_HIGH_WITNESS_EDIT_V1")
    print(f"object={TARGET}")
    print(f"vertices={len(me.vertices)}")
    print(f"faces={len(me.polygons)}")
    print(f"scale_xy={SCALE_XY}")
    print(f"moved_vertices={sum(d>1e-9 for d in moved)}")
    print(f"max_move={max(moved):.9f}")
    print("topology_changed=False")
    print("uv_changed=False")
    print("material_changed=False")
    print("physique_changed=False")
    print(f"output_blend={out}")
    print("high_witness_edit_check=PASS")
if __name__=="__main__": main()

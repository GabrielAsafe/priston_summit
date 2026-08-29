#!/usr/bin/env python3
from __future__ import annotations
import bpy, pathlib, struct, sys, hashlib, json, math, os, tempfile

BASE_SHA="40b02371a872745d98abc558042b62f75cfb001a297e1bc36582e6c5968495a5"
BASE_SIZE=187504
TARGET="TbhC03"
EXPECTED=(92,152,152)
SCALE_XY=1.20
SCHEMA="PT_PIKEMAN_TBHC03_HIGH_WITNESS_V1"
BLINKLESS_TWOSIDE_OFFSET_REL=124

DFILE_HEADER_SIZE=556
OBJINFO_SIZE=40
OBJ3D_SIZE=2236
VERTEX_SIZE=24
FACE_SIZE=36
TM_OFFSET=240
NODE_NAME_OFFSET=172
NVERTEX_OFFSET=84
NFACE_OFFSET=88
NTEXLINK_OFFSET=92
MAT_GROUP_SIZE=88
MATERIAL_SIZE=320

def sha(d): return hashlib.sha256(d).hexdigest()
def i32(d,o): return struct.unpack_from("<i",d,o)[0]
def u16(d,o): return struct.unpack_from("<H",d,o)[0]
def cstr(b): return b.split(b"\0",1)[0].decode("latin1","replace")

def objtab(data):
    if not data.startswith(b"SMD Model data Ver 0.62"): raise RuntimeError("bad header")
    cnt=i32(data,24); rows=[]
    for idx in range(cnt):
        off=DFILE_HEADER_SIZE+idx*OBJINFO_SIZE
        name=cstr(data[off:off+32]); length=i32(data,off+32); point=i32(data,off+36)
        if not name or length<OBJ3D_SIZE or point<0 or point+length>len(data): raise RuntimeError("bad object table")
        rows.append({"name":name,"length":length,"point":point})
    return rows

def counts(data,row):
    p=row["point"]
    if cstr(data[p+NODE_NAME_OFFSET:p+NODE_NAME_OFFSET+32])!=row["name"]: raise RuntimeError("NodeName mismatch")
    return i32(data,p+NVERTEX_OFFSET),i32(data,p+NFACE_OFFSET),i32(data,p+NTEXLINK_OFFSET)

def skel_tm(data):
    out={}
    for r in objtab(data):
        p=r["point"]
        vals=list(struct.unpack_from("<16i",data,p+TM_OFFSET))
        if abs(vals[15]-256)>2: raise RuntimeError(f"bad Tm {r['name']}")
        out[r["name"]]=vals
    return out

def forward(local,tm):
    x,y,z=local
    return (
      ((x*tm[0]+y*tm[4]+z*tm[8])/256.0+tm[12])/256.0,
      ((x*tm[1]+y*tm[5]+z*tm[9])/256.0+tm[13])/256.0,
      ((x*tm[2]+y*tm[6]+z*tm[10])/256.0+tm[14])/256.0,
    )

def inv3(a):
    a00,a01,a02=a[0]; a10,a11,a12=a[1]; a20,a21,a22=a[2]
    c00=a11*a22-a12*a21; c01=-(a10*a22-a12*a20); c02=a10*a21-a11*a20
    c10=-(a01*a22-a02*a21); c11=a00*a22-a02*a20; c12=-(a00*a21-a01*a20)
    c20=a01*a12-a02*a11; c21=-(a00*a12-a02*a10); c22=a00*a11-a01*a10
    det=a00*c00+a01*c01+a02*c02
    if abs(det)<1e-12: raise RuntimeError("singular")
    return ((c00/det,c10/det,c20/det),(c01/det,c11/det,c21/det),(c02/det,c12/det,c22/det))

def world_to_local(world,tm):
    A=((tm[0]/256,tm[1]/256,tm[2]/256),(tm[4]/256,tm[5]/256,tm[6]/256),(tm[8]/256,tm[9]/256,tm[10]/256))
    iv=inv3(A)
    q=(world[0]-tm[12]/256,world[1]-tm[13]/256,world[2]-tm[14]/256)
    lx=q[0]*iv[0][0]+q[1]*iv[1][0]+q[2]*iv[2][0]
    ly=q[0]*iv[0][1]+q[1]*iv[1][1]+q[2]*iv[2][1]
    lz=q[0]*iv[0][2]+q[1]*iv[1][2]+q[2]*iv[2][2]
    return int(round(lx*256)),int(round(ly*256)),int(round(lz*256))

def rigid_bone(obj,v,names):
    vals=[]; bad=[]
    for g in v.groups:
        if g.weight<=1e-6: continue
        name=obj.vertex_groups[g.group].name
        (vals if name in names else bad).append((name,float(g.weight)))
    if bad or len(vals)!=1 or vals[0][1]<0.999: raise RuntimeError(f"vertex {v.index}: bad groups vals={vals} bad={bad}")
    return vals[0][0]

def material0_twoside_offset(data):
    mc=i32(data,28); mp=i32(data,32); fp=i32(data,36)
    if mc!=2 or i32(data,mp+8)!=2: raise RuntimeError("material count")
    m0=mp+MAT_GROUP_SIZE
    # verify material region structure enough to ensure offset belongs to raw m0
    cursor=m0
    for _ in range(2):
        iu=i32(data,cursor); tc=i32(data,cursor+4); ac=i32(data,cursor+304)
        if not 0<=tc<=8 or not 0<=ac<=32: raise RuntimeError("bad counters")
        cursor += MATERIAL_SIZE
        if iu:
            sl=i32(data,cursor); cursor += 4+sl
    if cursor!=fp: raise RuntimeError("material region mismatch")
    return m0+124

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
    try: t.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+"\n",encoding="utf-8"); os.replace(t,path)
    finally:
        if t.exists(): t.unlink()

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=5:
        raise SystemExit("usage: -- <base-medium.smd> <m4.smb> <outA.smd> <outB.smd> <manifest_dir>")
    base=pathlib.Path(argv[0]).expanduser().resolve()
    skel=pathlib.Path(argv[1]).expanduser().resolve()
    outA=pathlib.Path(argv[2]).expanduser().resolve()
    outB=pathlib.Path(argv[3]).expanduser().resolve()
    md=pathlib.Path(argv[4]).expanduser().resolve()
    raw=base.read_bytes()
    if len(raw)!=BASE_SIZE or sha(raw)!=BASE_SHA: raise RuntimeError("base must be approved medium geometry artifact")

    row=next((r for r in objtab(raw) if r["name"]==TARGET),None)
    if row is None: raise RuntimeError("TbhC03 missing")
    if counts(raw,row)!=EXPECTED: raise RuntimeError(f"unexpected source counts {counts(raw,row)}")

    obj=bpy.data.objects.get(TARGET)
    if obj is None or obj.type!="MESH": raise RuntimeError("Blender TbhC03 missing")
    if obj.get("pt_high_witness_schema")!=SCHEMA: raise RuntimeError("wrong helper schema")
    me=obj.data
    if (len(me.vertices),len(me.polygons))!=EXPECTED[:2]: raise RuntimeError("topology count changed")
    if any(len(p.vertices)!=3 for p in me.polygons): raise RuntimeError("nontriangle")

    vstart=row["point"]+OBJ3D_SIZE
    fstart=vstart+EXPECTED[0]*VERTEX_SIZE
    for i,p in enumerate(me.polygons):
        sf=(u16(raw,fstart+i*FACE_SIZE),u16(raw,fstart+i*FACE_SIZE+2),u16(raw,fstart+i*FACE_SIZE+4))
        if tuple(int(x) for x in p.vertices)!=sf: raise RuntimeError(f"face {i} order changed")

    before=json.loads(obj["pt_high_witness_original_coords_json"])
    before=[tuple(float(x) for x in p) for p in before]
    cx=float(obj["pt_high_witness_center_x"]); cy=float(obj["pt_high_witness_center_y"]); cz=float(obj["pt_high_witness_center_z"])
    edited=[(float(v.co.x),float(v.co.y),float(v.co.z)) for v in me.vertices]
    expected=[(cx+(p[0]-cx)*SCALE_XY,cy+(p[1]-cy)*SCALE_XY,p[2]) for p in before]
    op_err=[math.dist(a,b) for a,b in zip(edited,expected)]
    if max(op_err)>1e-6: raise RuntimeError(f"helper transform mismatch {max(op_err)}")

    tm=skel_tm(skel.read_bytes())
    if len(tm)!=58: raise RuntimeError("skeleton count")
    names=set(tm)
    bones=[rigid_bone(obj,v,names) for v in me.vertices]

    A=bytearray(raw); allowed=set(); fits=[]
    for idx,(world,bone) in enumerate(zip(edited,bones)):
        loc=world_to_local(world,tm[bone]); post=forward(loc,tm[bone]); fits.append(math.dist(world,post))
        off=vstart+idx*VERTEX_SIZE; struct.pack_into("<3i",A,off,*loc); allowed.update(range(off,off+12))
    if max(fits)>0.01: raise RuntimeError(f"fit too large {max(fits)}")
    changedA=[i for i,(a,b) in enumerate(zip(raw,A)) if a!=b]
    outside=[i for i in changedA if i not in allowed]
    if outside: raise RuntimeError("A changed outside high vertex XYZ")
    if len(A)!=len(raw): raise RuntimeError("A size changed")

    # A is both medium + high widened, material baseline TwoSide=0.
    twooff=material0_twoside_offset(bytes(A))
    if i32(A,twooff)!=0: raise RuntimeError("A material0 TwoSide baseline not zero")
    B=bytearray(A); struct.pack_into("<I",B,twooff,1)
    diffAB=[i for i,(a,b) in enumerate(zip(A,B)) if a!=b]
    if len(diffAB)!=1 or diffAB[0] not in range(twooff,twooff+4): raise RuntimeError(f"A/B difference not isolated: {diffAB}")
    if i32(B,twooff)!=1: raise RuntimeError("B TwoSide verify")

    atomic_write(outA,bytes(A)); atomic_write(outB,bytes(B))
    shaA=sha(bytes(A)); shaB=sha(bytes(B))
    common={
      "tool":"PT_PIKEMAN_HIGHLOD_TWOSIDE_WITNESS_BUILD_V1",
      "profile_id":"pikeman_c01_m4",
      "base_medium_geometry_sha256":BASE_SHA,
      "target_high_object":TARGET,
      "source_counts":{"vertices":92,"faces":152,"texlinks":152},
      "output_counts":{"vertices":92,"faces":152,"texlinks":152},
      "high_geometry_operation":"SCALE_XY_1.20_AROUND_EMBEDDED_BLENDER_BASELINE_CENTROID",
      "controlled_operation_max_error":max(op_err),
      "max_vertex_fit_error":max(fits),
      "mean_vertex_fit_error":sum(fits)/len(fits),
      "outside_high_vertex_xyz_changed_byte_count":len(outside),
      "geometry_file_size_preserved":True,
      "ab_changed_byte_count":len(diffAB),
      "ab_only_material0_twoside_changed":True,
      "material0_twoside_offset":twooff,
      "runtime_installed":False,
      "build_check":"PASS"
    }
    ma={**common,"variant":"A","TwoSide":0,"artifact":str(outA),"artifact_sha256":shaA,"artifact_size":len(A)}
    mb={**common,"variant":"B","TwoSide":1,"artifact":str(outB),"artifact_sha256":shaB,"artifact_size":len(B)}
    atomic_json(md/"TmbC01_medium_high_wide_twoside0_v1_build_manifest.json",ma)
    atomic_json(md/"TmbC01_medium_high_wide_twoside1_v1_build_manifest.json",mb)

    print("PT_PIKEMAN_HIGHLOD_TWOSIDE_WITNESS_BUILD_V1")
    print("base_medium_geometry_preserved=True")
    print("high_object=TbhC03")
    print("high_counts=92/152/152")
    print(f"controlled_operation_max_error={max(op_err):.12g}")
    print(f"max_vertex_fit_error={max(fits):.12g}")
    print(f"outside_high_vertex_xyz_changed_byte_count={len(outside)}")
    print(f"A_sha256={shaA}")
    print("A_material0_TwoSide=0")
    print(f"B_sha256={shaB}")
    print("B_material0_TwoSide=1")
    print(f"AB_changed_byte_count={len(diffAB)}")
    print("AB_only_material0_twoside_changed=True")
    print("runtime_installed=False")
    print("build_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as e:
        print(f"build_check=FAIL: {type(e).__name__}: {e}")
        raise

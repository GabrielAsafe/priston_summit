#!/usr/bin/env python3
from __future__ import annotations

import bpy, pathlib, struct, sys, hashlib, json, math, os, tempfile

SOURCE_SHA = "9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SOURCE_SIZE = 187504
TARGET = "TbmC03"
EXPECTED_VERTS = 74
EXPECTED_FACES = 122
EXPECTED_TEXLINKS = 122
SCALE_XY = 1.20
SCHEMA = "PT_PIKEMAN_TBMC03_GEOMETRY_VERTEX_GATE_V1_1"

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

def sha256_bytes(d): return hashlib.sha256(d).hexdigest()
def i32(d,o): return struct.unpack_from("<i",d,o)[0]
def u16(d,o): return struct.unpack_from("<H",d,o)[0]
def cstr(raw): return raw.split(b"\0",1)[0].decode("latin1","replace")

def object_table(data):
    if not data.startswith(b"SMD Model data Ver 0.62"):
        raise RuntimeError("unexpected SMD/SMB header")
    cnt=i32(data,24)
    if cnt<=0 or cnt>10000: raise RuntimeError(f"invalid ObjCounter={cnt}")
    rows=[]
    for idx in range(cnt):
        off=DFILE_HEADER_SIZE+idx*OBJINFO_SIZE
        name=cstr(data[off:off+32]); length=i32(data,off+32); point=i32(data,off+36)
        if not name or length<OBJ3D_SIZE or point<0 or point+length>len(data):
            raise RuntimeError(f"invalid object table row {idx}")
        rows.append({"index":idx,"name":name,"length":length,"point":point})
    return rows

def raw_obj_counts(data,row):
    p=row["point"]
    if cstr(data[p+NODE_NAME_OFFSET:p+NODE_NAME_OFFSET+32])!=row["name"]:
        raise RuntimeError("raw NodeName mismatch")
    return i32(data,p+NVERTEX_OFFSET),i32(data,p+NFACE_OFFSET),i32(data,p+NTEXLINK_OFFSET)

def parse_skeleton_tm(data):
    out={}
    for row in object_table(data):
        p=row["point"]
        if cstr(data[p+NODE_NAME_OFFSET:p+NODE_NAME_OFFSET+32])!=row["name"]:
            raise RuntimeError("skeleton NodeName mismatch")
        vals=list(struct.unpack_from("<16i",data,p+TM_OFFSET))
        if abs(vals[15]-256)>2:
            raise RuntimeError(f"{row['name']}: unexpected Tm._44={vals[15]}")
        out[row["name"]]=vals
    return out

def forward_world(local_int,tm):
    x,y,z=local_int
    wx=((x*tm[0]+y*tm[4]+z*tm[8])/256.0+tm[12])/256.0
    wy=((x*tm[1]+y*tm[5]+z*tm[9])/256.0+tm[13])/256.0
    wz=((x*tm[2]+y*tm[6]+z*tm[10])/256.0+tm[14])/256.0
    return wx,wy,wz

def inv3(a):
    a00,a01,a02=a[0]; a10,a11,a12=a[1]; a20,a21,a22=a[2]
    c00=a11*a22-a12*a21; c01=-(a10*a22-a12*a20); c02=a10*a21-a11*a20
    c10=-(a01*a22-a02*a21); c11=a00*a22-a02*a20; c12=-(a00*a21-a01*a20)
    c20=a01*a12-a02*a11; c21=-(a00*a12-a02*a10); c22=a00*a11-a01*a10
    det=a00*c00+a01*c01+a02*c02
    if abs(det)<1e-12: raise RuntimeError("singular bone rotation matrix")
    return (
        (c00/det,c10/det,c20/det),
        (c01/det,c11/det,c21/det),
        (c02/det,c12/det,c22/det),
    )

def world_to_local_int(world,tm):
    A=((tm[0]/256.0,tm[1]/256.0,tm[2]/256.0),
       (tm[4]/256.0,tm[5]/256.0,tm[6]/256.0),
       (tm[8]/256.0,tm[9]/256.0,tm[10]/256.0))
    inv=inv3(A)
    q=(world[0]-tm[12]/256.0,world[1]-tm[13]/256.0,world[2]-tm[14]/256.0)
    lx=q[0]*inv[0][0]+q[1]*inv[1][0]+q[2]*inv[2][0]
    ly=q[0]*inv[0][1]+q[1]*inv[1][1]+q[2]*inv[2][1]
    lz=q[0]*inv[0][2]+q[1]*inv[1][2]+q[2]*inv[2][2]
    return int(round(lx*256.0)),int(round(ly*256.0)),int(round(lz*256.0))

def rigid_bone_for_vertex(obj,v,skeleton_names):
    vals=[]
    invalid=[]
    for g in v.groups:
        if g.weight<=1e-6: continue
        name=obj.vertex_groups[g.group].name
        (vals if name in skeleton_names else invalid).append((name,float(g.weight)))
    if invalid: raise RuntimeError(f"vertex {v.index}: non-skeleton groups {invalid}")
    if len(vals)!=1 or vals[0][1]<0.999:
        raise RuntimeError(f"vertex {v.index}: expected exactly one rigid bone, got {vals}")
    return vals[0][0]

def matrix_world_is_identity(obj,eps=1e-8):
    ident=((1,0,0,0),(0,1,0,0),(0,0,1,0),(0,0,0,1))
    return all(abs(float(obj.matrix_world[r][c])-ident[r][c])<=eps for r in range(4) for c in range(4))

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
        t.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
        os.replace(t,path)
    finally:
        if t.exists(): t.unlink()

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=4:
        raise SystemExit("usage: -- <source.smd> <skeleton.smb> <out.smd> <manifest.json>")
    src=pathlib.Path(argv[0]).expanduser().resolve()
    skel=pathlib.Path(argv[1]).expanduser().resolve()
    outp=pathlib.Path(argv[2]).expanduser().resolve()
    manp=pathlib.Path(argv[3]).expanduser().resolve()

    raw=src.read_bytes()
    if len(raw)!=SOURCE_SIZE or sha256_bytes(raw)!=SOURCE_SHA:
        raise RuntimeError("registered Pikeman TmbC01.smd baseline mismatch")

    rows=object_table(raw)
    row=next((r for r in rows if r["name"]==TARGET),None)
    if row is None: raise RuntimeError(f"source object {TARGET} missing")
    nv,nf,nt=raw_obj_counts(raw,row)
    if (nv,nf,nt)!=(EXPECTED_VERTS,EXPECTED_FACES,EXPECTED_TEXLINKS):
        raise RuntimeError(f"{TARGET}: source counts {(nv,nf,nt)} != expected")

    obj=bpy.data.objects.get(TARGET)
    if obj is None or obj.type!="MESH": raise RuntimeError(f"Blender object {TARGET} missing")
    if obj.get("pt_geometry_gate_schema")!=SCHEMA:
        raise RuntimeError("artist blend is not V1.1 controlled gate output")
    if not matrix_world_is_identity(obj): raise RuntimeError("target matrix_world not identity")

    me=obj.data
    if len(me.vertices)!=nv or len(me.polygons)!=nf:
        raise RuntimeError("topology count changed")
    if any(len(p.vertices)!=3 for p in me.polygons):
        raise RuntimeError("non-triangle polygon found")

    vertex_start=row["point"]+OBJ3D_SIZE
    face_start=vertex_start+nv*VERTEX_SIZE
    for i,p in enumerate(me.polygons):
        src_face=(u16(raw,face_start+i*FACE_SIZE+0),
                  u16(raw,face_start+i*FACE_SIZE+2),
                  u16(raw,face_start+i*FACE_SIZE+4))
        if tuple(int(x) for x in p.vertices)!=src_face:
            raise RuntimeError(f"face {i}: topology/order changed")

    # V1.1 operation proof is based on the exact Blender baseline stored by helper.
    before_json=obj.get("pt_geometry_gate_original_coords_json")
    if not isinstance(before_json,str):
        raise RuntimeError("missing embedded original Blender coordinates")
    before=json.loads(before_json)
    if len(before)!=nv or any(len(p)!=3 for p in before):
        raise RuntimeError("invalid embedded original coordinates")
    before=[tuple(float(x) for x in p) for p in before]

    cx=float(obj["pt_geometry_gate_center_x"])
    cy=float(obj["pt_geometry_gate_center_y"])
    cz=float(obj["pt_geometry_gate_center_z"])
    calc_center=(sum(p[0] for p in before)/nv,sum(p[1] for p in before)/nv,sum(p[2] for p in before)/nv)
    center_err=math.dist((cx,cy,cz),calc_center)
    if center_err>1e-9:
        raise RuntimeError(f"embedded centroid mismatch: {center_err}")

    edited=[(float(v.co.x),float(v.co.y),float(v.co.z)) for v in me.vertices]
    expected=[(cx+(p[0]-cx)*SCALE_XY,cy+(p[1]-cy)*SCALE_XY,p[2]) for p in before]
    op_err=[math.dist(a,b) for a,b in zip(edited,expected)]
    max_op=max(op_err); mean_op=sum(op_err)/nv
    if max_op>1e-6:
        raise RuntimeError(f"artist geometry differs from controlled helper transform: {max_op}")

    skel_raw=skel.read_bytes()
    tm_by_bone=parse_skeleton_tm(skel_raw)
    if len(tm_by_bone)!=58: raise RuntimeError(f"expected 58 skeleton objects, got {len(tm_by_bone)}")
    skeleton_names=set(tm_by_bone)

    bone_names=[rigid_bone_for_vertex(obj,v,skeleton_names) for v in me.vertices]

    out=bytearray(raw); allowed=set(); fit_errors=[]; moved_count=0
    for idx,(world,bone,old) in enumerate(zip(edited,bone_names,before)):
        local_int=world_to_local_int(world,tm_by_bone[bone])
        fwd=forward_world(local_int,tm_by_bone[bone])
        fit=math.dist(world,fwd); fit_errors.append(fit)
        if math.dist(world,old)>1e-9: moved_count+=1
        off=vertex_start+idx*VERTEX_SIZE
        struct.pack_into("<3i",out,off,*local_int)
        allowed.update(range(off,off+12))

    max_fit=max(fit_errors); mean_fit=sum(fit_errors)/nv
    if max_fit>0.01:
        raise RuntimeError(f"local integer forward-fit too large: {max_fit}")
    if moved_count==0: raise RuntimeError("no vertices moved")

    changed=[i for i,(a,b) in enumerate(zip(raw,out)) if a!=b]
    outside=[i for i in changed if i not in allowed]
    if outside: raise RuntimeError(f"bytes changed outside target vertex XYZ: {outside[:20]}")
    if len(out)!=len(raw): raise RuntimeError("file size changed")

    atomic_write(outp,bytes(out)); outsha=sha256_bytes(bytes(out))
    report={
        "tool":"PT_PIKEMAN_GEOMETRY_VERTEX_BUILD_V1_1",
        "profile_id":"pikeman_c01_m4",
        "asset_set_id":"CHARSET-PIKEMAN-C01-M4",
        "roundtrip_target":"char/tmABCD/TmbC01.smd",
        "source":str(src),"source_sha256":SOURCE_SHA,"source_size":SOURCE_SIZE,
        "skeleton":str(skel),"target_object":TARGET,
        "source_counts":{"vertices":nv,"faces":nf,"texlinks":nt},
        "output_counts":{"vertices":nv,"faces":nf,"texlinks":nt},
        "geometry_operation":{"type":"SCALE_XY_AROUND_EMBEDDED_BLENDER_BASELINE_CENTROID",
                              "scale_xy":SCALE_XY,"centroid":[cx,cy,cz]},
        "embedded_original_blender_coords":True,
        "controlled_operation_max_error":max_op,
        "controlled_operation_mean_error":mean_op,
        "moved_vertices":moved_count,
        "rigid_bone_count":len(set(bone_names)),
        "rigid_bones":sorted(set(bone_names)),
        "max_vertex_fit_error":max_fit,
        "mean_vertex_fit_error":mean_fit,
        "artifact":str(outp),"artifact_sha256":outsha,"artifact_size":len(out),
        "changed_byte_count":len(changed),"changed_offsets":changed,
        "outside_target_vertex_xyz_changed_byte_count":len(outside),
        "file_size_preserved":True,
        "topology_serialized_from_source_unchanged":True,
        "uv_serialized_from_source_unchanged":True,
        "materials_serialized_from_source_unchanged":True,
        "physique_names_not_reserialized":True,
        "runtime_installed":False,"build_check":"PASS",
    }
    atomic_json(manp,report)

    print("PT_PIKEMAN_GEOMETRY_VERTEX_BUILD_V1_1")
    print(f"source_sha256={SOURCE_SHA}")
    print(f"object={TARGET}")
    print(f"source_counts={nv}/{nf}/{nt}")
    print(f"output_counts={nv}/{nf}/{nt}")
    print(f"operation=SCALE_XY_AROUND_EMBEDDED_BLENDER_BASELINE_CENTROID factor={SCALE_XY}")
    print(f"controlled_operation_max_error={max_op:.12g}")
    print(f"moved_vertices={moved_count}")
    print(f"rigid_bone_count={len(set(bone_names))}")
    print(f"max_vertex_fit_error={max_fit:.12g}")
    print(f"mean_vertex_fit_error={mean_fit:.12g}")
    print(f"changed_byte_count={len(changed)}")
    print(f"outside_target_vertex_xyz_changed_byte_count={len(outside)}")
    print("topology_serialized_from_source_unchanged=True")
    print("uv_serialized_from_source_unchanged=True")
    print("materials_serialized_from_source_unchanged=True")
    print("file_size_preserved=True")
    print(f"artifact={outp}")
    print(f"artifact_sha256={outsha}")
    print(f"manifest={manp}")
    print("runtime_installed=False")
    print("build_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as exc:
        print(f"build_check=FAIL: {type(exc).__name__}: {exc}")
        raise

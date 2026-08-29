#!/usr/bin/env python3
"""
Priestess C01 HIGH append-safe topology exporter V1.

Target:
  MshC01 (HIGH LOD)
  pristine source: 173 vertices / 331 faces / 331 TexLinks

Rules:
  - original 173 vertices must remain present, same indices, same bind positions
    and same rigid Physique bones
  - new vertices may be appended
  - all polygons must be triangles
  - faces/TexLinks are rebuilt from Blender
  - polygon.material_index must be a material ID already present in pristine MshC01
  - source material -> hTexture mapping is learned from pristine SMD
  - original smVERTEX records are byte-preserved
  - new normals: BONE_ROW + equal-weight geometric direction
  - other 14 SMD object blocks remain byte-identical

This writes a NEW SMD only; it never touches the game client.
"""

import argparse, hashlib, json, math, statistics, struct, sys
from collections import defaultdict
from pathlib import Path

import bpy
from mathutils import Vector

HEADER=b"SMD Model data Ver 0.62"
SOURCE_SHA="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
TARGET="MshC01"
SOURCE_NV=173
SOURCE_NF=331
SOURCE_NT=331

SZ_HDR=556; SZ_INFO=40; SZ_OBJ=2236; SZ_VERT=24; SZ_FACE=36; SZ_TEX=32
SZ_ROT=20; SZ_POS=16; SZ_SCALE=16; SZ_MATRIX=64

O_VERTEX_PTR=4; O_FACE_PTR=8; O_TEX_PTR=12; O_PHYS_PTR=16
O_VERTEX_CAP=76; O_FACE_CAP=80; O_NV=84; O_NF=88; O_NT=92
O_NAME=172; O_TM=240; O_NR=684; O_NP=688; O_NS=692
F_TEX_PTR=32
T_U=0; T_V=12; T_HTEX=24; T_NEXT=28


def i32(b,o): return struct.unpack_from("<i",b,o)[0]
def u32(b,o): return struct.unpack_from("<I",b,o)[0]
def cstr(b): return b.split(b"\0",1)[0].decode("latin-1",errors="replace")
def sha(b): return hashlib.sha256(b).hexdigest()


def parse_file(data):
    if len(data)<SZ_HDR or not data.startswith(HEADER):
        raise ValueError("unexpected SMD/SMB header")
    n=i32(data,24)
    infos=[]
    for idx in range(n):
        off=SZ_HDR+idx*SZ_INFO
        r=data[off:off+SZ_INFO]
        infos.append(dict(index=idx,table_off=off,name=cstr(r[:32]),
                          length=i32(r,32),start=i32(r,36)))
    for idx,inf in enumerate(infos):
        if inf["length"]<=0: raise ValueError(f"{inf['name']}: bad length")
        end=inf["start"]+inf["length"]
        if idx+1<len(infos):
            if end!=infos[idx+1]["start"]:
                raise ValueError(f"{inf['name']}: non-contiguous objects")
        elif end!=len(data):
            raise ValueError("last object does not close file")
    return infos


def obj_layout(start,nv,nf,nt,nr,np,ns,phys):
    v=start+SZ_OBJ
    f=v+nv*SZ_VERT
    t=f+nf*SZ_FACE
    r=t+nt*SZ_TEX
    p=r+nr*SZ_ROT
    s=p+np*SZ_POS
    prev=s+ns*SZ_SCALE
    phy=prev+nr*SZ_MATRIX if phys else None
    end=phy+nv*32 if phys else prev+nr*SZ_MATRIX
    return dict(vertices=v,faces=f,tex=t,rot=r,pos=p,scale=s,prev=prev,
                physique=phy,end=end)


def parse_object(data,infos,name):
    inf=next((x for x in infos if x["name"]==name),None)
    if not inf: raise ValueError(f"object {name!r} missing")
    raw=data[inf["start"]:inf["start"]+SZ_OBJ]
    if cstr(raw[O_NAME:O_NAME+32])!=name:
        raise ValueError(f"{name}: NodeName mismatch")
    nv=i32(raw,O_NV); nf=i32(raw,O_NF); nt=i32(raw,O_NT)
    nr=i32(raw,O_NR); np=i32(raw,O_NP); ns=i32(raw,O_NS)
    phys=u32(raw,O_PHYS_PTR)!=0
    lay=obj_layout(inf["start"],nv,nf,nt,nr,np,ns,phys)
    if lay["end"]!=inf["start"]+inf["length"]:
        raise ValueError(f"{name}: length mismatch")

    verts=[struct.unpack_from("<6i",data,lay["vertices"]+i*SZ_VERT)
           for i in range(nv)]
    faces=[]
    for i in range(nf):
        off=lay["faces"]+i*SZ_FACE
        r=data[off:off+SZ_FACE]
        a,b,c,mat=struct.unpack_from("<4H",r,0)
        faces.append(dict(raw=r,verts=(a,b,c),material=mat,
                          lp_tex=u32(r,F_TEX_PTR)))
    tex=[]
    for i in range(nt):
        off=lay["tex"]+i*SZ_TEX
        r=data[off:off+SZ_TEX]
        tex.append(dict(raw=r,u=struct.unpack_from("<3f",r,T_U),
                        v=struct.unpack_from("<3f",r,T_V),
                        hTexture=u32(r,T_HTEX),NextTex=u32(r,T_NEXT)))
    bones=[]
    if phys:
        for i in range(nv):
            off=lay["physique"]+i*32
            bones.append(cstr(data[off:off+32]))
    return dict(info=inf,raw=raw,nv=nv,nf=nf,nt=nt,nr=nr,np=np,ns=ns,
                phys=phys,layout=lay,vertices=verts,faces=faces,texlinks=tex,
                bones=bones,texbase=u32(raw,O_TEX_PTR))


def parse_skeleton(data):
    infos=parse_file(data)
    out={}
    for inf in infos:
        raw=data[inf["start"]:inf["start"]+SZ_OBJ]
        name=cstr(raw[O_NAME:O_NAME+32])
        if name in out: raise ValueError(f"duplicate bone {name!r}")
        out[name]=struct.unpack_from("<16i",raw,O_TM)
    return out


def m(tm,r,c): return tm[(r-1)*4+(c-1)]


def world_fixed(local,tm):
    x,y,z=map(int,local)
    rx=x*m(tm,1,1)+y*m(tm,2,1)+z*m(tm,3,1)
    ry=x*m(tm,1,2)+y*m(tm,2,2)+z*m(tm,3,2)
    rz=x*m(tm,1,3)+y*m(tm,2,3)+z*m(tm,3,3)
    return ((rx>>8)+m(tm,4,1),(ry>>8)+m(tm,4,2),(rz>>8)+m(tm,4,3))


def inv_row3(vec,tm):
    a,b,c=m(tm,1,1)/256.,m(tm,1,2)/256.,m(tm,1,3)/256.
    d,e,f=m(tm,2,1)/256.,m(tm,2,2)/256.,m(tm,2,3)/256.
    g,h,i=m(tm,3,1)/256.,m(tm,3,2)/256.,m(tm,3,3)/256.
    det=a*(e*i-f*h)-b*(d*i-f*g)+c*(d*h-e*g)
    if abs(det)<1e-12: raise ValueError("singular bone matrix")
    inv=(((e*i-f*h)/det,(c*h-b*i)/det,(b*f-c*e)/det),
         ((f*g-d*i)/det,(a*i-c*g)/det,(c*d-a*f)/det),
         ((d*h-e*g)/det,(b*g-a*h)/det,(a*e-b*d)/det))
    x,y,z=vec
    return (x*inv[0][0]+y*inv[1][0]+z*inv[2][0],
            x*inv[0][1]+y*inv[1][1]+z*inv[2][1],
            x*inv[0][2]+y*inv[1][2]+z*inv[2][2])


def inverse_point(world,tm):
    return inv_row3((world[0]-m(tm,4,1)/256.,
                     world[1]-m(tm,4,2)/256.,
                     world[2]-m(tm,4,3)/256.),tm)


def fit_local(world,tm,radius=4):
    q=inverse_point(world,tm)
    center=tuple(int(round(x*256.)) for x in q)
    best=None
    for dx in range(-radius,radius+1):
      for dy in range(-radius,radius+1):
       for dz in range(-radius,radius+1):
        cand=(center[0]+dx,center[1]+dy,center[2]+dz)
        wf=world_fixed(cand,tm)
        represented=tuple(x/256. for x in wf)
        err2=sum((represented[k]-world[k])**2 for k in range(3))
        score=(err2,cand)
        if best is None or score<best[0]:
            best=(score,cand,represented)
    return best[1],best[2],math.sqrt(best[0][0])


def normalize(v):
    mag=math.sqrt(sum(float(x)*float(x) for x in v))
    if mag<=1e-20: raise ValueError("zero normal")
    return tuple(float(x)/mag for x in v)


def tri_normal(coords):
    a,b,c=map(Vector,coords)
    n=(b-a).cross(c-a)
    if n.length<=1e-12: raise ValueError("degenerate triangle")
    n.normalize()
    return (float(n.x),float(n.y),float(n.z))


def equal_normals(mesh):
    acc=[Vector((0,0,0)) for _ in mesh.vertices]
    cnt=[0]*len(mesh.vertices)
    for p in mesh.polygons:
        ids=[int(v) for v in p.vertices]
        coords=[tuple(float(x) for x in mesh.vertices[v].co) for v in ids]
        n=Vector(tri_normal(coords))
        for v in ids:
            acc[v]+=n; cnt[v]+=1
    out=[]
    for vi,(a,n) in enumerate(zip(acc,cnt)):
        if n==0 or a.length<=1e-12:
            raise ValueError(f"vertex {vi}: no geometric normal")
        a.normalize()
        out.append((float(a.x),float(a.y),float(a.z)))
    return out


def rigid_bone(obj,vi):
    rows=[(obj.vertex_groups[e.group].name,float(e.weight))
          for e in obj.data.vertices[vi].groups if e.weight>1e-8]
    if len(rows)!=1 or abs(rows[0][1]-1.0)>1e-5:
        raise ValueError(f"vertex {vi}: expected one rigid group weight 1")
    return rows[0][0]


def bone_slot(name):
    r=name.encode("latin-1")
    if len(r)>=32: raise ValueError(f"bone name too long: {name!r}")
    return r+b"\0"*(32-len(r))


def face_blob(template,verts,material,texptr):
    r=bytearray(template)
    struct.pack_into("<3H",r,0,*verts)
    struct.pack_into("<H",r,6,material)
    struct.pack_into("<I",r,F_TEX_PTR,texptr)
    return bytes(r)


def tex_blob(template,uvs,htex):
    r=bytearray(template)
    struct.pack_into("<3f",r,T_U,*(x[0] for x in uvs))
    struct.pack_into("<3f",r,T_V,*(x[1] for x in uvs))
    struct.pack_into("<I",r,T_HTEX,htex)
    struct.pack_into("<I",r,T_NEXT,0)
    return bytes(r)


def blender_uvs_smd(mesh,p,uvl):
    return [(float(uvl.data[li].uv.x),1.0-float(uvl.data[li].uv.y))
            for li in p.loop_indices]


def material_templates(src_obj):
    base=src_obj["texbase"]
    by_mat={}
    hsets=defaultdict(set)
    for f in src_obj["faces"]:
        d=f["lp_tex"]-base
        if d<0 or d%SZ_TEX:
            raise ValueError("source face has invalid TexLink pointer")
        ti=d//SZ_TEX
        if not 0<=ti<src_obj["nt"]:
            raise ValueError("source TexLink index out of range")
        tex=src_obj["texlinks"][ti]
        if tex["NextTex"]!=0:
            raise ValueError("HIGH source has multi-layer TexLink")
        mat=f["material"]
        hsets[mat].add(tex["hTexture"])
        by_mat.setdefault(mat,(f["raw"],tex["raw"]))
    for mat,hs in hsets.items():
        if len(hs)!=1:
            raise ValueError(f"material {mat}: multiple source hTexture values {hs}")
    return by_mat,{mat:next(iter(hs)) for mat,hs in hsets.items()}


def verify_output(data,nv,nf):
    infos=parse_file(data)
    o=parse_object(data,infos,TARGET)
    if (o["nv"],o["nf"],o["nt"])!=(nv,nf,nf):
        raise ValueError("output counts mismatch")
    base=o["texbase"]
    for fi,f in enumerate(o["faces"]):
        if any(v>=nv for v in f["verts"]):
            raise ValueError(f"face {fi}: vertex OOB")
        d=f["lp_tex"]-base
        if d<0 or d%SZ_TEX: raise ValueError(f"face {fi}: bad tex ptr")
        ti=d//SZ_TEX
        if not 0<=ti<o["nt"]: raise ValueError(f"face {fi}: tex index OOB")
        if o["texlinks"][ti]["NextTex"]!=0:
            raise ValueError(f"face {fi}: unexpected layer chain")
    if len(o["bones"])!=nv or any(not b for b in o["bones"]):
        raise ValueError("output Physique invalid")
    return infos,o


def parse_args(argv):
    p=argparse.ArgumentParser()
    p.add_argument("--smd",required=True)
    p.add_argument("--skeleton",required=True)
    p.add_argument("--object",default=TARGET)
    p.add_argument("--out",required=True)
    p.add_argument("--report")
    p.add_argument("--max-fit-error",type=float,default=0.02)
    return p.parse_args(argv)


def main(argv):
    a=parse_args(argv)
    if a.object!=TARGET:
        raise ValueError("HIGH exporter V1 supports only MshC01")

    smd=Path(a.smd).expanduser().resolve()
    skel=Path(a.skeleton).expanduser().resolve()
    outp=Path(a.out).expanduser().resolve()

    src=smd.read_bytes()
    if sha(src)!=SOURCE_SHA:
        raise ValueError("source is not approved pristine MfbC01.smd")

    infos=parse_file(src)
    so=parse_object(src,infos,TARGET)
    if (so["nv"],so["nf"],so["nt"])!=(SOURCE_NV,SOURCE_NF,SOURCE_NT):
        raise ValueError("unexpected pristine MshC01 counts")
    if not so["phys"] or so["nr"] or so["np"] or so["ns"]:
        raise ValueError("unexpected HIGH Physique/animation-track state")
    if i32(so["raw"],O_VERTEX_CAP)!=SOURCE_NV or i32(so["raw"],O_FACE_CAP)!=SOURCE_NF:
        raise ValueError("unexpected capacity mirrors")

    templates,htex_by_mat=material_templates(so)
    allowed_materials=set(templates)

    skeleton=parse_skeleton(skel.read_bytes())

    obj=bpy.data.objects.get(TARGET)
    if obj is None or obj.type!="MESH":
        raise ValueError("MshC01 Blender mesh missing")
    mesh=obj.data; mesh.update()

    if len(mesh.vertices)<SOURCE_NV:
        raise ValueError("original HIGH vertices deleted")
    if len(mesh.vertices)>65535:
        raise ValueError("vertex count exceeds uint16 face-index limit")
    if len(mesh.uv_layers)!=1 or mesh.uv_layers.active is None:
        raise ValueError("requires exactly one active UV layer")

    for p in mesh.polygons:
        if len(p.vertices)!=3:
            raise ValueError(f"polygon {p.index}: triangulate first")
        mat=int(p.material_index)
        if mat not in allowed_materials:
            raise ValueError(
                f"polygon {p.index}: material {mat} absent from pristine MshC01; "
                f"allowed={sorted(allowed_materials)}"
            )

    max_old=0.0
    for vi in range(SOURCE_NV):
        bone=so["bones"][vi]
        if rigid_bone(obj,vi)!=bone:
            raise ValueError(f"old vertex {vi}: Physique changed")
        tm=skeleton.get(bone)
        if tm is None: raise ValueError(f"missing skeleton bone {bone!r}")
        wf=world_fixed(so["vertices"][vi][:3],tm)
        expected=tuple(x/256. for x in wf)
        actual=tuple(float(x) for x in mesh.vertices[vi].co)
        err=math.sqrt(sum((actual[k]-expected[k])**2 for k in range(3)))
        max_old=max(max_old,err)
        if err>1e-6:
            raise ValueError(f"old vertex {vi} moved by {err:.9g}")

    new_bones=[]
    for vi in range(SOURCE_NV,len(mesh.vertices)):
        b=rigid_bone(obj,vi)
        if b not in skeleton:
            raise ValueError(f"new vertex {vi}: bone {b!r} missing from skeleton")
        new_bones.append(b)

    eqn=equal_normals(mesh)
    mags_by_bone=defaultdict(list); all_mags=[]
    for vi,sv in enumerate(so["vertices"]):
        nx,ny,nz=sv[3:6]
        mag=math.sqrt(nx*nx+ny*ny+nz*nz)
        mags_by_bone[so["bones"][vi]].append(mag); all_mags.append(mag)
    fallback=statistics.median(all_mags)

    vertices=bytearray(src[so["layout"]["vertices"]:so["layout"]["faces"]])
    fit_errors=[]; new_rows=[]
    for vi in range(SOURCE_NV,len(mesh.vertices)):
        bone=new_bones[vi-SOURCE_NV]; tm=skeleton[bone]
        world=tuple(float(x) for x in mesh.vertices[vi].co)
        local,represented,fit=fit_local(world,tm)
        if fit>a.max_fit_error:
            raise ValueError(f"new vertex {vi}: fit {fit} exceeds max")
        ldir=normalize(inv_row3(eqn[vi],tm))
        scale=statistics.median(mags_by_bone[bone]) if mags_by_bone[bone] else fallback
        normal=tuple(int(round(x*scale)) for x in ldir)
        vertices.extend(struct.pack("<6i",*local,*normal))
        fit_errors.append(fit)
        new_rows.append(dict(index=vi,bone=bone,world=list(world),
                             local_fixed=list(local),represented_world=list(represented),
                             fit_error=fit,normal_local=list(normal),normal_scale=scale))

    uv=mesh.uv_layers.active
    faces=bytearray(); tex=bytearray()
    for fi,p in enumerate(mesh.polygons):
        ids=tuple(int(v) for v in p.vertices)
        if any(v>65535 for v in ids): raise ValueError("uint16 index overflow")
        mat=int(p.material_index)
        ft,tt=templates[mat]
        ptr=so["texbase"]+fi*SZ_TEX
        if ptr>0xffffffff: raise ValueError("historical TexLink pointer overflow")
        faces.extend(face_blob(ft,ids,mat,ptr))
        tex.extend(tex_blob(tt,blender_uvs_smd(mesh,p,uv),htex_by_mat[mat]))

    phys=bytearray(src[so["layout"]["physique"]:so["layout"]["end"]])
    for b in new_bones: phys.extend(bone_slot(b))

    nv=len(mesh.vertices); nf=len(mesh.polygons); nt=nf
    raw=bytearray(so["raw"])
    for off,val in ((O_VERTEX_CAP,nv),(O_FACE_CAP,nf),(O_NV,nv),(O_NF,nf),(O_NT,nt)):
        struct.pack_into("<i",raw,off,val)

    block=bytes(raw)+bytes(vertices)+bytes(faces)+bytes(tex)+bytes(phys)
    expected=SZ_OBJ+nv*SZ_VERT+nf*SZ_FACE+nt*SZ_TEX+nv*32
    if len(block)!=expected: raise ValueError("new HIGH block size mismatch")

    prefix=bytearray(src[:infos[0]["start"]])
    blocks=[]; cursor=infos[0]["start"]; table=[]
    for inf in infos:
        b=block if inf["name"]==TARGET else src[inf["start"]:inf["start"]+inf["length"]]
        blocks.append(b)
        struct.pack_into("<i",prefix,inf["table_off"]+32,len(b))
        struct.pack_into("<i",prefix,inf["table_off"]+36,cursor)
        table.append(dict(name=inf["name"],old_start=inf["start"],new_start=cursor,
                          old_length=inf["length"],new_length=len(b)))
        cursor+=len(b)
    out=bytes(prefix)+b"".join(blocks)

    out_infos,out_obj=verify_output(out,nv,nf)

    for old,new in zip(infos,out_infos):
        if old["name"]==TARGET: continue
        ob=src[old["start"]:old["start"]+old["length"]]
        nb=out[new["start"]:new["start"]+new["length"]]
        if ob!=nb: raise ValueError(f"unrelated object changed: {old['name']}")

    outp.parent.mkdir(parents=True,exist_ok=True)
    outp.write_bytes(out)

    report=dict(
        tool="PT_C01_HIGH_TOPOLOGY_EXPORT_V1_APPEND_SAFE",
        blend=bpy.data.filepath,source_smd=str(smd),source_sha256=sha(src),
        output=str(outp),output_sha256=sha(out),
        source_counts=dict(vertices=SOURCE_NV,faces=SOURCE_NF,texlinks=SOURCE_NT),
        output_counts=dict(vertices=nv,faces=nf,texlinks=nt),
        new_vertices=nv-SOURCE_NV,face_delta=nf-SOURCE_NF,
        input_size=len(src),output_size=len(out),growth_bytes=len(out)-len(src),
        max_old_vertex_error=max_old,
        max_new_vertex_fit_error=max(fit_errors) if fit_errors else 0.0,
        allowed_materials=sorted(allowed_materials),
        hTexture_by_material={str(k):v for k,v in htex_by_mat.items()},
        new_vertex_rows=new_rows,object_table=table,
        semantic_check="PASS",high_append_safe_topology_export_check="PASS")
    rp=Path(a.report).expanduser().resolve() if a.report else outp.with_suffix(".json")
    rp.parent.mkdir(parents=True,exist_ok=True)
    rp.write_text(json.dumps(report,indent=2),encoding="utf-8")

    print("PT_C01_HIGH_TOPOLOGY_EXPORT_V1_APPEND_SAFE")
    print("object=MshC01")
    print(f"source_counts={SOURCE_NV}/{SOURCE_NF}/{SOURCE_NT}")
    print(f"output_counts={nv}/{nf}/{nt}")
    print(f"new_vertices={nv-SOURCE_NV}")
    print(f"face_delta={nf-SOURCE_NF:+d}")
    print(f"allowed_materials={sorted(allowed_materials)}")
    print(f"max_old_vertex_error={max_old:.12g}")
    print(f"max_new_vertex_fit_error={(max(fit_errors) if fit_errors else 0.0):.12g}")
    print(f"input_size={len(src)}")
    print(f"output_size={len(out)}")
    print(f"growth_bytes={len(out)-len(src):+d}")
    print(f"input_sha256={sha(src)}")
    print(f"output_sha256={sha(out)}")
    print(f"report={rp}")
    print("semantic_check=PASS")
    print("high_append_safe_topology_export_check=PASS")


if __name__=="__main__":
    try:
        argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"high_append_safe_topology_export_check=FAIL: {type(exc).__name__}: {exc}")
        raise

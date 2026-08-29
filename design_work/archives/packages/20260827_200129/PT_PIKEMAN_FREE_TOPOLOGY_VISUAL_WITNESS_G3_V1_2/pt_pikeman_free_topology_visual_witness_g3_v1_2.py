#!/usr/bin/env python3
import bpy, json, math, pathlib, sys

TARGET="TbmC03"
EXPECTED_V=74
EXPECTED_F=122
OUT_V=75
OUT_F=124
SPIKE_Y=4.0
SCHEMA="PT_PIKEMAN_FREE_TOPOLOGY_G3_VISUAL_WITNESS_V1_2"

def rigid_bone(obj, vid):
    rows=[]
    for g in obj.data.vertices[vid].groups:
        if g.weight > 1e-6:
            rows.append((obj.vertex_groups[g.group].name,float(g.weight)))
    if len(rows)==1 and rows[0][1] >= 0.999:
        return rows[0][0]
    return None

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=2:
        raise SystemExit("usage: -- <out.blend> <report.json>")
    out=pathlib.Path(argv[0]).expanduser().resolve()
    rep=pathlib.Path(argv[1]).expanduser().resolve()

    obj=bpy.data.objects.get(TARGET)
    if obj is None or obj.type!="MESH":
        raise RuntimeError(f"missing {TARGET}")
    me=obj.data
    if (len(me.vertices),len(me.polygons))!=(EXPECTED_V,EXPECTED_F):
        raise RuntimeError(f"unexpected counts {len(me.vertices)}/{len(me.polygons)}")
    if any(len(p.vertices)!=3 for p in me.polygons):
        raise RuntimeError("source must be triangulated")
    uv=me.uv_layers.active
    if uv is None or len(me.uv_layers)!=1:
        raise RuntimeError("expected one active UV layer")

    coords=[tuple(float(x) for x in v.co) for v in me.vertices]
    faces=[tuple(int(i) for i in p.vertices) for p in me.polygons]
    mats=[int(p.material_index) for p in me.polygons]
    uv_faces=[[tuple(float(x) for x in uv.data[li].uv) for li in p.loop_indices] for p in me.polygons]
    weights={v.index:[(int(g.group),float(g.weight)) for g in v.groups if g.weight>1e-8] for v in me.vertices}

    # Front witness: choose same-rigid-bone triangle with largest centroid Y.
    # Secondary preference keeps it close to X center and torso height ~30.
    cand=[]
    for p in me.polygons:
        ids=tuple(int(i) for i in p.vertices)
        bones=[rigid_bone(obj,i) for i in ids]
        if None in bones or len(set(bones))!=1:
            continue
        c=tuple(sum(coords[i][a] for i in ids)/3.0 for a in range(3))
        score=(c[1], -abs(c[0]), -abs(c[2]-30.0))
        cand.append((score,p.index,ids,bones[0],c))
    if not cand:
        raise RuntimeError("no same-rigid-bone triangle found")
    cand.sort(reverse=True)
    _,fi,ids,bone,center=cand[0]

    new_vid=len(coords)
    new_pos=(center[0], center[1]+SPIKE_Y, center[2])
    coords.append(new_pos)

    src_uv=uv_faces[fi]
    uvc=(sum(u[0] for u in src_uv)/3.0,sum(u[1] for u in src_uv)/3.0)
    a,b,c=ids
    ua,ub,uc=src_uv
    mi=mats[fi]

    of=[]; om=[]; ou=[]
    for i,(f,m,u) in enumerate(zip(faces,mats,uv_faces)):
        if i!=fi:
            of.append(f); om.append(m); ou.append(u)
        else:
            of += [(a,b,new_vid),(b,c,new_vid),(c,a,new_vid)]
            om += [mi,mi,mi]
            ou += [[ua,ub,uvc],[ub,uc,uvc],[uc,ua,uvc]]

    if (len(coords),len(of))!=(OUT_V,OUT_F):
        raise RuntimeError("output topology mismatch")

    old=me
    nm=bpy.data.meshes.new(old.name+"_G3_VISUAL_WITNESS")
    for mat in old.materials: nm.materials.append(mat)
    nm.from_pydata(coords,[],of); nm.update(calc_edges=True)
    for p,m in zip(nm.polygons,om): p.material_index=m
    nuv=nm.uv_layers.new(name=uv.name)
    for p,uvs in zip(nm.polygons,ou):
        for li,val in zip(p.loop_indices,uvs): nuv.data[li].uv=val

    obj.data=nm
    # Mesh replacement clears deform membership; restore it exactly.
    for vid,rows in weights.items():
        for gi,w in rows:
            obj.vertex_groups[gi].add([vid],w,'REPLACE')
    vg=obj.vertex_groups.get(bone)
    if vg is None: raise RuntimeError(f"bone group missing {bone!r}")
    vg.add([new_vid],1.0,'REPLACE')

    nr=[(obj.vertex_groups[g.group].name,float(g.weight)) for g in obj.data.vertices[new_vid].groups if g.weight>1e-6]
    if len(nr)!=1 or nr[0][0]!=bone or nr[0][1]<0.999:
        raise RuntimeError(f"new vertex not rigid {nr}")

    obj["pt_g3_schema"]=SCHEMA
    obj["pt_g3_operation"]="FRONT_Y_SPIKE"
    obj["pt_g3_source_face"]=int(fi)
    obj["pt_g3_new_vertex"]=int(new_vid)
    obj["pt_g3_bone"]=bone
    obj["pt_g3_spike_y"]=SPIKE_Y

    out.parent.mkdir(parents=True,exist_ok=True)
    rep.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))
    data={
      "tool":"PT_PIKEMAN_FREE_TOPOLOGY_VISUAL_WITNESS_G3_V1_2",
      "object":TARGET,
      "source_counts":{"vertices":74,"faces":122},
      "output_counts":{"vertices":75,"faces":124},
      "source_face":fi,
      "source_face_center":center,
      "new_vertex":new_vid,
      "new_vertex_position":new_pos,
      "new_vertex_bone":bone,
      "spike_axis":"+Y",
      "spike_distance":SPIKE_Y,
      "material_id":mi,
      "uv_interpolated":True,
      "new_vertex_rigid":True,
      "client_modified":False,
      "edit_check":"PASS"
    }
    rep.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    print("PT_PIKEMAN_FREE_TOPOLOGY_VISUAL_WITNESS_G3_V1_2")
    print(f"object={TARGET}")
    print("source_counts=74/122")
    print("output_counts=75/124")
    print(f"source_face={fi}")
    print(f"source_face_center={center}")
    print(f"new_vertex={new_vid}")
    print(f"new_vertex_position={new_pos}")
    print(f"new_vertex_bone={bone!r}")
    print("spike_axis=+Y")
    print(f"spike_distance={SPIKE_Y}")
    print(f"material_id={mi}")
    print("uv_interpolated=True")
    print("new_vertex_rigid=True")
    print(f"artist_blend={out}")
    print(f"report={rep}")
    print("client_modified=False")
    print("visual_witness_edit_check=PASS")

if __name__=="__main__":
    main()

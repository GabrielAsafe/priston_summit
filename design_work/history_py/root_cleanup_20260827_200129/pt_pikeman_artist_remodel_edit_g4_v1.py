#!/usr/bin/env python3
import bpy, json, math, pathlib, sys
from collections import defaultdict, deque

TARGET="TbmC03"
EXPECTED_V=74
EXPECTED_F=122
PATCH_FACE_COUNT=8
OUT_V=82
OUT_F=138
BULGE_MAX=0.90
SCHEMA="PT_PIKEMAN_ARTIST_REMODEL_G4_V1"

def v_memberships(obj, vid):
    return [(int(g.group),float(g.weight)) for g in obj.data.vertices[vid].groups if g.weight > 1e-8]

def rigid_bone_name(obj, vid):
    rows=[(obj.vertex_groups[g.group].name,float(g.weight))
          for g in obj.data.vertices[vid].groups if g.weight > 1e-6]
    if len(rows)==1 and rows[0][1] >= 0.999:
        return rows[0][0]
    return None

def normalize(v):
    n=math.sqrt(sum(x*x for x in v))
    if n < 1e-9: return (0.0,1.0,0.0)
    return tuple(x/n for x in v)

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=2:
        raise SystemExit("usage: -- <out.blend> <report.json>")
    out=pathlib.Path(argv[0]).expanduser().resolve()
    report=pathlib.Path(argv[1]).expanduser().resolve()

    obj=bpy.data.objects.get(TARGET)
    if obj is None or obj.type!="MESH":
        raise RuntimeError(f"missing mesh {TARGET}")
    me=obj.data
    if (len(me.vertices),len(me.polygons)) != (EXPECTED_V,EXPECTED_F):
        raise RuntimeError(f"{TARGET}: expected {EXPECTED_V}/{EXPECTED_F}, got {len(me.vertices)}/{len(me.polygons)}")
    if any(len(p.vertices)!=3 for p in me.polygons):
        raise RuntimeError("G4 requires triangulated source")
    uv=me.uv_layers.active
    if uv is None or len(me.uv_layers)!=1:
        raise RuntimeError("expected exactly one active UV layer")
    group_names=[vg.name for vg in obj.vertex_groups]
    if len(group_names)!=11 or len(set(group_names))!=11:
        raise RuntimeError(f"expected 11 unique source vertex groups, got {group_names}")
    active_group_index=int(obj.vertex_groups.active_index)

    coords=[tuple(float(x) for x in v.co) for v in me.vertices]
    faces=[tuple(int(i) for i in p.vertices) for p in me.polygons]
    mats=[int(p.material_index) for p in me.polygons]
    uv_faces=[[tuple(float(x) for x in uv.data[li].uv) for li in p.loop_indices] for p in me.polygons]
    weights={v.index:v_memberships(obj,v.index) for v in me.vertices}
    source_weight_map={vid:[(group_names[gi],w) for gi,w in rows] for vid,rows in weights.items()}

    mesh_center=tuple(sum(c[a] for c in coords)/len(coords) for a in range(3))

    # Eligible faces: all three vertices rigid to exactly the same bone.
    eligible={}
    for p in me.polygons:
        ids=tuple(int(i) for i in p.vertices)
        bones=[rigid_bone_name(obj,i) for i in ids]
        if None in bones or len(set(bones))!=1:
            continue
        center=tuple(sum(coords[i][a] for i in ids)/3.0 for a in range(3))
        eligible[p.index]={"ids":ids,"bone":bones[0],"center":center}

    # Adjacency by shared source edge, constrained to same rigid bone.
    edge_faces=defaultdict(list)
    for fi,row in eligible.items():
        a,b,c=row["ids"]
        for e in (tuple(sorted((a,b))),tuple(sorted((b,c))),tuple(sorted((c,a)))):
            edge_faces[e].append(fi)
    adj=defaultdict(set)
    for flist in edge_faces.values():
        for a in flist:
            for b in flist:
                if a!=b and eligible[a]["bone"]==eligible[b]["bone"]:
                    adj[a].add(b)

    # Connected components. Prefer Spine1 if it can supply 8 faces; otherwise
    # use the largest same-bone component.
    seen=set(); comps=[]
    for fi in eligible:
        if fi in seen: continue
        q=[fi]; seen.add(fi); comp=[]
        while q:
            x=q.pop(); comp.append(x)
            for y in adj[x]:
                if y not in seen:
                    seen.add(y); q.append(y)
        bones={eligible[x]["bone"] for x in comp}
        if len(bones)!=1:
            raise RuntimeError("component crossed bone boundary")
        comps.append((next(iter(bones)),comp))
    suitable=[x for x in comps if len(x[1])>=PATCH_FACE_COUNT]
    if not suitable:
        raise RuntimeError(f"no same-bone connected component has {PATCH_FACE_COUNT} faces")
    spine=[x for x in suitable if x[0]=="Bip01 Spine1"]
    bone,component = (max(spine,key=lambda x:len(x[1])) if spine else max(suitable,key=lambda x:len(x[1])))

    # Seed near the component center, then BFS exactly 8 connected faces.
    cc=tuple(sum(eligible[fi]["center"][a] for fi in component)/len(component) for a in range(3))
    seed=min(component,key=lambda fi:sum((eligible[fi]["center"][a]-cc[a])**2 for a in range(3)))
    selected=[]
    q=deque([seed]); picked={seed}
    while q and len(selected)<PATCH_FACE_COUNT:
        x=q.popleft(); selected.append(x)
        neigh=sorted(adj[x], key=lambda fi:sum((eligible[fi]["center"][a]-cc[a])**2 for a in range(3)))
        for y in neigh:
            if y in component and y not in picked:
                picked.add(y); q.append(y)
    if len(selected)!=PATCH_FACE_COUNT:
        raise RuntimeError(f"could select only {len(selected)} connected patch faces")

    selected_set=set(selected)
    patch_center=tuple(sum(eligible[fi]["center"][a] for fi in selected)/len(selected) for a in range(3))
    distances=[
        math.sqrt(sum((eligible[fi]["center"][a]-patch_center[a])**2 for a in range(3)))
        for fi in selected
    ]
    max_dist=max(max(distances),1e-9)

    # Refine each selected triangle with one center vertex. Center vertices form
    # a gentle outward dome: stronger near patch center, softer at perimeter.
    new_specs={}
    out_coords=list(coords)
    for fi,d in zip(selected,distances):
        fc=eligible[fi]["center"]
        radial=normalize(tuple(fc[a]-mesh_center[a] for a in range(3)))
        falloff=1.0 - 0.45*(d/max_dist)
        offset=BULGE_MAX*falloff
        pos=tuple(fc[a]+radial[a]*offset for a in range(3))
        new_vid=len(out_coords); out_coords.append(pos)
        new_specs[fi]={"vid":new_vid,"position":pos,"offset":offset,"radial":radial}

    out_faces=[]; out_mats=[]; out_uv=[]
    for fi,(face,mat,uvs) in enumerate(zip(faces,mats,uv_faces)):
        if fi not in selected_set:
            out_faces.append(face); out_mats.append(mat); out_uv.append(uvs)
            continue
        a,b,c=face
        ua,ub,uc=uvs
        uvc=(sum(u[0] for u in uvs)/3.0,sum(u[1] for u in uvs)/3.0)
        nv=new_specs[fi]["vid"]
        out_faces += [(a,b,nv),(b,c,nv),(c,a,nv)]
        out_mats += [mat,mat,mat]
        out_uv += [[ua,ub,uvc],[ub,uc,uvc],[uc,ua,uvc]]

    if (len(out_coords),len(out_faces)) != (OUT_V,OUT_F):
        raise RuntimeError(f"unexpected output counts {len(out_coords)}/{len(out_faces)}")

    old_mesh=me
    new_mesh=bpy.data.meshes.new(old_mesh.name+"_G4_ARTIST_REMODEL")
    for mat in old_mesh.materials: new_mesh.materials.append(mat)
    new_mesh.from_pydata(out_coords,[],out_faces)
    new_mesh.update(calc_edges=True)
    for p,mi in zip(new_mesh.polygons,out_mats): p.material_index=mi
    new_uv=new_mesh.uv_layers.new(name=uv.name)
    for p,uvs in zip(new_mesh.polygons,out_uv):
        if len(p.loop_indices)!=len(uvs): raise RuntimeError("UV loop count mismatch")
        for li,val in zip(p.loop_indices,uvs): new_uv.data[li].uv=val

    obj.data=new_mesh

    # Blender may clear deform-group schema after mesh replacement. Recreate it
    # explicitly, then restore all original memberships by original index.
    while len(obj.vertex_groups):
        obj.vertex_groups.remove(obj.vertex_groups[-1])
    for name in group_names: obj.vertex_groups.new(name=name)
    if [vg.name for vg in obj.vertex_groups] != group_names:
        raise RuntimeError("vertex-group schema recreation mismatch")

    restored=0
    for vid,rows in weights.items():
        for gi,w in rows:
            obj.vertex_groups[gi].add([vid],w,'REPLACE'); restored+=1

    vg=obj.vertex_groups.get(bone)
    if vg is None: raise RuntimeError(f"patch bone group missing: {bone!r}")
    for fi in selected:
        vg.add([new_specs[fi]["vid"]],1.0,'REPLACE')

    if 0 <= active_group_index < len(obj.vertex_groups):
        obj.vertex_groups.active_index=active_group_index

    # Strict old-weight identity.
    max_weight_error=0.0
    for vid in range(EXPECTED_V):
        after=[(obj.vertex_groups[g.group].name,float(g.weight))
               for g in obj.data.vertices[vid].groups if g.weight>1e-8]
        before=source_weight_map[vid]
        if [x[0] for x in after] != [x[0] for x in before]:
            raise RuntimeError(f"old vertex {vid}: membership changed")
        for (_,bw),(_,aw) in zip(before,after):
            max_weight_error=max(max_weight_error,abs(bw-aw))
    if max_weight_error>1e-8:
        raise RuntimeError(f"old vertex weight drift {max_weight_error}")

    for fi in selected:
        vid=new_specs[fi]["vid"]
        rows=[(obj.vertex_groups[g.group].name,float(g.weight))
              for g in obj.data.vertices[vid].groups if g.weight>1e-6]
        if len(rows)!=1 or rows[0][0]!=bone or rows[0][1]<0.999:
            raise RuntimeError(f"new vertex {vid} not rigid: {rows}")

    # Topology diagnostics: refinement must not increase non-manifold edges.
    edge_degree=defaultdict(int)
    for f in out_faces:
        a,b,c=f
        for e in (tuple(sorted((a,b))),tuple(sorted((b,c))),tuple(sorted((c,a)))):
            edge_degree[e]+=1
    nonmanifold=sum(1 for d in edge_degree.values() if d>2)
    boundary=sum(1 for d in edge_degree.values() if d==1)
    if nonmanifold != 1:
        raise RuntimeError(f"expected inherited nonmanifold_edge_count=1, got {nonmanifold}")

    obj["pt_g4_schema"]=SCHEMA
    obj["pt_g4_operation"]="CONNECTED_8_FACE_REFINEMENT_WITH_OUTWARD_DOME"
    obj["pt_g4_patch_bone"]=bone
    obj["pt_g4_patch_faces"]=json.dumps(selected)
    obj["pt_g4_bulge_max"]=BULGE_MAX

    out.parent.mkdir(parents=True,exist_ok=True)
    report.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    data={
      "tool":"PT_PIKEMAN_ARTIST_REMODEL_GATE_G4_V1",
      "profile_id":"pikeman_c01_m4",
      "object":TARGET,
      "source_counts":{"vertices":EXPECTED_V,"faces":EXPECTED_F},
      "output_counts":{"vertices":OUT_V,"faces":OUT_F},
      "vertex_delta":OUT_V-EXPECTED_V,
      "face_delta":OUT_F-EXPECTED_F,
      "patch_face_count":PATCH_FACE_COUNT,
      "patch_bone":bone,
      "selected_source_faces":selected,
      "patch_center":patch_center,
      "bulge_max":BULGE_MAX,
      "new_vertices":[new_specs[fi] | {"source_face":fi} for fi in selected],
      "source_vertex_group_count":len(group_names),
      "output_vertex_group_count":len(obj.vertex_groups),
      "restored_weight_memberships":restored,
      "old_vertex_weight_max_error":max_weight_error,
      "new_vertices_rigid":True,
      "uv_centers_interpolated":True,
      "nonmanifold_edge_count":nonmanifold,
      "boundary_edge_count":boundary,
      "client_modified":False,
      "edit_check":"PASS"
    }
    report.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_ARTIST_REMODEL_EDIT_G4_V1")
    print(f"object={TARGET}")
    print(f"source_counts={EXPECTED_V}/{EXPECTED_F}")
    print(f"output_counts={OUT_V}/{OUT_F}")
    print(f"vertex_delta=+{OUT_V-EXPECTED_V}")
    print(f"face_delta=+{OUT_F-EXPECTED_F}")
    print(f"patch_face_count={PATCH_FACE_COUNT}")
    print(f"patch_bone={bone!r}")
    print(f"selected_source_faces={selected}")
    print(f"patch_center={patch_center}")
    print(f"bulge_max={BULGE_MAX}")
    print(f"source_vertex_group_count={len(group_names)}")
    print(f"output_vertex_group_count={len(obj.vertex_groups)}")
    print(f"restored_weight_memberships={restored}")
    print(f"old_vertex_weight_max_error={max_weight_error:.12g}")
    print("new_vertices_rigid=True")
    print("uv_centers_interpolated=True")
    print(f"nonmanifold_edge_count={nonmanifold}")
    print(f"boundary_edge_count={boundary}")
    print(f"artist_blend={out}")
    print(f"report={report}")
    print("client_modified=False")
    print("artist_remodel_edit_check=PASS")

if __name__=="__main__":
    main()

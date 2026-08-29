#!/usr/bin/env python3
import bpy, json, math, pathlib, sys
from collections import defaultdict, deque

TARGET="TbmC03"
EXPECTED_V=74
EXPECTED_F=122
PATCH_FACE_COUNT=8
OUT_V=82
OUT_F=138

# Strong visual witness:
SOURCE_VERTEX_PUSH=2.50
CENTER_EXTRA_PUSH=1.50

SCHEMA="PT_PIKEMAN_ARTIST_REMODEL_VISUAL_G4_V1_1"

def normalize(v):
    n=math.sqrt(sum(x*x for x in v))
    if n < 1e-9:
        raise RuntimeError("cannot normalize zero vector")
    return tuple(x/n for x in v)

def memberships(obj, vid):
    return [(int(g.group),float(g.weight))
            for g in obj.data.vertices[vid].groups if g.weight > 1e-8]

def rigid_bone(obj, vid):
    rows=[(obj.vertex_groups[g.group].name,float(g.weight))
          for g in obj.data.vertices[vid].groups if g.weight > 1e-6]
    if len(rows)==1 and rows[0][1] >= 0.999:
        return rows[0][0]
    return None

def bbox(coords):
    return {
      "min":[min(c[a] for c in coords) for a in range(3)],
      "max":[max(c[a] for c in coords) for a in range(3)],
    }

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
        raise RuntimeError(f"unexpected source counts {len(me.vertices)}/{len(me.polygons)}")
    if any(len(p.vertices)!=3 for p in me.polygons):
        raise RuntimeError("source must be triangulated")
    uv=me.uv_layers.active
    if uv is None or len(me.uv_layers)!=1:
        raise RuntimeError("expected exactly one active UV layer")

    group_names=[vg.name for vg in obj.vertex_groups]
    if len(group_names)!=11 or len(set(group_names))!=11:
        raise RuntimeError(f"expected 11 unique vertex groups, got {group_names}")
    active_group_index=int(obj.vertex_groups.active_index)

    coords=[tuple(float(x) for x in v.co) for v in me.vertices]
    faces=[tuple(int(i) for i in p.vertices) for p in me.polygons]
    mats=[int(p.material_index) for p in me.polygons]
    uv_faces=[[tuple(float(x) for x in uv.data[li].uv) for li in p.loop_indices]
              for p in me.polygons]
    weights={v.index:memberships(obj,v.index) for v in me.vertices}
    source_weight_map={vid:[(group_names[gi],w) for gi,w in rows]
                       for vid,rows in weights.items()}

    mesh_center=tuple(sum(c[a] for c in coords)/len(coords) for a in range(3))
    source_bbox=bbox(coords)

    eligible={}
    for p in me.polygons:
        ids=tuple(int(i) for i in p.vertices)
        bones=[rigid_bone(obj,i) for i in ids]
        if None in bones or len(set(bones))!=1:
            continue
        center=tuple(sum(coords[i][a] for i in ids)/3.0 for a in range(3))
        radial_len=math.sqrt(sum((center[a]-mesh_center[a])**2 for a in range(3)))
        eligible[p.index]={"ids":ids,"bone":bones[0],"center":center,"radial_len":radial_len}

    edge_faces=defaultdict(list)
    for fi,row in eligible.items():
        a,b,c=row["ids"]
        for e in (tuple(sorted((a,b))),tuple(sorted((b,c))),tuple(sorted((c,a)))):
            edge_faces[e].append(fi)

    adj=defaultdict(set)
    for rows in edge_faces.values():
        for a in rows:
            for b in rows:
                if a!=b and eligible[a]["bone"]==eligible[b]["bone"]:
                    adj[a].add(b)

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
            raise RuntimeError("same-bone component invariant failed")
        comps.append((next(iter(bones)),comp))

    suitable=[x for x in comps if len(x[1])>=PATCH_FACE_COUNT]
    if not suitable:
        raise RuntimeError("no safe connected patch")
    spine=[x for x in suitable if x[0]=="Bip01 Spine1"]
    bone,component=(max(spine,key=lambda x:len(x[1]))
                    if spine else max(suitable,key=lambda x:len(x[1])))

    # Seed at the most radially exposed face of the safe component.
    seed=max(component,key=lambda fi:eligible[fi]["radial_len"])
    selected=[]
    q=deque([seed]); queued={seed}
    while q and len(selected)<PATCH_FACE_COUNT:
        x=q.popleft(); selected.append(x)
        neighbors=sorted(
            [y for y in adj[x] if y in component and y not in queued],
            key=lambda fi:eligible[fi]["radial_len"],
            reverse=True
        )
        for y in neighbors:
            queued.add(y); q.append(y)
    if len(selected)!=PATCH_FACE_COUNT:
        raise RuntimeError(f"selected only {len(selected)} patch faces")

    selected_set=set(selected)
    patch_vertices=sorted({vid for fi in selected for vid in eligible[fi]["ids"]})
    if len(patch_vertices)<6:
        raise RuntimeError("patch is unexpectedly small")

    # Every moved source vertex is guaranteed rigid to the selected bone.
    for vid in patch_vertices:
        if rigid_bone(obj,vid)!=bone:
            raise RuntimeError(f"patch vertex {vid} is not rigid to {bone!r}")

    # Strong silhouette change: move all source vertices participating in the
    # patch radially outward 2.5 units. Shared neighboring faces deform with
    # them naturally, keeping connectivity.
    out_coords=list(coords)
    source_moves={}
    for vid in patch_vertices:
        direction=normalize(tuple(coords[vid][a]-mesh_center[a] for a in range(3)))
        newpos=tuple(coords[vid][a]+direction[a]*SOURCE_VERTEX_PUSH for a in range(3))
        out_coords[vid]=newpos
        source_moves[vid]={"old":coords[vid],"new":newpos,"direction":direction}

    # Add one center vertex per selected source triangle. Its base center is
    # computed from already-moved corners, plus another 1.5 outward.
    new_specs={}
    for fi in selected:
        ids=eligible[fi]["ids"]
        moved_center=tuple(sum(out_coords[i][a] for i in ids)/3.0 for a in range(3))
        direction=normalize(tuple(moved_center[a]-mesh_center[a] for a in range(3)))
        newpos=tuple(moved_center[a]+direction[a]*CENTER_EXTRA_PUSH for a in range(3))
        newvid=len(out_coords); out_coords.append(newpos)
        new_specs[fi]={
          "vid":newvid,
          "moved_corner_center":moved_center,
          "new":newpos,
          "direction":direction,
        }

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

    if (len(out_coords),len(out_faces))!=(OUT_V,OUT_F):
        raise RuntimeError(f"unexpected output counts {len(out_coords)}/{len(out_faces)}")

    old_mesh=me
    new_mesh=bpy.data.meshes.new(old_mesh.name+"_G4_VISUAL_V1_1")
    for mat in old_mesh.materials: new_mesh.materials.append(mat)
    new_mesh.from_pydata(out_coords,[],out_faces)
    new_mesh.update(calc_edges=True)
    for p,mi in zip(new_mesh.polygons,out_mats): p.material_index=mi

    new_uv=new_mesh.uv_layers.new(name=uv.name)
    for p,uvs in zip(new_mesh.polygons,out_uv):
        if len(p.loop_indices)!=len(uvs): raise RuntimeError("UV loop mismatch")
        for li,val in zip(p.loop_indices,uvs):
            new_uv.data[li].uv=val

    obj.data=new_mesh

    while len(obj.vertex_groups):
        obj.vertex_groups.remove(obj.vertex_groups[-1])
    for name in group_names:
        obj.vertex_groups.new(name=name)
    if [vg.name for vg in obj.vertex_groups]!=group_names:
        raise RuntimeError("vertex-group schema recreation mismatch")

    restored=0
    for vid,rows in weights.items():
        for gi,w in rows:
            obj.vertex_groups[gi].add([vid],w,'REPLACE'); restored+=1
    vg=obj.vertex_groups.get(bone)
    if vg is None: raise RuntimeError("patch bone group missing after recreation")
    for fi in selected:
        vg.add([new_specs[fi]["vid"]],1.0,'REPLACE')
    if 0 <= active_group_index < len(obj.vertex_groups):
        obj.vertex_groups.active_index=active_group_index

    max_weight_error=0.0
    for vid in range(EXPECTED_V):
        after=[(obj.vertex_groups[g.group].name,float(g.weight))
               for g in obj.data.vertices[vid].groups if g.weight>1e-8]
        before=source_weight_map[vid]
        if [x[0] for x in after] != [x[0] for x in before]:
            raise RuntimeError(f"membership changed at old vertex {vid}")
        for (_,bw),(_,aw) in zip(before,after):
            max_weight_error=max(max_weight_error,abs(bw-aw))
    if max_weight_error>1e-8:
        raise RuntimeError(f"old weight drift={max_weight_error}")

    for fi in selected:
        vid=new_specs[fi]["vid"]
        rows=[(obj.vertex_groups[g.group].name,float(g.weight))
              for g in obj.data.vertices[vid].groups if g.weight>1e-6]
        if len(rows)!=1 or rows[0][0]!=bone or rows[0][1]<0.999:
            raise RuntimeError(f"new vertex {vid} not rigid: {rows}")

    edge_degree=defaultdict(int)
    for f in out_faces:
        a,b,c=f
        for e in (tuple(sorted((a,b))),tuple(sorted((b,c))),tuple(sorted((c,a)))):
            edge_degree[e]+=1
    nonmanifold=sum(1 for d in edge_degree.values() if d>2)
    boundary=sum(1 for d in edge_degree.values() if d==1)
    if nonmanifold!=1:
        raise RuntimeError(f"expected inherited nonmanifold=1, got {nonmanifold}")

    output_bbox=bbox(out_coords)
    max_old_move=max(
      math.sqrt(sum((source_moves[vid]["new"][a]-source_moves[vid]["old"][a])**2 for a in range(3)))
      for vid in patch_vertices
    )
    if abs(max_old_move-SOURCE_VERTEX_PUSH)>1e-6:
        raise RuntimeError("source vertex displacement mismatch")

    obj["pt_g4_visual_schema"]=SCHEMA
    obj["pt_g4_visual_patch_bone"]=bone
    obj["pt_g4_visual_selected_faces"]=json.dumps(selected)
    obj["pt_g4_visual_source_vertex_push"]=SOURCE_VERTEX_PUSH
    obj["pt_g4_visual_center_extra_push"]=CENTER_EXTRA_PUSH

    out.parent.mkdir(parents=True,exist_ok=True)
    report.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    data={
      "tool":"PT_PIKEMAN_ARTIST_REMODEL_VISUAL_G4_V1_1",
      "profile_id":"pikeman_c01_m4",
      "object":TARGET,
      "source_counts":{"vertices":74,"faces":122},
      "output_counts":{"vertices":82,"faces":138},
      "vertex_delta":8,
      "face_delta":16,
      "patch_face_count":8,
      "patch_bone":bone,
      "selected_source_faces":selected,
      "moved_source_vertices":patch_vertices,
      "moved_source_vertex_count":len(patch_vertices),
      "source_vertex_push":SOURCE_VERTEX_PUSH,
      "center_extra_push":CENTER_EXTRA_PUSH,
      "max_old_vertex_move":max_old_move,
      "source_bbox":source_bbox,
      "output_bbox":output_bbox,
      "source_vertex_group_count":len(group_names),
      "output_vertex_group_count":len(obj.vertex_groups),
      "restored_weight_memberships":restored,
      "old_vertex_weight_max_error":max_weight_error,
      "new_vertices_rigid":True,
      "uv_centers_interpolated":True,
      "nonmanifold_edge_count":nonmanifold,
      "boundary_edge_count":boundary,
      "client_modified":False,
      "edit_check":"PASS",
    }
    report.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_ARTIST_REMODEL_VISUAL_EDIT_G4_V1_1")
    print(f"object={TARGET}")
    print("source_counts=74/122")
    print("output_counts=82/138")
    print("vertex_delta=+8")
    print("face_delta=+16")
    print("patch_face_count=8")
    print(f"patch_bone={bone!r}")
    print(f"selected_source_faces={selected}")
    print(f"moved_source_vertices={patch_vertices}")
    print(f"moved_source_vertex_count={len(patch_vertices)}")
    print(f"source_vertex_push={SOURCE_VERTEX_PUSH}")
    print(f"center_extra_push={CENTER_EXTRA_PUSH}")
    print(f"max_old_vertex_move={max_old_move:.12g}")
    print(f"source_bbox={source_bbox}")
    print(f"output_bbox={output_bbox}")
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
    print("g4_visual_edit_check=PASS")

if __name__=="__main__":
    main()

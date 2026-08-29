#!/usr/bin/env python3
import bpy, json, pathlib, sys

TARGET="TbmC03"
EXPECTED_V=74
EXPECTED_F=122
OUT_V=75
OUT_F=124
SPIKE_Y=4.0
SCHEMA="PT_PIKEMAN_FREE_TOPOLOGY_G3_VISUAL_WITNESS_V1_3"

def memberships(obj, vid):
    return [
        (int(g.group), float(g.weight))
        for g in obj.data.vertices[vid].groups
        if g.weight > 1e-8
    ]

def rigid_bone(obj, vid):
    rows = [
        (obj.vertex_groups[g.group].name, float(g.weight))
        for g in obj.data.vertices[vid].groups
        if g.weight > 1e-6
    ]
    if len(rows) == 1 and rows[0][1] >= 0.999:
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
        raise RuntimeError("expected exactly one active UV layer")

    # Preserve vertex-group schema BEFORE replacing the Mesh datablock.
    group_names=[vg.name for vg in obj.vertex_groups]
    if len(group_names) != 11:
        raise RuntimeError(f"expected 11 source vertex groups, got {len(group_names)}")
    if len(set(group_names)) != len(group_names):
        raise RuntimeError("duplicate vertex-group names")
    active_group_index = int(obj.vertex_groups.active_index)

    coords=[tuple(float(x) for x in v.co) for v in me.vertices]
    faces=[tuple(int(i) for i in p.vertices) for p in me.polygons]
    mats=[int(p.material_index) for p in me.polygons]
    uv_faces=[
        [tuple(float(x) for x in uv.data[li].uv) for li in p.loop_indices]
        for p in me.polygons
    ]
    weights={v.index:memberships(obj,v.index) for v in me.vertices}

    # Record exact source weight data for a post-rebuild comparison.
    source_weight_map={
        vid: [(group_names[gi],w) for gi,w in rows]
        for vid,rows in weights.items()
    }

    # Choose a visibly frontal triangle: maximum centroid +Y, while requiring
    # all three vertices to be rigidly bound to the same bone.
    candidates=[]
    for p in me.polygons:
        ids=tuple(int(i) for i in p.vertices)
        bones=[rigid_bone(obj,i) for i in ids]
        if None in bones or len(set(bones))!=1:
            continue
        center=tuple(sum(coords[i][a] for i in ids)/3.0 for a in range(3))
        score=(center[1], -abs(center[0]), -abs(center[2]-30.0))
        candidates.append((score,p.index,ids,bones[0],center))

    if not candidates:
        raise RuntimeError("no same-rigid-bone triangle found")
    candidates.sort(reverse=True)
    _,face_idx,ids,bone_name,center=candidates[0]

    new_vid=len(coords)
    new_pos=(center[0], center[1]+SPIKE_Y, center[2])
    coords.append(new_pos)

    src_uv=uv_faces[face_idx]
    uv_center=(
        sum(u[0] for u in src_uv)/3.0,
        sum(u[1] for u in src_uv)/3.0,
    )
    a,b,c=ids
    ua,ub,uc=src_uv
    material_id=mats[face_idx]

    out_faces=[]; out_mats=[]; out_uv=[]
    for i,(face,mat,uvs) in enumerate(zip(faces,mats,uv_faces)):
        if i != face_idx:
            out_faces.append(face)
            out_mats.append(mat)
            out_uv.append(uvs)
        else:
            out_faces.extend([
                (a,b,new_vid),
                (b,c,new_vid),
                (c,a,new_vid),
            ])
            out_mats.extend([material_id]*3)
            out_uv.extend([
                [ua,ub,uv_center],
                [ub,uc,uv_center],
                [uc,ua,uv_center],
            ])

    if (len(coords),len(out_faces)) != (OUT_V,OUT_F):
        raise RuntimeError("output topology mismatch")

    old_mesh=me
    new_mesh=bpy.data.meshes.new(old_mesh.name+"_G3_VISUAL_WITNESS_V1_3")
    for mat in old_mesh.materials:
        new_mesh.materials.append(mat)
    new_mesh.from_pydata(coords,[],out_faces)
    new_mesh.update(calc_edges=True)

    for p,mi in zip(new_mesh.polygons,out_mats):
        p.material_index=mi

    new_uv=new_mesh.uv_layers.new(name=uv.name)
    for p,uvs in zip(new_mesh.polygons,out_uv):
        if len(p.loop_indices) != len(uvs):
            raise RuntimeError("UV loop count mismatch")
        for li,value in zip(p.loop_indices,uvs):
            new_uv.data[li].uv=value

    # Replace mesh, then explicitly recreate object-level deform-group schema.
    obj.data=new_mesh

    while len(obj.vertex_groups):
        obj.vertex_groups.remove(obj.vertex_groups[-1])
    for name in group_names:
        obj.vertex_groups.new(name=name)

    if [vg.name for vg in obj.vertex_groups] != group_names:
        raise RuntimeError("vertex-group schema recreation mismatch")

    # Restore every old membership using the exact original group index/order.
    restored_memberships=0
    for vid,rows in weights.items():
        for gi,w in rows:
            obj.vertex_groups[gi].add([vid],w,'REPLACE')
            restored_memberships += 1

    vg=obj.vertex_groups.get(bone_name)
    if vg is None:
        raise RuntimeError(f"selected rigid bone group missing after recreation: {bone_name!r}")
    vg.add([new_vid],1.0,'REPLACE')

    if 0 <= active_group_index < len(obj.vertex_groups):
        obj.vertex_groups.active_index=active_group_index

    # Exact old-weight postcondition by group name.
    max_weight_error=0.0
    for vid in range(EXPECTED_V):
        after=[
            (obj.vertex_groups[g.group].name,float(g.weight))
            for g in obj.data.vertices[vid].groups
            if g.weight > 1e-8
        ]
        before=source_weight_map[vid]
        if [x[0] for x in after] != [x[0] for x in before]:
            raise RuntimeError(f"vertex {vid}: group membership changed {before} -> {after}")
        for (_,bw),(_,aw) in zip(before,after):
            max_weight_error=max(max_weight_error,abs(bw-aw))

    if max_weight_error > 1e-8:
        raise RuntimeError(f"old vertex weight drift={max_weight_error}")

    new_rows=[
        (obj.vertex_groups[g.group].name,float(g.weight))
        for g in obj.data.vertices[new_vid].groups
        if g.weight > 1e-6
    ]
    if len(new_rows)!=1 or new_rows[0][0]!=bone_name or new_rows[0][1]<0.999:
        raise RuntimeError(f"new vertex not rigid: {new_rows}")

    if len(obj.data.uv_layers)!=1:
        raise RuntimeError("UV layer schema changed")
    if len(obj.data.vertices)!=OUT_V or len(obj.data.polygons)!=OUT_F:
        raise RuntimeError("final topology mismatch")

    obj["pt_g3_schema"]=SCHEMA
    obj["pt_g3_operation"]="FRONT_Y_SPIKE"
    obj["pt_g3_source_face"]=int(face_idx)
    obj["pt_g3_new_vertex"]=int(new_vid)
    obj["pt_g3_bone"]=bone_name
    obj["pt_g3_spike_y"]=SPIKE_Y

    out.parent.mkdir(parents=True,exist_ok=True)
    rep.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    data={
        "tool":"PT_PIKEMAN_FREE_TOPOLOGY_VISUAL_WITNESS_G3_V1_3",
        "profile_id":"pikeman_c01_m4",
        "object":TARGET,
        "source_counts":{"vertices":74,"faces":122},
        "output_counts":{"vertices":75,"faces":124},
        "source_face":face_idx,
        "source_face_center":center,
        "new_vertex":new_vid,
        "new_vertex_position":new_pos,
        "new_vertex_bone":bone_name,
        "spike_axis":"+Y",
        "spike_distance":SPIKE_Y,
        "material_id":material_id,
        "source_vertex_group_count":len(group_names),
        "output_vertex_group_count":len(obj.vertex_groups),
        "restored_weight_memberships":restored_memberships,
        "old_vertex_weight_max_error":max_weight_error,
        "uv_interpolated":True,
        "vertex_group_schema_recreated":True,
        "old_vertex_weights_preserved":True,
        "new_vertex_rigid":True,
        "client_modified":False,
        "edit_check":"PASS",
    }
    rep.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_FREE_TOPOLOGY_VISUAL_WITNESS_G3_V1_3")
    print(f"object={TARGET}")
    print("source_counts=74/122")
    print("output_counts=75/124")
    print("vertex_delta=+1")
    print("face_delta=+2")
    print(f"source_face={face_idx}")
    print(f"source_face_center={center}")
    print(f"new_vertex={new_vid}")
    print(f"new_vertex_position={new_pos}")
    print(f"new_vertex_bone={bone_name!r}")
    print("spike_axis=+Y")
    print(f"spike_distance={SPIKE_Y}")
    print(f"material_id={material_id}")
    print(f"source_vertex_group_count={len(group_names)}")
    print(f"output_vertex_group_count={len(obj.vertex_groups)}")
    print(f"restored_weight_memberships={restored_memberships}")
    print(f"old_vertex_weight_max_error={max_weight_error:.12g}")
    print("uv_interpolated=True")
    print("vertex_group_schema_recreated=True")
    print("old_vertex_weights_preserved=True")
    print("new_vertex_rigid=True")
    print(f"artist_blend={out}")
    print(f"report={rep}")
    print("client_modified=False")
    print("visual_witness_edit_check=PASS")

if __name__=="__main__":
    main()

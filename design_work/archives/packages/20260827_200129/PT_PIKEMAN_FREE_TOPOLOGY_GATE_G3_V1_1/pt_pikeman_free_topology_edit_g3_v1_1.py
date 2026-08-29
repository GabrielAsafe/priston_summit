#!/usr/bin/env python3
import bpy
import json
import math
import pathlib
import sys

TARGET = "TbmC03"
EXPECTED_V = 74
EXPECTED_F = 122
EXPECTED_OUT_V = 75
EXPECTED_OUT_F = 124
SPIKE = 0.80
SCHEMA = "PT_PIKEMAN_FREE_TOPOLOGY_G3_V1_1"

def memberships_by_name(obj, vid):
    v = obj.data.vertices[vid]
    rows = []
    for g in v.groups:
        if g.weight > 1e-8:
            rows.append((obj.vertex_groups[g.group].name, float(g.weight)))
    return rows

def exact_rigid_bone(obj, vid):
    rows = [(n,w) for n,w in memberships_by_name(obj,vid) if w > 1e-6]
    if len(rows) != 1 or rows[0][1] < 0.999:
        return None
    return rows[0][0]

def main():
    argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv) != 2:
        raise SystemExit("usage: -- <output.blend> <report.json>")
    out = pathlib.Path(argv[0]).expanduser().resolve()
    report = pathlib.Path(argv[1]).expanduser().resolve()

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError(f"missing mesh {TARGET}")
    me = obj.data
    if len(me.vertices) != EXPECTED_V or len(me.polygons) != EXPECTED_F:
        raise RuntimeError(f"{TARGET}: expected {EXPECTED_V}/{EXPECTED_F}, got {len(me.vertices)}/{len(me.polygons)}")
    if any(len(p.vertices) != 3 for p in me.polygons):
        raise RuntimeError("G3 V1.1 requires source triangles")
    if len(me.uv_layers) != 1:
        raise RuntimeError(f"expected exactly one UV layer, got {len(me.uv_layers)}")
    uv_src = me.uv_layers.active or me.uv_layers[0]
    if uv_src is None:
        raise RuntimeError("active UV layer missing")
    if len(obj.vertex_groups) == 0:
        raise RuntimeError("source object has no vertex groups")

    # Save object-level group schema AND per-vertex weights by NAME.
    # This is deliberate: replacing obj.data can clear/de-sync Blender's
    # vertex-group collection, so group indices are not stable across the swap.
    group_names = [vg.name for vg in obj.vertex_groups]
    if len(group_names) != len(set(group_names)):
        raise RuntimeError("duplicate vertex group names")
    weights_by_vertex = {
        v.index: memberships_by_name(obj, v.index)
        for v in me.vertices
    }

    coords = [tuple(float(x) for x in v.co) for v in me.vertices]
    faces = [tuple(int(i) for i in p.vertices) for p in me.polygons]
    mats = [int(p.material_index) for p in me.polygons]
    uv_faces = [
        [tuple(float(x) for x in uv_src.data[li].uv) for li in p.loop_indices]
        for p in me.polygons
    ]

    mesh_center = (
        sum(p[0] for p in coords) / len(coords),
        sum(p[1] for p in coords) / len(coords),
        sum(p[2] for p in coords) / len(coords),
    )

    candidates = []
    for p in me.polygons:
        ids = tuple(int(i) for i in p.vertices)
        bones = [exact_rigid_bone(obj, i) for i in ids]
        if None in bones or len(set(bones)) != 1:
            continue
        center = tuple(sum(coords[i][a] for i in ids) / 3.0 for a in range(3))
        radial = tuple(center[i] - mesh_center[i] for i in range(3))
        n = tuple(float(x) for x in p.normal)
        dot = sum(n[i] * radial[i] for i in range(3))
        score = center[1] * 10.0 + max(dot, -5.0) + center[2] * 0.02
        candidates.append((score, p.index, ids, bones[0], center, n, dot))

    if not candidates:
        raise RuntimeError("no same-rigid-bone triangle available for deterministic poke")
    candidates.sort(reverse=True)
    _, face_idx, ids, bone_name, center, normal, dot = candidates[0]

    if dot < 0:
        normal = tuple(-x for x in normal)
    nlen = math.sqrt(sum(x*x for x in normal))
    if nlen < 1e-8:
        raise RuntimeError("chosen face has zero normal")
    normal = tuple(x/nlen for x in normal)
    spike_pos = tuple(center[i] + normal[i]*SPIKE for i in range(3))

    new_vid = len(coords)
    coords.append(spike_pos)

    src_uv = uv_faces[face_idx]
    uv_center = (
        sum(u[0] for u in src_uv) / 3.0,
        sum(u[1] for u in src_uv) / 3.0,
    )
    a,b,c = ids
    ua,ub,uc = src_uv
    src_mat = mats[face_idx]

    out_faces, out_mats, out_uv = [], [], []
    for i,(f,m,u) in enumerate(zip(faces,mats,uv_faces)):
        if i != face_idx:
            out_faces.append(f); out_mats.append(m); out_uv.append(u)
        else:
            out_faces.extend([(a,b,new_vid),(b,c,new_vid),(c,a,new_vid)])
            out_mats.extend([src_mat,src_mat,src_mat])
            out_uv.extend([
                [ua,ub,uv_center],
                [ub,uc,uv_center],
                [uc,ua,uv_center],
            ])

    if len(coords) != EXPECTED_OUT_V or len(out_faces) != EXPECTED_OUT_F:
        raise RuntimeError("unexpected deterministic output counts")

    old_mesh = me
    old_materials = [mat for mat in old_mesh.materials]

    new_mesh = bpy.data.meshes.new(old_mesh.name + "_G3_FREE_TOPOLOGY")
    for mat in old_materials:
        new_mesh.materials.append(mat)
    new_mesh.from_pydata(coords, [], out_faces)
    new_mesh.update(calc_edges=True)

    for p,mi in zip(new_mesh.polygons,out_mats):
        p.material_index = mi

    uv_new = new_mesh.uv_layers.new(name=uv_src.name)
    for p,uvs in zip(new_mesh.polygons,out_uv):
        if len(uvs) != len(p.loop_indices):
            raise RuntimeError("UV loop count mismatch during rebuild")
        for li,uv in zip(p.loop_indices,uvs):
            uv_new.data[li].uv = uv

    # Replace mesh.
    obj.data = new_mesh

    # Blender 5.x can leave the new Mesh with no usable deform-group schema
    # after the datablock swap. Recreate groups by NAME explicitly.
    while len(obj.vertex_groups):
        obj.vertex_groups.remove(obj.vertex_groups[-1])
    recreated = {}
    for name in group_names:
        recreated[name] = obj.vertex_groups.new(name=name)

    if [vg.name for vg in obj.vertex_groups] != group_names:
        raise RuntimeError("vertex group schema recreation mismatch")

    # Restore every old vertex's complete weight set by group name.
    restored_memberships = 0
    for vid,rows in weights_by_vertex.items():
        for name,w in rows:
            vg = recreated.get(name)
            if vg is None:
                raise RuntimeError(f"missing recreated group {name!r}")
            vg.add([vid], w, 'REPLACE')
            restored_memberships += 1

    # New vertex: exactly one rigid bone.
    vg = recreated.get(bone_name)
    if vg is None:
        raise RuntimeError(f"selected rigid bone group not recreated: {bone_name!r}")
    vg.add([new_vid], 1.0, 'REPLACE')

    # Strict weight preservation check for original vertices.
    weight_max_error = 0.0
    for vid,expected in weights_by_vertex.items():
        actual = memberships_by_name(obj,vid)
        ed = {n:w for n,w in expected}
        ad = {n:w for n,w in actual}
        if set(ed) != set(ad):
            raise RuntimeError(f"vertex {vid}: restored group membership differs: expected={ed} actual={ad}")
        for n in ed:
            weight_max_error = max(weight_max_error, abs(ed[n]-ad[n]))
    if weight_max_error > 1e-7:
        raise RuntimeError(f"restored weight error too large: {weight_max_error}")

    new_rows = [(n,w) for n,w in memberships_by_name(obj,new_vid) if w > 1e-6]
    if len(new_rows) != 1 or new_rows[0][0] != bone_name or new_rows[0][1] < 0.999:
        raise RuntimeError(f"new vertex is not rigid: {new_rows}")

    if len(obj.data.vertices) != EXPECTED_OUT_V or len(obj.data.polygons) != EXPECTED_OUT_F:
        raise RuntimeError("post-rebuild topology counts mismatch")
    if len(obj.data.uv_layers) != 1:
        raise RuntimeError("UV layer count changed unexpectedly")
    if len(obj.data.uv_layers.active.data) != sum(len(p.vertices) for p in obj.data.polygons):
        raise RuntimeError("UV loop data count mismatch")
    if len(obj.vertex_groups) != len(group_names):
        raise RuntimeError("vertex group count changed after recreation")

    obj["pt_g3_schema"] = SCHEMA
    obj["pt_g3_operation"] = "POKE_ONE_RIGID_TRIANGLE_OUTWARD"
    obj["pt_g3_source_face"] = int(face_idx)
    obj["pt_g3_new_vertex"] = int(new_vid)
    obj["pt_g3_bone"] = bone_name
    obj["pt_g3_spike"] = float(SPIKE)

    out.parent.mkdir(parents=True,exist_ok=True)
    report.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    rep = {
        "tool":"PT_PIKEMAN_FREE_TOPOLOGY_EDIT_G3_V1_1",
        "profile_id":"pikeman_c01_m4",
        "object":TARGET,
        "source_counts":{"vertices":EXPECTED_V,"faces":EXPECTED_F},
        "output_counts":{"vertices":EXPECTED_OUT_V,"faces":EXPECTED_OUT_F},
        "vertex_delta":1,
        "face_delta":2,
        "operation":"POKE_ONE_RIGID_TRIANGLE_OUTWARD",
        "source_face":face_idx,
        "new_vertex":new_vid,
        "new_vertex_bone":bone_name,
        "spike_distance":SPIKE,
        "material_id":src_mat,
        "uv_layer":uv_src.name,
        "source_vertex_group_count":len(group_names),
        "output_vertex_group_count":len(obj.vertex_groups),
        "restored_weight_memberships":restored_memberships,
        "old_vertex_weight_max_error":weight_max_error,
        "uv_preserved_and_interpolated":True,
        "vertex_group_schema_recreated":True,
        "old_vertex_weights_preserved":True,
        "new_vertex_rigid":True,
        "artist_blend":str(out),
        "edit_check":"PASS",
    }
    report.write_text(json.dumps(rep,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_FREE_TOPOLOGY_EDIT_G3_V1_1")
    print(f"object={TARGET}")
    print(f"source_counts={EXPECTED_V}/{EXPECTED_F}")
    print(f"output_counts={EXPECTED_OUT_V}/{EXPECTED_OUT_F}")
    print("vertex_delta=+1")
    print("face_delta=+2")
    print(f"source_face={face_idx}")
    print(f"new_vertex={new_vid}")
    print(f"new_vertex_bone={bone_name!r}")
    print(f"material_id={src_mat}")
    print(f"spike_distance={SPIKE}")
    print(f"source_vertex_group_count={len(group_names)}")
    print(f"output_vertex_group_count={len(obj.vertex_groups)}")
    print(f"restored_weight_memberships={restored_memberships}")
    print(f"old_vertex_weight_max_error={weight_max_error:.12g}")
    print("uv_preserved_and_interpolated=True")
    print("vertex_group_schema_recreated=True")
    print("old_vertex_weights_preserved=True")
    print("new_vertex_rigid=True")
    print(f"artist_blend={out}")
    print(f"report={report}")
    print("client_modified=False")
    print("free_topology_edit_check=PASS")

if __name__ == "__main__":
    main()

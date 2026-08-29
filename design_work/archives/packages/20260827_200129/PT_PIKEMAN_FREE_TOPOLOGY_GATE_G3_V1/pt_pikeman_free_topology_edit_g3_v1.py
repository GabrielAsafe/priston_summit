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
SCHEMA = "PT_PIKEMAN_FREE_TOPOLOGY_G3_V1"

def rigid_memberships(obj, vid):
    v = obj.data.vertices[vid]
    rows = []
    for g in v.groups:
        if g.weight > 1e-6:
            rows.append((obj.vertex_groups[g.group].name, float(g.weight)))
    return rows

def exact_rigid_bone(obj, vid):
    rows = rigid_memberships(obj, vid)
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
        raise RuntimeError(
            f"{TARGET}: expected {EXPECTED_V}/{EXPECTED_F}, got "
            f"{len(me.vertices)}/{len(me.polygons)}"
        )
    if any(len(p.vertices) != 3 for p in me.polygons):
        raise RuntimeError("G3 V1 requires source triangles")
    if len(me.uv_layers) != 1:
        raise RuntimeError(f"expected exactly one UV layer, got {len(me.uv_layers)}")
    uv_src = me.uv_layers.active or me.uv_layers[0]
    if uv_src is None:
        raise RuntimeError("active UV layer missing")

    # Preserve complete original mesh semantics needed by the free writer.
    coords = [tuple(float(x) for x in v.co) for v in me.vertices]
    faces = [tuple(int(i) for i in p.vertices) for p in me.polygons]
    mats = [int(p.material_index) for p in me.polygons]
    uv_faces = []
    for p in me.polygons:
        uv_faces.append([
            tuple(float(x) for x in uv_src.data[li].uv)
            for li in p.loop_indices
        ])

    group_weights = {}
    for v in me.vertices:
        group_weights[v.index] = [
            (int(g.group), float(g.weight))
            for g in v.groups if g.weight > 1e-8
        ]

    mesh_center = (
        sum(p[0] for p in coords) / len(coords),
        sum(p[1] for p in coords) / len(coords),
        sum(p[2] for p in coords) / len(coords),
    )

    # Pick a front/outward triangle whose three vertices are already rigidly
    # assigned to the same bone. This makes the new center vertex unambiguous.
    candidates = []
    for p in me.polygons:
        ids = tuple(int(i) for i in p.vertices)
        bones = [exact_rigid_bone(obj, i) for i in ids]
        if None in bones or len(set(bones)) != 1:
            continue
        center = tuple(sum(coords[i][a] for i in ids) / 3.0 for a in range(3))
        radial = (
            center[0]-mesh_center[0],
            center[1]-mesh_center[1],
            center[2]-mesh_center[2],
        )
        n = tuple(float(x) for x in p.normal)
        dot = sum(n[i]*radial[i] for i in range(3))
        # Prefer visually exposed +Y faces, then outwardness and height.
        score = center[1]*10.0 + max(dot, -5.0) + center[2]*0.02
        candidates.append((score, p.index, ids, bones[0], center, n, dot))

    if not candidates:
        raise RuntimeError("no same-rigid-bone triangle available for deterministic poke")
    candidates.sort(reverse=True)
    _, face_idx, ids, bone_name, center, normal, dot = candidates[0]

    # Ensure displacement points away from object centroid even if winding is reversed.
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
        sum(u[0] for u in src_uv)/3.0,
        sum(u[1] for u in src_uv)/3.0,
    )
    a,b,c = ids
    ua,ub,uc = src_uv
    src_mat = mats[face_idx]

    out_faces = []
    out_mats = []
    out_uv = []
    for i,(f,m,u) in enumerate(zip(faces,mats,uv_faces)):
        if i != face_idx:
            out_faces.append(f); out_mats.append(m); out_uv.append(u)
            continue
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
    new_mesh = bpy.data.meshes.new(old_mesh.name + "_G3_FREE_TOPOLOGY")
    for mat in old_mesh.materials:
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

    # Replacing Mesh datablock drops deform weights; restore original weights,
    # then give the new vertex exactly one known rigid bone.
    obj.data = new_mesh
    for vid,rows in group_weights.items():
        for gi,w in rows:
            obj.vertex_groups[gi].add([vid], w, 'REPLACE')
    vg = obj.vertex_groups.get(bone_name)
    if vg is None:
        raise RuntimeError(f"selected rigid bone group disappeared: {bone_name!r}")
    vg.add([new_vid], 1.0, 'REPLACE')

    # Strict postconditions.
    if len(obj.data.vertices) != EXPECTED_OUT_V or len(obj.data.polygons) != EXPECTED_OUT_F:
        raise RuntimeError("post-rebuild topology counts mismatch")
    new_rows = rigid_memberships(obj,new_vid)
    if len(new_rows) != 1 or new_rows[0][0] != bone_name or new_rows[0][1] < 0.999:
        raise RuntimeError(f"new vertex is not rigid: {new_rows}")
    if len(obj.data.uv_layers) != 1:
        raise RuntimeError("UV layer count changed unexpectedly")
    if len(obj.data.uv_layers.active.data) != sum(len(p.vertices) for p in obj.data.polygons):
        raise RuntimeError("UV loop data count mismatch")

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
        "tool":"PT_PIKEMAN_FREE_TOPOLOGY_EDIT_G3_V1",
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
        "uv_preserved_and_interpolated":True,
        "old_vertex_weights_preserved":True,
        "new_vertex_rigid":True,
        "artist_blend":str(out),
        "edit_check":"PASS",
    }
    report.write_text(json.dumps(rep,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_FREE_TOPOLOGY_EDIT_G3_V1")
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
    print("uv_preserved_and_interpolated=True")
    print("old_vertex_weights_preserved=True")
    print("new_vertex_rigid=True")
    print(f"artist_blend={out}")
    print(f"report={report}")
    print("client_modified=False")
    print("free_topology_edit_check=PASS")

if __name__ == "__main__":
    main()

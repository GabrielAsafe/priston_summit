#!/usr/bin/env python3
"""
Priestess C01 HIGH topology patch helper V1.

Target:
  MshC01 (HIGH LOD)
  pristine: 173 vertices / 331 faces

Automatically finds the largest edge-connected component of triangular faces
whose three vertices are rigidly assigned to the preferred bone (default
Bip01 Spine1). Selects up to --faces faces from that component.

Each selected face:
  1 triangle -> 3 triangles
  +1 appended center vertex
  +2 net faces

Original 173 vertices remain untouched.
New vertex inherits source face rigid bone, averaged UV, material and smooth flag.
"""

import argparse
import hashlib
import sys
from collections import defaultdict, deque
from pathlib import Path

import bpy
from mathutils import Vector

SOURCE_SHA = "9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"
SOURCE_V = 173
SOURCE_F = 331
TARGET = "MshC01"


def args_parse(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--object", default=TARGET)
    p.add_argument("--faces", type=int, default=8)
    p.add_argument("--bone", default="Bip01 Spine1")
    p.add_argument("--offset", type=float, default=0.60)
    p.add_argument("--source-smd", default="MfbC01.smd")
    p.add_argument("--out", default="blender/MfbC01_high_patch_v1.blend")
    return p.parse_args(argv)


def rigid_bone(obj, vi):
    rows = []
    for e in obj.data.vertices[vi].groups:
        if e.weight > 1e-8:
            if not (0 <= e.group < len(obj.vertex_groups)):
                raise RuntimeError(f"vertex {vi}: bad group index")
            rows.append((obj.vertex_groups[e.group].name, float(e.weight)))
    if len(rows) != 1 or abs(rows[0][1]-1.0) > 1e-5:
        return None
    return rows[0][0]


def capture_groups(obj):
    names = [g.name for g in obj.vertex_groups]
    rows = []
    for v in obj.data.vertices:
        rows.append([(int(e.group), float(e.weight))
                     for e in v.groups if e.weight > 1e-8])
    return names, rows


def restore_groups(obj, names, old_rows, new_bones):
    for g in list(obj.vertex_groups):
        obj.vertex_groups.remove(g)
    groups = [obj.vertex_groups.new(name=n) for n in names]
    name_to_idx = {n:i for i,n in enumerate(names)}

    for vi, memberships in enumerate(old_rows):
        for gi, w in memberships:
            groups[gi].add([vi], w, "REPLACE")

    base = len(old_rows)
    for j, bone in enumerate(new_bones):
        if bone not in name_to_idx:
            raise RuntimeError(f"missing vertex group {bone!r}")
        groups[name_to_idx[bone]].add([base+j], 1.0, "REPLACE")


def face_common_bone(obj, p):
    if len(p.vertices) != 3:
        return None
    bs = [rigid_bone(obj, int(v)) for v in p.vertices]
    if any(b is None for b in bs):
        return None
    return bs[0] if bs[0] == bs[1] == bs[2] else None


def adjacency(mesh, eligible):
    edge_faces = defaultdict(list)
    for fi in eligible:
        ids = [int(v) for v in mesh.polygons[fi].vertices]
        for k in range(3):
            a,b = ids[k], ids[(k+1)%3]
            if a>b: a,b=b,a
            edge_faces[(a,b)].append(fi)

    adj = {fi:set() for fi in eligible}
    for fs in edge_faces.values():
        if len(fs)==2:
            a,b=fs
            adj[a].add(b)
            adj[b].add(a)
    return adj


def components(adj):
    remain = set(adj)
    comps = []
    while remain:
        seed = min(remain)
        q = deque([seed])
        remain.remove(seed)
        comp = []
        while q:
            x=q.popleft()
            comp.append(x)
            for n in sorted(adj[x]):
                if n in remain:
                    remain.remove(n)
                    q.append(n)
        comps.append(comp)
    return comps


def choose_patch(obj, bone, requested):
    mesh = obj.data
    eligible = []
    for p in mesh.polygons:
        if face_common_bone(obj, p) == bone and float(p.area) > 1e-10:
            eligible.append(int(p.index))

    if not eligible:
        raise RuntimeError(f"no suitable {bone!r} triangles in {TARGET}")

    adj = adjacency(mesh, eligible)
    comps = components(adj)
    comps.sort(key=lambda c: (-len(c), min(c)))
    comp = comps[0]

    count = min(requested, len(comp))
    if count < 1:
        raise RuntimeError("requested face count must be >=1")

    # Start at largest-area face in the largest component.
    seed = sorted(comp, key=lambda fi: (-float(mesh.polygons[fi].area), fi))[0]
    comp_set = set(comp)
    seen = {seed}
    q = deque([seed])
    chosen = []

    while q and len(chosen) < count:
        fi=q.popleft()
        chosen.append(fi)
        ns=[n for n in adj[fi] if n in comp_set and n not in seen]
        ns.sort(key=lambda n: (-float(mesh.polygons[n].area), n))
        for n in ns:
            seen.add(n)
            q.append(n)

    if len(chosen) != count:
        raise RuntimeError("internal connected-patch selection failure")
    return chosen, len(comp), seed


def poly_uvs(mesh,p,uvl):
    return [tuple(float(x) for x in uvl.data[li].uv) for li in p.loop_indices]


def main(argv):
    a=args_parse(argv)
    if a.object != TARGET:
        raise RuntimeError("HIGH helper V1 supports only MshC01")

    obj=bpy.data.objects.get(TARGET)
    if obj is None or obj.type!="MESH":
        raise RuntimeError("MshC01 mesh not found")
    mesh=obj.data
    mesh.update()

    if len(mesh.vertices)!=SOURCE_V or len(mesh.polygons)!=SOURCE_F:
        raise RuntimeError(
            f"requires pristine MshC01 {SOURCE_V}/{SOURCE_F}; "
            f"got {len(mesh.vertices)}/{len(mesh.polygons)}"
        )
    if len(mesh.uv_layers)!=1 or mesh.uv_layers.active is None:
        raise RuntimeError("requires exactly one active UV layer")
    if any(len(p.vertices)!=3 for p in mesh.polygons):
        raise RuntimeError("source contains non-triangle polygon")

    src=Path(a.source_smd).expanduser().resolve()
    if not src.is_file():
        raise RuntimeError(f"source SMD missing: {src}")
    src_sha=hashlib.sha256(src.read_bytes()).hexdigest()
    if src_sha!=SOURCE_SHA:
        raise RuntimeError(f"unexpected source SMD SHA256: {src_sha}")

    chosen, component_size, seed = choose_patch(obj,a.bone,a.faces)
    uv_layer=mesh.uv_layers.active

    group_names,old_groups=capture_groups(obj)
    old_vertices=[tuple(float(x) for x in v.co) for v in mesh.vertices]
    old_faces=[tuple(int(x) for x in p.vertices) for p in mesh.polygons]
    old_mats=[int(p.material_index) for p in mesh.polygons]
    old_smooth=[bool(p.use_smooth) for p in mesh.polygons]
    old_uvs=[poly_uvs(mesh,p,uv_layer) for p in mesh.polygons]

    verts=list(old_vertices)
    faces=list(old_faces)
    mats=list(old_mats)
    smooth=list(old_smooth)
    uvs=list(old_uvs)
    new_bones=[]
    rows=[]

    for fi in chosen:
        p=mesh.polygons[fi]
        ia,ib,ic=old_faces[fi]
        bone=face_common_bone(obj,p)
        if bone != a.bone:
            raise RuntimeError("selected face bone changed")

        pa,pb,pc=[Vector(old_vertices[i]) for i in (ia,ib,ic)]
        center=(pa+pb+pc)/3.0
        n=Vector(p.normal)
        if n.length<=1e-12:
            raise RuntimeError(f"face {fi}: zero normal")
        n.normalize()
        pos=center+n*float(a.offset)

        new_idx=len(verts)
        verts.append(tuple(float(x) for x in pos))
        new_bones.append(bone)

        ua,ub,uc=[Vector(x) for x in old_uvs[fi]]
        um=(ua+ub+uc)/3.0

        faces[fi]=(ia,ib,new_idx)
        mats[fi]=old_mats[fi]
        smooth[fi]=old_smooth[fi]
        uvs[fi]=(tuple(ua),tuple(ub),tuple(um))

        faces.extend([(ib,ic,new_idx),(ic,ia,new_idx)])
        mats.extend([old_mats[fi],old_mats[fi]])
        smooth.extend([old_smooth[fi],old_smooth[fi]])
        uvs.extend([
            (tuple(ub),tuple(uc),tuple(um)),
            (tuple(uc),tuple(ua),tuple(um)),
        ])

        rows.append((new_idx,fi,bone,tuple(float(x) for x in pos)))

    new_mesh=bpy.data.meshes.new(mesh.name+"_HighPatchV1")
    for mat in mesh.materials:
        new_mesh.materials.append(mat)
    new_mesh.from_pydata(verts,[],faces)
    new_mesh.update(calc_edges=True)

    nuv=new_mesh.uv_layers.new(name=uv_layer.name)
    for pi,p in enumerate(new_mesh.polygons):
        p.material_index=mats[pi]
        p.use_smooth=smooth[pi]
        for li,uv in zip(p.loop_indices,uvs[pi]):
            nuv.data[li].uv=uv

    obj.data=new_mesh
    restore_groups(obj,group_names,old_groups,new_bones)

    expected_v=SOURCE_V+len(chosen)
    expected_f=SOURCE_F+2*len(chosen)
    if len(obj.data.vertices)!=expected_v or len(obj.data.polygons)!=expected_f:
        raise RuntimeError("post-patch count mismatch")

    for vi in range(SOURCE_V):
        if tuple(float(x) for x in obj.data.vertices[vi].co) != old_vertices[vi]:
            raise RuntimeError(f"original vertex {vi} moved")

    for j,bone in enumerate(new_bones):
        vi=SOURCE_V+j
        if rigid_bone(obj,vi)!=bone:
            raise RuntimeError(f"new vertex {vi}: bone verify failed")

    obj["pt_topology_operation"]="HIGH_CONNECTED_FACE_CENTER_SUBDIVIDE"
    obj["pt_topology_helper_version"]=1
    obj["pt_topology_source_smd_sha256"]=src_sha
    obj["pt_topology_source_vertex_count"]=SOURCE_V
    obj["pt_topology_source_face_count"]=SOURCE_F
    obj["pt_topology_component_size"]=component_size
    obj["pt_topology_seed_face"]=seed
    obj["pt_topology_patch_faces"]=",".join(map(str,chosen))
    obj["pt_topology_bone"]=a.bone
    obj["pt_topology_offset"]=float(a.offset)

    out=Path(a.out).expanduser().resolve()
    out.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    print("PT_C01_HIGH_TOPOLOGY_PATCH_HELPER_V1")
    print("object=MshC01")
    print("operation=HIGH_CONNECTED_FACE_CENTER_SUBDIVIDE")
    print(f"source_vertices={SOURCE_V}")
    print(f"source_faces={SOURCE_F}")
    print(f"preferred_bone={a.bone!r}")
    print(f"largest_component_faces={component_size}")
    print(f"seed_face={seed}")
    print(f"requested_faces={a.faces}")
    print(f"selected_faces={chosen}")
    print(f"selected_face_count={len(chosen)}")
    print(f"offset={float(a.offset):.9g}")
    print(f"new_vertices={len(obj.data.vertices)}")
    print(f"new_faces={len(obj.data.polygons)}")
    print(f"vertex_delta={len(obj.data.vertices)-SOURCE_V}")
    print(f"face_delta={len(obj.data.polygons)-SOURCE_F:+d}")
    for vi,fi,bone,pos in rows:
        print(f"NEW v={vi} source_face={fi} bone={bone!r} "
              f"pos={tuple(round(x,6) for x in pos)}")
    print(f"source_smd_sha256={src_sha}")
    print(f"blend={out}")
    print("high_topology_patch_helper_check=PASS")


if __name__=="__main__":
    try:
        argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
        main(argv)
    except Exception as exc:
        print(f"high_topology_patch_helper_check=FAIL: {type(exc).__name__}: {exc}")
        raise

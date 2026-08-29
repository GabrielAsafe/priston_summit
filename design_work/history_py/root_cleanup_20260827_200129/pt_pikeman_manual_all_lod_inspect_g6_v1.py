#!/usr/bin/env python3
import bpy, hashlib, json, pathlib, struct, sys

TARGETS=("TbhC03","TbmC03","TblC03")
ALL_MESHES=(
 "TbhC03","TbmC03","TblC03",
 "TblC01","TbmC01","TbhC01",
 "TahC01","TamC01","TalC01",
)
SOURCE_COUNTS={
 "TbhC03":(92,152),
 "TbmC03":(74,122),
 "TblC03":(63,100),
}
NON_TARGET_COUNTS={
 "TblC01":(73,112),
 "TbmC01":(74,128),
 "TbhC01":(85,156),
 "TahC01":(222,406),
 "TamC01":(157,282),
 "TalC01":(115,192),
}

def f64(x): return struct.pack("<d",float(x))

def mesh_signature(obj):
    h=hashlib.sha256()
    h.update(b"MATRIX\0")
    for row in obj.matrix_world:
        for x in row: h.update(f64(x))
    me=obj.data
    h.update(struct.pack("<III",len(me.vertices),len(me.polygons),len(me.loops)))
    for v in me.vertices:
        for x in v.co: h.update(f64(x))
    for p in me.polygons:
        h.update(struct.pack("<ii",len(p.vertices),int(p.material_index)))
        for i in p.vertices: h.update(struct.pack("<i",int(i)))
    h.update(struct.pack("<I",len(me.uv_layers)))
    for layer in me.uv_layers:
        h.update(layer.name.encode("utf-8","surrogatepass")+b"\0")
        h.update(b"1" if layer.active else b"0")
        h.update(b"1" if layer.active_render else b"0")
        h.update(struct.pack("<I",len(layer.data)))
        for d in layer.data:
            h.update(f64(d.uv[0])); h.update(f64(d.uv[1]))
    h.update(struct.pack("<I",len(me.materials)))
    names=[vg.name for vg in obj.vertex_groups]
    h.update(struct.pack("<I",len(names)))
    for n in names: h.update(n.encode("utf-8","surrogatepass")+b"\0")
    for v in me.vertices:
        rows=sorted((int(g.group),float(g.weight)) for g in v.groups if g.weight>1e-12)
        h.update(struct.pack("<I",len(rows)))
        for gi,w in rows:
            h.update(struct.pack("<i",gi)); h.update(f64(w))
    return h.hexdigest()

def topology_signature(obj):
    h=hashlib.sha256(); me=obj.data
    h.update(struct.pack("<II",len(me.vertices),len(me.polygons)))
    for p in me.polygons:
        h.update(struct.pack("<ii",len(p.vertices),int(p.material_index)))
        for i in p.vertices: h.update(struct.pack("<i",int(i)))
    return h.hexdigest()

def position_signature(obj):
    h=hashlib.sha256()
    for v in obj.data.vertices:
        for x in v.co: h.update(f64(x))
    return h.hexdigest()

def uv_signature(obj):
    h=hashlib.sha256(); me=obj.data
    h.update(struct.pack("<I",len(me.uv_layers)))
    for layer in me.uv_layers:
        h.update(layer.name.encode("utf-8","surrogatepass")+b"\0")
        for d in layer.data:
            h.update(f64(d.uv[0])); h.update(f64(d.uv[1]))
    return h.hexdigest()

def weight_signature(obj):
    h=hashlib.sha256()
    names=[vg.name for vg in obj.vertex_groups]
    for n in names: h.update(n.encode("utf-8","surrogatepass")+b"\0")
    for v in obj.data.vertices:
        rows=sorted((int(g.group),float(g.weight)) for g in v.groups if g.weight>1e-12)
        h.update(struct.pack("<I",len(rows)))
        for gi,w in rows:
            h.update(struct.pack("<i",gi)); h.update(f64(w))
    return h.hexdigest()

def parts(obj):
    return {
      "mesh":mesh_signature(obj),
      "topology":topology_signature(obj),
      "positions":position_signature(obj),
      "uv":uv_signature(obj),
      "weights":weight_signature(obj),
    }

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=2:
        raise SystemExit("usage: -- <baseline.blend> <report.json>")
    baseline=pathlib.Path(argv[0]).expanduser().resolve()
    report=pathlib.Path(argv[1]).expanduser().resolve()
    if not baseline.is_file():
        raise RuntimeError(f"baseline missing: {baseline}")

    current={}
    for name in ALL_MESHES:
        obj=bpy.data.objects.get(name)
        if obj is None or obj.type!="MESH":
            raise RuntimeError(f"artist blend missing mesh {name}")
        current[name]=obj

    requested_names=tuple(ALL_MESHES)
    with bpy.data.libraries.load(str(baseline),link=False) as (data_from,data_to):
        missing=[n for n in requested_names if n not in data_from.objects]
        if missing:
            raise RuntimeError(f"baseline missing objects: {missing}")
        data_to.objects=list(requested_names)
    loaded_objects=list(data_to.objects)
    if len(loaded_objects)!=len(requested_names):
        raise RuntimeError("baseline object-count load mismatch")
    if any(o is None for o in loaded_objects):
        bad=[requested_names[i] for i,o in enumerate(loaded_objects) if o is None]
        raise RuntimeError(f"failed to load baseline objects: {bad}")
    baseline_objs=dict(zip(requested_names,loaded_objects))

    target_rows=[]
    for name in TARGETS:
        artist=current[name]
        base=baseline_objs[name]
        me=artist.data

        if len(me.uv_layers)!=1 or me.uv_layers.active is None or me.uv_layers.active.name!="SMD_UV":
            raise RuntimeError(f"{name}: expected exactly one active SMD_UV layer")
        if len(artist.vertex_groups)!=11:
            raise RuntimeError(f"{name}: expected 11 vertex groups, got {len(artist.vertex_groups)}")
        used=sorted(set(int(p.material_index) for p in me.polygons))
        if any(i!=0 for i in used):
            raise RuntimeError(f"{name}: only material 0 permitted, got {used}")

        ap=parts(artist); bp=parts(base)
        changed=ap["mesh"]!=bp["mesh"]
        if not changed:
            raise RuntimeError(f"{name}: G6 requires a manual change on every LOD target")

        src=SOURCE_COUNTS[name]
        if (len(base.data.vertices),len(base.data.polygons))!=src:
            raise RuntimeError(f"{name}: baseline counts drift")

        row={
          "object":name,
          "source_counts":{"vertices":src[0],"faces":src[1]},
          "artist_counts":{"vertices":len(me.vertices),"faces":len(me.polygons)},
          "vertex_delta":len(me.vertices)-src[0],
          "face_delta":len(me.polygons)-src[1],
          "changed":True,
          "topology_changed":ap["topology"]!=bp["topology"],
          "positions_changed":ap["positions"]!=bp["positions"],
          "uv_changed":ap["uv"]!=bp["uv"],
          "weights_changed":ap["weights"]!=bp["weights"],
          "used_materials":used,
          "uv_layer":"SMD_UV",
          "vertex_group_count":len(artist.vertex_groups),
          "artist_signature":ap,
          "baseline_signature":bp,
        }
        target_rows.append(row)

    non_target_rows=[]
    for name,expected in NON_TARGET_COUNTS.items():
        artist=current[name]
        base=baseline_objs[name]
        counts=(len(artist.data.vertices),len(artist.data.polygons))
        if counts!=expected:
            raise RuntimeError(f"{name}: non-target count drift {counts} != {expected}")
        same=mesh_signature(artist)==mesh_signature(base)
        if not same:
            raise RuntimeError(f"{name}: non-target changed in artist master")
        non_target_rows.append({
          "object":name,
          "counts":counts,
          "unchanged":True,
        })

    data={
      "tool":"PT_PIKEMAN_MANUAL_ALL_LOD_INSPECT_G6_V1",
      "profile_id":"pikeman_c01_m4",
      "targets":target_rows,
      "target_count":len(target_rows),
      "all_targets_changed":all(x["changed"] for x in target_rows),
      "non_targets":non_target_rows,
      "non_target_unchanged_count":len(non_target_rows),
      "client_modified":False,
      "inspect_check":"PASS",
    }
    report.parent.mkdir(parents=True,exist_ok=True)
    report.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_MANUAL_ALL_LOD_INSPECT_G6_V1")
    print(f"targets={list(TARGETS)}")
    for row in target_rows:
        print(
          f"OBJECT {row['object']}: "
          f"source={row['source_counts']['vertices']}/{row['source_counts']['faces']} "
          f"artist={row['artist_counts']['vertices']}/{row['artist_counts']['faces']} "
          f"vertex_delta={row['vertex_delta']:+d} face_delta={row['face_delta']:+d} "
          f"topology_changed={row['topology_changed']} "
          f"positions_changed={row['positions_changed']} "
          f"uv_changed={row['uv_changed']} "
          f"weights_changed={row['weights_changed']}"
        )
    print("all_targets_changed=True")
    print("used_materials_all_targets=[0]")
    print("uv_layer_all_targets='SMD_UV'")
    print("vertex_group_count_all_targets=11")
    print(f"non_target_unchanged_count={len(non_target_rows)}")
    print(f"report={report}")
    print("client_modified=False")
    print("g6_artist_inspect_check=PASS")

if __name__=="__main__":
    main()

#!/usr/bin/env python3
import bpy, hashlib, json, pathlib, struct, sys

TARGET="TbmC03"
MESH_NAMES=[
 "TbhC03","TbmC03","TblC03",
 "TblC01","TbmC01","TbhC01",
 "TahC01","TamC01","TalC01",
]
EXPECTED_NON_TARGET_COUNTS={
 "TbhC03":(92,152),"TblC03":(63,100),
 "TblC01":(73,112),"TbmC01":(74,128),"TbhC01":(85,156),
 "TahC01":(222,406),"TamC01":(157,282),"TalC01":(115,192),
}

def f64(x):
    return struct.pack("<d",float(x))

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

    # Do not hash material datablock names: appending the baseline into an
    # already-open .blend may cause Blender to suffix datablock names (.001).
    # Material semantics relevant to geometry are captured by slot count and
    # per-polygon material_index above.
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
    h=hashlib.sha256()
    me=obj.data
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
    h=hashlib.sha256()
    me=obj.data
    h.update(struct.pack("<I",len(me.uv_layers)))
    for layer in me.uv_layers:
        h.update(layer.name.encode()+b"\0")
        for d in layer.data:
            h.update(f64(d.uv[0])); h.update(f64(d.uv[1]))
    return h.hexdigest()

def weight_signature(obj):
    h=hashlib.sha256()
    names=[vg.name for vg in obj.vertex_groups]
    for n in names: h.update(n.encode()+b"\0")
    for v in obj.data.vertices:
        rows=sorted((int(g.group),float(g.weight)) for g in v.groups if g.weight>1e-12)
        h.update(struct.pack("<I",len(rows)))
        for gi,w in rows:
            h.update(struct.pack("<i",gi)); h.update(f64(w))
    return h.hexdigest()

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=2:
        raise SystemExit("usage: -- <baseline.blend> <report.json>")
    baseline=pathlib.Path(argv[0]).expanduser().resolve()
    report=pathlib.Path(argv[1]).expanduser().resolve()
    if not baseline.exists():
        raise RuntimeError(f"baseline missing: {baseline}")

    current={}
    for name in MESH_NAMES:
        obj=bpy.data.objects.get(name)
        if obj is None or obj.type!="MESH":
            raise RuntimeError(f"artist blend missing mesh {name}")
        current[name]=obj

    target=current[TARGET]
    me=target.data
    if len(me.uv_layers)!=1 or me.uv_layers.active is None or me.uv_layers.active.name!="SMD_UV":
        raise RuntimeError(f"{TARGET}: expected exactly one active UV layer named SMD_UV")
    if len(target.vertex_groups)!=11:
        raise RuntimeError(f"{TARGET}: expected 11 vertex groups, got {len(target.vertex_groups)}")
    used_materials=sorted(set(int(p.material_index) for p in me.polygons))
    if any(i != 0 for i in used_materials):
        raise RuntimeError(f"{TARGET}: manual G5 permits only source material 0, got {used_materials}")

    artist_sigs={n:mesh_signature(o) for n,o in current.items()}
    artist_target_parts={
      "mesh":artist_sigs[TARGET],
      "topology":topology_signature(target),
      "positions":position_signature(target),
      "uv":uv_signature(target),
      "weights":weight_signature(target),
    }

    with bpy.data.libraries.load(str(baseline),link=False) as (data_from,data_to):
        missing=[n for n in MESH_NAMES if n not in data_from.objects]
        if missing: raise RuntimeError(f"baseline missing objects: {missing}")
        data_to.objects=MESH_NAMES

    loaded={orig:obj for orig,obj in zip(MESH_NAMES,data_to.objects)}
    if any(v is None for v in loaded.values()):
        raise RuntimeError("failed to load baseline objects")

    baseline_sigs={n:mesh_signature(o) for n,o in loaded.items()}
    base_target=loaded[TARGET]
    baseline_target_parts={
      "mesh":baseline_sigs[TARGET],
      "topology":topology_signature(base_target),
      "positions":position_signature(base_target),
      "uv":uv_signature(base_target),
      "weights":weight_signature(base_target),
    }

    non_target=[]
    for name in MESH_NAMES:
        if name==TARGET: continue
        a=current[name]
        expected=EXPECTED_NON_TARGET_COUNTS[name]
        counts=(len(a.data.vertices),len(a.data.polygons))
        same=artist_sigs[name]==baseline_sigs[name]
        non_target.append({"object":name,"counts":counts,"expected_counts":expected,"unchanged":same})
        if counts!=expected:
            raise RuntimeError(f"{name}: unexpected counts {counts}, expected {expected}")
        if not same:
            raise RuntimeError(f"{name}: artist file changed a non-target mesh/transform/UV/weights")

    target_changed=artist_target_parts["mesh"]!=baseline_target_parts["mesh"]
    if not target_changed:
        raise RuntimeError(f"{TARGET}: no manual geometry/UV/weight/transform change detected")

    source_counts=(len(base_target.data.vertices),len(base_target.data.polygons))
    artist_counts=(len(target.data.vertices),len(target.data.polygons))
    topology_changed=artist_target_parts["topology"]!=baseline_target_parts["topology"]
    positions_changed=artist_target_parts["positions"]!=baseline_target_parts["positions"]
    uv_changed=artist_target_parts["uv"]!=baseline_target_parts["uv"]
    weights_changed=artist_target_parts["weights"]!=baseline_target_parts["weights"]

    report.parent.mkdir(parents=True,exist_ok=True)
    data={
      "tool":"PT_PIKEMAN_MANUAL_ARTIST_INSPECT_G5_V1",
      "profile_id":"pikeman_c01_m4",
      "target":TARGET,
      "source_counts":{"vertices":source_counts[0],"faces":source_counts[1]},
      "artist_counts":{"vertices":artist_counts[0],"faces":artist_counts[1]},
      "vertex_delta":artist_counts[0]-source_counts[0],
      "face_delta":artist_counts[1]-source_counts[1],
      "target_changed":target_changed,
      "topology_changed":topology_changed,
      "positions_changed":positions_changed,
      "uv_changed":uv_changed,
      "weights_changed":weights_changed,
      "used_materials":used_materials,
      "uv_layer":"SMD_UV",
      "vertex_group_count":len(target.vertex_groups),
      "non_target_objects":non_target,
      "non_target_unchanged_count":sum(1 for x in non_target if x["unchanged"]),
      "artist_target_signature":artist_target_parts,
      "baseline_target_signature":baseline_target_parts,
      "client_modified":False,
      "manual_artist_inspect_check":"PASS",
    }
    report.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_MANUAL_ARTIST_INSPECT_G5_V1")
    print(f"target={TARGET}")
    print(f"source_counts={source_counts[0]}/{source_counts[1]}")
    print(f"artist_counts={artist_counts[0]}/{artist_counts[1]}")
    print(f"vertex_delta={artist_counts[0]-source_counts[0]:+d}")
    print(f"face_delta={artist_counts[1]-source_counts[1]:+d}")
    print(f"target_changed={target_changed}")
    print(f"topology_changed={topology_changed}")
    print(f"positions_changed={positions_changed}")
    print(f"uv_changed={uv_changed}")
    print(f"weights_changed={weights_changed}")
    print(f"used_materials={used_materials}")
    print("uv_layer='SMD_UV'")
    print(f"vertex_group_count={len(target.vertex_groups)}")
    print(f"non_target_unchanged_count={sum(1 for x in non_target if x['unchanged'])}")
    print(f"artist_target_sha256={artist_target_parts['mesh']}")
    print(f"report={report}")
    print("client_modified=False")
    print("manual_artist_inspect_check=PASS")

if __name__=="__main__":
    main()

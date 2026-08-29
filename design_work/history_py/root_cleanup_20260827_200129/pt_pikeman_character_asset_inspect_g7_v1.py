#!/usr/bin/env python3
import bpy, hashlib, json, pathlib, struct, sys

TARGETS=(
 "TbhC03","TbmC03","TblC03",
 "TblC01","TbmC01","TbhC01",
 "TahC01","TamC01","TalC01",
)
SOURCE_COUNTS={
 "TbhC03":(92,152),
 "TbmC03":(74,122),
 "TblC03":(63,100),
 "TblC01":(73,112),
 "TbmC01":(74,128),
 "TbhC01":(85,156),
 "TahC01":(222,406),
 "TamC01":(157,282),
 "TalC01":(115,192),
}
ALLOWED_MATERIAL={
 "TbhC03":0,"TbmC03":0,"TblC03":0,
 "TblC01":1,"TbmC01":1,"TbhC01":1,
 "TahC01":1,"TamC01":1,"TalC01":1,
}

def f64(x): return struct.pack("<d",float(x))

def topology_signature(obj):
    h=hashlib.sha256(); me=obj.data
    h.update(struct.pack("<III",len(me.vertices),len(me.polygons),len(me.loops)))
    for p in me.polygons:
        h.update(struct.pack("<ii",len(p.vertices),int(p.material_index)))
        for i in p.vertices:
            h.update(struct.pack("<i",int(i)))
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
        h.update(b"1" if layer.active else b"0")
        h.update(b"1" if layer.active_render else b"0")
        h.update(struct.pack("<I",len(layer.data)))
        for d in layer.data:
            h.update(f64(d.uv[0])); h.update(f64(d.uv[1]))
    return h.hexdigest()

def weight_signature(obj):
    h=hashlib.sha256()
    groups=[g.name for g in obj.vertex_groups]
    h.update(struct.pack("<I",len(groups)))
    for n in groups:
        h.update(n.encode("utf-8","surrogatepass")+b"\0")
    for v in obj.data.vertices:
        rows=sorted((int(g.group),float(g.weight)) for g in v.groups if g.weight>1e-12)
        h.update(struct.pack("<I",len(rows)))
        for gi,w in rows:
            h.update(struct.pack("<i",gi)); h.update(f64(w))
    return h.hexdigest()

def transform_signature(obj):
    h=hashlib.sha256()
    for row in obj.matrix_world:
        for x in row: h.update(f64(x))
    return h.hexdigest()

def parts(obj):
    return {
      "topology":topology_signature(obj),
      "positions":position_signature(obj),
      "uv":uv_signature(obj),
      "weights":weight_signature(obj),
      "transform":transform_signature(obj),
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
    for name in TARGETS:
        obj=bpy.data.objects.get(name)
        if obj is None or obj.type!="MESH":
            raise RuntimeError(f"artist blend missing mesh {name}")
        current[name]=obj

    requested=tuple(TARGETS)
    with bpy.data.libraries.load(str(baseline),link=False) as (data_from,data_to):
        missing=[n for n in requested if n not in data_from.objects]
        if missing:
            raise RuntimeError(f"baseline missing objects: {missing}")
        data_to.objects=list(requested)
    loaded=list(data_to.objects)
    if len(loaded)!=len(requested):
        raise RuntimeError("baseline load count mismatch")
    if any(o is None for o in loaded):
        bad=[requested[i] for i,o in enumerate(loaded) if o is None]
        raise RuntimeError(f"failed to load baseline objects: {bad}")
    base=dict(zip(requested,loaded))

    rows=[]
    unchanged=[]
    violations=[]
    for name in TARGETS:
        artist=current[name]; baseline_obj=base[name]
        me=artist.data; bme=baseline_obj.data
        src=SOURCE_COUNTS[name]
        if (len(bme.vertices),len(bme.polygons))!=src:
            violations.append(f"{name}: baseline count drift")

        # Preserve UV schema exactly.
        uv_names=[u.name for u in me.uv_layers]
        base_uv_names=[u.name for u in bme.uv_layers]
        if uv_names!=base_uv_names or uv_names!=["SMD_UV"]:
            violations.append(f"{name}: UV schema {uv_names} != ['SMD_UV']")
        if me.uv_layers.active is None or me.uv_layers.active.name!="SMD_UV":
            violations.append(f"{name}: SMD_UV not active")

        # Preserve group schema names/order; topology may add vertices but not bones/groups.
        groups=[g.name for g in artist.vertex_groups]
        base_groups=[g.name for g in baseline_obj.vertex_groups]
        if groups!=base_groups:
            violations.append(f"{name}: vertex-group schema drift")

        # Imported objects must remain armature-bound.
        if not any(m.type=="ARMATURE" for m in artist.modifiers):
            violations.append(f"{name}: missing ARMATURE modifier")

        expected_mat=ALLOWED_MATERIAL[name]
        used=sorted(set(int(p.material_index) for p in me.polygons))
        if used!=[expected_mat]:
            violations.append(f"{name}: used materials {used} != [{expected_mat}]")
        if len(me.materials)!=len(bme.materials):
            violations.append(
              f"{name}: material-slot count drift {len(me.materials)} != {len(bme.materials)}"
            )

        ap=parts(artist); bp=parts(baseline_obj)
        flags={
          "topology_changed":ap["topology"]!=bp["topology"],
          "positions_changed":ap["positions"]!=bp["positions"],
          "uv_changed":ap["uv"]!=bp["uv"],
          "weights_changed":ap["weights"]!=bp["weights"],
          "transform_changed":ap["transform"]!=bp["transform"],
        }
        changed=any(flags.values())
        if not changed:
            unchanged.append(name)

        rows.append({
          "object":name,
          "source_counts":{"vertices":src[0],"faces":src[1]},
          "artist_counts":{"vertices":len(me.vertices),"faces":len(me.polygons)},
          "vertex_delta":len(me.vertices)-src[0],
          "face_delta":len(me.polygons)-src[1],
          "allowed_material":expected_mat,
          "used_materials":used,
          "uv_layer_names":uv_names,
          "vertex_group_count":len(groups),
          "changed":changed,
          **flags,
        })

    data={
      "tool":"PT_PIKEMAN_CHARACTER_ASSET_INSPECT_G7_V1",
      "profile_id":"pikeman_c01_m4",
      "targets":rows,
      "target_count":len(rows),
      "unchanged_targets":unchanged,
      "violations":violations,
      "all_targets_changed":not unchanged,
      "client_modified":False,
      "inspect_check":"PASS" if not unchanged and not violations else "FAIL",
    }
    report.parent.mkdir(parents=True,exist_ok=True)
    report.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_CHARACTER_ASSET_INSPECT_G7_V1")
    for row in rows:
        print(
          f"OBJECT {row['object']}: "
          f"source={row['source_counts']['vertices']}/{row['source_counts']['faces']} "
          f"artist={row['artist_counts']['vertices']}/{row['artist_counts']['faces']} "
          f"vertex_delta={row['vertex_delta']:+d} face_delta={row['face_delta']:+d} "
          f"material={row['used_materials']} "
          f"topology_changed={row['topology_changed']} "
          f"positions_changed={row['positions_changed']} "
          f"uv_changed={row['uv_changed']} "
          f"weights_changed={row['weights_changed']} "
          f"transform_changed={row['transform_changed']} "
          f"changed={row['changed']}"
        )
    print(f"unchanged_targets={unchanged}")
    print(f"violations={violations}")
    print(f"all_targets_changed={not unchanged}")
    print(f"target_count={len(rows)}")
    print(f"report={report}")
    print("client_modified=False")

    if violations:
        raise RuntimeError("G7 artist schema violations: "+ "; ".join(violations))
    if unchanged:
        raise RuntimeError("G7 requires manual changes on every mesh: "+", ".join(unchanged))
    print("g7_artist_inspect_check=PASS")

if __name__=="__main__":
    main()

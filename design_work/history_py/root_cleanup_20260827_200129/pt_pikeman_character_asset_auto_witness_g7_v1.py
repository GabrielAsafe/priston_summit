#!/usr/bin/env python3
import bpy, hashlib, json, pathlib, sys

TARGETS = {
    "TblC01": 1.20,
    "TbmC01": 1.20,
    "TbhC01": 1.20,
    "TahC01": 1.12,
    "TamC01": 1.12,
    "TalC01": 1.12,
}

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for b in iter(lambda: f.read(1024 * 1024), b""):
            h.update(b)
    return h.hexdigest()

def main():
    argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv) != 2:
        raise SystemExit("usage: -- <output.blend> <report.json>")

    output = pathlib.Path(argv[0]).expanduser().resolve()
    report = pathlib.Path(argv[1]).expanduser().resolve()

    rows = []
    for name, factor in TARGETS.items():
        obj = bpy.data.objects.get(name)
        if obj is None or obj.type != "MESH":
            raise RuntimeError(f"missing mesh: {name}")

        me = obj.data
        if len(me.vertices) == 0:
            raise RuntimeError(f"{name}: empty mesh")

        # Strict semantic preflight.
        uv_names = [u.name for u in me.uv_layers]
        if uv_names != ["SMD_UV"] or me.uv_layers.active is None or me.uv_layers.active.name != "SMD_UV":
            raise RuntimeError(f"{name}: invalid UV schema: {uv_names}")

        used_materials = sorted(set(int(p.material_index) for p in me.polygons))
        if used_materials != [1]:
            raise RuntimeError(f"{name}: expected material 1 only, got {used_materials}")

        if not any(m.type == "ARMATURE" for m in obj.modifiers):
            raise RuntimeError(f"{name}: ARMATURE modifier missing")

        group_names_before = [g.name for g in obj.vertex_groups]
        weights_before = [
            sorted((int(g.group), float(g.weight)) for g in v.groups if g.weight > 1e-12)
            for v in me.vertices
        ]

        cx = sum(v.co.x for v in me.vertices) / len(me.vertices)
        cy = sum(v.co.y for v in me.vertices) / len(me.vertices)

        max_move = 0.0
        moved = 0
        for v in me.vertices:
            ox, oy, oz = float(v.co.x), float(v.co.y), float(v.co.z)
            nx = cx + (ox - cx) * factor
            ny = cy + (oy - cy) * factor
            v.co.x = nx
            v.co.y = ny
            dx, dy = nx - ox, ny - oy
            d = (dx*dx + dy*dy) ** 0.5
            if d > 1e-12:
                moved += 1
                if d > max_move:
                    max_move = d

        # Nothing except positions may have changed.
        if [u.name for u in me.uv_layers] != uv_names:
            raise RuntimeError(f"{name}: UV schema changed unexpectedly")
        if [g.name for g in obj.vertex_groups] != group_names_before:
            raise RuntimeError(f"{name}: vertex-group schema changed unexpectedly")
        weights_after = [
            sorted((int(g.group), float(g.weight)) for g in v.groups if g.weight > 1e-12)
            for v in me.vertices
        ]
        if weights_after != weights_before:
            raise RuntimeError(f"{name}: weights changed unexpectedly")

        rows.append({
            "object": name,
            "factor_xy": factor,
            "vertices": len(me.vertices),
            "faces": len(me.polygons),
            "moved_vertices": moved,
            "max_xy_move": max_move,
            "used_materials": used_materials,
            "uv_layers": uv_names,
            "vertex_group_count": len(group_names_before),
            "topology_changed": False,
            "uv_changed": False,
            "weights_changed": False,
            "positions_changed": moved > 0,
        })

    output.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(output))

    if not output.is_file():
        raise RuntimeError("output blend was not created")

    data = {
        "tool": "PT_PIKEMAN_CHARACTER_ASSET_AUTO_WITNESS_G7_V1",
        "operation": "XY_SCALE_AROUND_LOCAL_CENTROID",
        "targets": rows,
        "target_count": len(rows),
        "output_blend": str(output),
        "output_sha256": sha256(output),
        "client_modified": False,
        "witness_check": "PASS",
    }
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    print("PT_PIKEMAN_CHARACTER_ASSET_AUTO_WITNESS_G7_V1")
    for row in rows:
        print(
            f"OBJECT {row['object']}: factor_xy={row['factor_xy']:.2f} "
            f"V={row['vertices']} F={row['faces']} "
            f"moved_vertices={row['moved_vertices']} "
            f"max_xy_move={row['max_xy_move']:.6f} "
            f"material={row['used_materials']} "
            f"uv={row['uv_layers']} "
            f"groups={row['vertex_group_count']}"
        )
    print(f"output={output}")
    print(f"output_sha256={data['output_sha256']}")
    print(f"report={report}")
    print("client_modified=False")
    print("auto_witness_check=PASS")

if __name__ == "__main__":
    main()

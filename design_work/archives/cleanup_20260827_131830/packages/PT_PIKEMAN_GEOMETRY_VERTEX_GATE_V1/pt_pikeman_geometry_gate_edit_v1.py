#!/usr/bin/env python3
import bpy
import pathlib
import sys
import math

TARGET = "TbmC03"
EXPECTED_VERTS = 74
EXPECTED_FACES = 122
SCALE_XY = 1.20
SCHEMA = "PT_PIKEMAN_TBMC03_GEOMETRY_VERTEX_GATE_V1"

def matrix_world_is_identity(obj, eps=1e-8):
    ident = (
        (1.0,0.0,0.0,0.0),
        (0.0,1.0,0.0,0.0),
        (0.0,0.0,1.0,0.0),
        (0.0,0.0,0.0,1.0),
    )
    for r in range(4):
        for c in range(4):
            if abs(float(obj.matrix_world[r][c]) - ident[r][c]) > eps:
                return False
    return True

def main():
    argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv) != 1:
        raise SystemExit("usage: -- <out.blend>")
    out = pathlib.Path(argv[0]).expanduser().resolve()

    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError(f"missing mesh object {TARGET!r}")
    if not matrix_world_is_identity(obj):
        raise RuntimeError(f"{TARGET}: matrix_world must be identity for this controlled gate")

    me = obj.data
    if len(me.vertices) != EXPECTED_VERTS:
        raise RuntimeError(f"{TARGET}: expected {EXPECTED_VERTS} vertices, got {len(me.vertices)}")
    if len(me.polygons) != EXPECTED_FACES:
        raise RuntimeError(f"{TARGET}: expected {EXPECTED_FACES} polygons, got {len(me.polygons)}")
    if any(len(p.vertices) != 3 for p in me.polygons):
        raise RuntimeError(f"{TARGET}: gate requires already-triangulated topology")

    # This gate deliberately changes positions only.
    cx = sum(float(v.co.x) for v in me.vertices) / len(me.vertices)
    cy = sum(float(v.co.y) for v in me.vertices) / len(me.vertices)
    cz = sum(float(v.co.z) for v in me.vertices) / len(me.vertices)

    before = [(float(v.co.x), float(v.co.y), float(v.co.z)) for v in me.vertices]
    for v in me.vertices:
        v.co.x = cx + (float(v.co.x) - cx) * SCALE_XY
        v.co.y = cy + (float(v.co.y) - cy) * SCALE_XY

    after = [(float(v.co.x), float(v.co.y), float(v.co.z)) for v in me.vertices]
    moved = [
        math.dist(a, b)
        for a, b in zip(before, after)
    ]
    moved_count = sum(1 for d in moved if d > 1e-9)
    if moved_count == 0:
        raise RuntimeError("controlled geometry edit moved no vertices")

    obj["pt_geometry_gate_schema"] = SCHEMA
    obj["pt_geometry_gate_target"] = TARGET
    obj["pt_geometry_gate_scale_xy"] = SCALE_XY
    obj["pt_geometry_gate_center_x"] = cx
    obj["pt_geometry_gate_center_y"] = cy
    obj["pt_geometry_gate_center_z"] = cz
    obj["pt_geometry_gate_expected_vertices"] = EXPECTED_VERTS
    obj["pt_geometry_gate_expected_faces"] = EXPECTED_FACES
    obj["pt_geometry_gate_positions_only"] = True

    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    print("PT_PIKEMAN_GEOMETRY_GATE_EDIT_V1")
    print(f"object={TARGET}")
    print(f"vertices={len(me.vertices)}")
    print(f"faces={len(me.polygons)}")
    print(f"center=({cx:.9f},{cy:.9f},{cz:.9f})")
    print(f"scale_xy={SCALE_XY}")
    print(f"moved_vertices={moved_count}")
    print(f"max_move={max(moved):.9f}")
    print("topology_changed=False")
    print("uv_changed=False")
    print("material_changed=False")
    print("physique_changed=False")
    print(f"output_blend={out}")
    print("geometry_gate_edit_check=PASS")

if __name__ == "__main__":
    main()

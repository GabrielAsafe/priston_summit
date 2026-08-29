#!/usr/bin/env python3
"""
PT BLENDER UV STORAGE PROBE V2

Read-only diagnostic for Blender Edit Mode / BMesh UV storage.

Checks:
1. Mesh RNA UV storage before mode switch.
2. Edit BMesh loop UV layer and samples.
3. Forces OBJECT mode in memory (does NOT save).
4. Mesh RNA UV storage after mode switch.

No .blend/SMD/client file is written.
"""

import bpy
import bmesh

TARGET = "MshC01"


def safe_len(x):
    try:
        return len(x)
    except Exception as exc:
        return f"ERR:{type(exc).__name__}:{exc}"


def mesh_dump(label, obj):
    mesh = obj.data
    print(f"\n[{label}]")
    print(f"object_mode={obj.mode}")
    print(f"context_mode={bpy.context.mode}")
    print(f"vertices={len(mesh.vertices)}")
    print(f"edges={len(mesh.edges)}")
    print(f"polygons={len(mesh.polygons)}")
    print(f"loops={len(mesh.loops)}")
    print(f"uv_layer_count={len(mesh.uv_layers)}")
    print(f"active_uv={mesh.uv_layers.active.name if mesh.uv_layers.active else None!r}")

    for i, uv in enumerate(mesh.uv_layers):
        print(
            "UV_LAYER "
            f"index={i} "
            f"name={uv.name!r} "
            f"data_len={safe_len(uv.data)}"
        )

    for i, attr in enumerate(mesh.attributes):
        print(
            "ATTRIBUTE "
            f"index={i} "
            f"name={attr.name!r} "
            f"domain={getattr(attr, 'domain', None)!r} "
            f"type={getattr(attr, 'data_type', None)!r} "
            f"data_len={safe_len(attr.data)}"
        )


def bmesh_dump(obj):
    print("\n[EDIT_BMESH]")
    if obj.mode != "EDIT":
        print("bmesh_available=False reason=object_not_in_edit_mode")
        return

    bm = bmesh.from_edit_mesh(obj.data)
    bm.verts.ensure_lookup_table()
    bm.faces.ensure_lookup_table()

    print("bmesh_available=True")
    print(f"bm_vertices={len(bm.verts)}")
    print(f"bm_edges={len(bm.edges)}")
    print(f"bm_faces={len(bm.faces)}")
    print(f"bm_loops={sum(len(f.loops) for f in bm.faces)}")

    layers = list(bm.loops.layers.uv.values())
    print(f"bm_uv_layer_count={len(layers)}")
    print(f"bm_uv_layer_names={[layer.name for layer in layers]}")

    active = bm.loops.layers.uv.active
    print(f"bm_active_uv={active.name if active else None!r}")

    if active is None:
        print("bm_uv_sample=[]")
        return

    sample = []
    total = 0
    nonfinite = 0

    for f in bm.faces:
        for loop in f.loops:
            uv = loop[active].uv
            total += 1
            u = float(uv.x)
            v = float(uv.y)
            if not (-1e100 < u < 1e100 and -1e100 < v < 1e100):
                nonfinite += 1
            if len(sample) < 8:
                sample.append((f.index, loop.vert.index, u, v))

    print(f"bm_uv_corner_count={total}")
    print(f"bm_uv_nonfinite={nonfinite}")
    print(f"bm_uv_sample={sample}")


def force_object_mode(obj):
    print("\n[MODE_SWITCH]")
    try:
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)

        if obj.mode == "EDIT":
            result = bpy.ops.object.mode_set(mode="OBJECT")
            print(f"mode_set_result={result}")
        else:
            print("mode_set_result=NOT_NEEDED")

        obj.data.update()
        print(f"object_mode_after={obj.mode}")
        print(f"context_mode_after={bpy.context.mode}")
    except Exception as exc:
        print(f"mode_set_error={type(exc).__name__}: {exc}")
        raise


def main():
    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    mesh_dump("BEFORE_OBJECT_MODE", obj)
    bmesh_dump(obj)
    force_object_mode(obj)
    mesh_dump("AFTER_OBJECT_MODE", obj)

    uv = obj.data.uv_layers.active
    if uv is not None:
        print(f"\nFINAL_UV_DATA_LEN={len(uv.data)}")
        print(f"FINAL_LOOP_COUNT={len(obj.data.loops)}")
        if len(uv.data) == len(obj.data.loops):
            print("uv_storage_object_mode_check=PASS")
        else:
            print("uv_storage_object_mode_check=FAIL")
    else:
        print("\nFINAL_UV_DATA_LEN=NO_ACTIVE_UV")
        print("uv_storage_object_mode_check=FAIL")

    print("uv_storage_probe_v2_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"uv_storage_probe_v2_check=FAIL: {type(exc).__name__}: {exc}")
        raise

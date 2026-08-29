#!/usr/bin/env python3
"""
PT BLENDER UV STORAGE PROBE V1
Read-only diagnostic for Blender 5.x UV representation.

Prints:
- object/mode
- mesh counts
- UV layer metadata/data lengths
- generic mesh attributes and their domains/types/lengths
- optional Edit Mode synchronization effect

Does not save the .blend and does not write SMD/client files.
"""

import sys
import bpy

TARGET = "MshC01"


def safe_len(x):
    try:
        return len(x)
    except Exception as exc:
        return f"ERR:{type(exc).__name__}:{exc}"


def dump(label, obj):
    mesh = obj.data
    print(f"\n[{label}]")
    print(f"object={obj.name}")
    print(f"object_mode={obj.mode}")
    print(f"context_mode={bpy.context.mode}")
    print(f"vertices={len(mesh.vertices)}")
    print(f"edges={len(mesh.edges)}")
    print(f"polygons={len(mesh.polygons)}")
    print(f"loops={len(mesh.loops)}")

    print(f"uv_layer_count={len(mesh.uv_layers)}")
    print(f"active_uv_index={getattr(mesh.uv_layers, 'active_index', None)}")
    print(f"active_uv_name={mesh.uv_layers.active.name if mesh.uv_layers.active else None!r}")

    for i, uv in enumerate(mesh.uv_layers):
        print(
            "UV_LAYER "
            f"index={i} "
            f"name={uv.name!r} "
            f"active={uv == mesh.uv_layers.active} "
            f"active_render={getattr(uv, 'active_render', None)} "
            f"data_len={safe_len(uv.data)}"
        )

    print(f"attribute_count={len(mesh.attributes)}")
    for i, attr in enumerate(mesh.attributes):
        print(
            "ATTRIBUTE "
            f"index={i} "
            f"name={attr.name!r} "
            f"domain={getattr(attr, 'domain', None)!r} "
            f"data_type={getattr(attr, 'data_type', None)!r} "
            f"data_len={safe_len(attr.data)}"
        )

    print(f"color_attribute_count={len(mesh.color_attributes)}")
    for i, attr in enumerate(mesh.color_attributes):
        print(
            "COLOR_ATTRIBUTE "
            f"index={i} "
            f"name={attr.name!r} "
            f"domain={getattr(attr, 'domain', None)!r} "
            f"data_type={getattr(attr, 'data_type', None)!r} "
            f"data_len={safe_len(attr.data)}"
        )

    # Specifically inspect likely UV-related generic attributes.
    for name in ("SMD_UV", ".corner_vert", ".corner_edge"):
        attr = mesh.attributes.get(name)
        if attr is not None:
            sample = []
            n = min(5, safe_len(attr.data) if isinstance(safe_len(attr.data), int) else 0)
            for j in range(n):
                row = attr.data[j]
                vals = {}
                for field in ("vector", "value", "uv"):
                    if hasattr(row, field):
                        try:
                            v = getattr(row, field)
                            vals[field] = tuple(v) if hasattr(v, "__len__") else v
                        except Exception as exc:
                            vals[field] = f"ERR:{exc}"
                sample.append(vals)
            print(
                f"ATTRIBUTE_SAMPLE name={name!r} "
                f"domain={attr.domain!r} "
                f"data_type={attr.data_type!r} "
                f"sample={sample}"
            )


def main():
    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    dump("BEFORE_SYNC", obj)

    sync_attempted = False
    sync_result = None
    if obj.mode == "EDIT":
        sync_attempted = True
        try:
            sync_result = obj.update_from_editmode()
        except Exception as exc:
            sync_result = f"ERR:{type(exc).__name__}:{exc}"

    print(f"\neditmode_sync_attempted={sync_attempted}")
    print(f"editmode_sync_result={sync_result}")

    dump("AFTER_SYNC", obj)

    print("\nuv_storage_probe_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"uv_storage_probe_check=FAIL: {type(exc).__name__}: {exc}")
        raise

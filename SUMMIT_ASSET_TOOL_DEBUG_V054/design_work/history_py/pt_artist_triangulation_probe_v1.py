#!/usr/bin/env python3
"""
PT ARTIST TRIANGULATION PROBE V1

Read-only diagnosis for artist-authored MshC01.

Compares two non-destructive export triangulation paths:
  A) Mesh.calc_loop_triangles()
  B) bmesh.ops.triangulate() on a temporary BMesh copy

Reports:
  - artist polygon-size histogram
  - degenerate triangles from each method
  - source polygon IDs implicated by Mesh LoopTriangles
  - UV availability in temporary BMesh
  - min/max triangle areas

Does NOT save the .blend.
Does NOT write SMD/client files.
"""

import math
import bpy
import bmesh
from collections import Counter, defaultdict
from mathutils import Vector

TARGET = "MshC01"
AREA_EPS = 1e-10


def sync_object_mode(obj):
    original = obj.mode
    changed = False
    if obj.mode == "EDIT":
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        result = bpy.ops.object.mode_set(mode="OBJECT")
        if "FINISHED" not in result:
            raise RuntimeError(f"mode_set failed: {result}")
        changed = True
    obj.data.update()
    return original, changed


def tri_area_coords(a, b, c):
    return 0.5 * (b - a).cross(c - a).length


def mesh_looptri_stats(mesh):
    mesh.calc_loop_triangles()

    deg = []
    areas = []
    by_poly_size = Counter()
    implicated_polys = defaultdict(list)

    for ti, tri in enumerate(mesh.loop_triangles):
        ids = [int(v) for v in tri.vertices]
        coords = [Vector(mesh.vertices[v].co) for v in ids]
        area = tri_area_coords(*coords)
        areas.append(area)

        pi = int(tri.polygon_index)
        psize = len(mesh.polygons[pi].vertices)
        by_poly_size[psize] += 1

        if len(set(ids)) != 3 or area <= AREA_EPS:
            row = {
                "tri": ti,
                "polygon": pi,
                "polygon_size": psize,
                "verts": ids,
                "area": area,
            }
            deg.append(row)
            implicated_polys[pi].append(row)

    return {
        "triangles": len(mesh.loop_triangles),
        "degenerate": deg,
        "areas": areas,
        "triangles_by_source_polygon_size": by_poly_size,
        "implicated_polys": implicated_polys,
    }


def bmesh_stats(mesh):
    bm = bmesh.new()
    try:
        bm.from_mesh(mesh)
        bm.verts.ensure_lookup_table()
        bm.edges.ensure_lookup_table()
        bm.faces.ensure_lookup_table()

        uv_name = mesh.uv_layers.active.name if mesh.uv_layers.active else None
        uv_layer = bm.loops.layers.uv.get(uv_name) if uv_name else None

        before_faces = len(bm.faces)
        before_verts = len(bm.verts)

        result = bmesh.ops.triangulate(
            bm,
            faces=list(bm.faces),
            quad_method="BEAUTY",
            ngon_method="BEAUTY",
        )

        bm.verts.ensure_lookup_table()
        bm.edges.ensure_lookup_table()
        bm.faces.ensure_lookup_table()

        deg = []
        areas = []
        uv_nonfinite = 0

        for fi, face in enumerate(bm.faces):
            ids = [v.index for v in face.verts]
            coords = [Vector(v.co) for v in face.verts]
            if len(coords) != 3:
                raise RuntimeError(f"BMesh triangulate left non-triangle face {fi}")

            area = tri_area_coords(*coords)
            areas.append(area)
            if len(set(ids)) != 3 or area <= AREA_EPS:
                deg.append({
                    "face": fi,
                    "verts": ids,
                    "area": area,
                })

            if uv_layer is not None:
                for loop in face.loops:
                    uv = loop[uv_layer].uv
                    if not (math.isfinite(float(uv.x)) and math.isfinite(float(uv.y))):
                        uv_nonfinite += 1

        return {
            "before_vertices": before_verts,
            "after_vertices": len(bm.verts),
            "before_faces": before_faces,
            "after_faces": len(bm.faces),
            "uv_layer_found": uv_layer is not None,
            "uv_nonfinite": uv_nonfinite,
            "degenerate": deg,
            "areas": areas,
            "triangulate_result_keys": sorted(result.keys()),
        }
    finally:
        bm.free()


def main():
    obj = bpy.data.objects.get(TARGET)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("MshC01 mesh not found")

    original_mode, synchronized = sync_object_mode(obj)
    mesh = obj.data

    poly_hist = Counter(len(p.vertices) for p in mesh.polygons)
    print("PT_ARTIST_TRIANGULATION_PROBE_V1")
    print(f"original_object_mode={original_mode}")
    print(f"edit_mode_synchronized={synchronized}")
    print(f"vertices={len(mesh.vertices)}")
    print(f"artist_polygons={len(mesh.polygons)}")
    print(f"loops={len(mesh.loops)}")
    print(f"polygon_size_histogram={dict(sorted(poly_hist.items()))}")
    print(f"active_uv={mesh.uv_layers.active.name if mesh.uv_layers.active else None!r}")
    print(f"active_uv_data_len={len(mesh.uv_layers.active.data) if mesh.uv_layers.active else 0}")

    a = mesh_looptri_stats(mesh)
    print("\n[MESH_LOOP_TRIANGLES]")
    print(f"triangles={a['triangles']}")
    print(f"degenerate_count={len(a['degenerate'])}")
    print(f"triangles_by_source_polygon_size={dict(sorted(a['triangles_by_source_polygon_size'].items()))}")
    if a["areas"]:
        print(f"min_area={min(a['areas']):.12g}")
        print(f"max_area={max(a['areas']):.12g}")

    deg_poly_hist = Counter(
        row["polygon_size"] for row in a["degenerate"]
    )
    print(f"degenerate_by_source_polygon_size={dict(sorted(deg_poly_hist.items()))}")
    print(f"implicated_polygon_count={len(a['implicated_polys'])}")
    print(f"implicated_polygon_sample={list(a['implicated_polys'].keys())[:30]}")
    print(f"degenerate_sample={a['degenerate'][:20]}")

    b = bmesh_stats(mesh)
    print("\n[TEMP_BMESH_TRIANGULATE]")
    print(f"before_vertices={b['before_vertices']}")
    print(f"after_vertices={b['after_vertices']}")
    print(f"before_faces={b['before_faces']}")
    print(f"after_faces={b['after_faces']}")
    print(f"uv_layer_found={b['uv_layer_found']}")
    print(f"uv_nonfinite={b['uv_nonfinite']}")
    print(f"degenerate_count={len(b['degenerate'])}")
    if b["areas"]:
        print(f"min_area={min(b['areas']):.12g}")
        print(f"max_area={max(b['areas']):.12g}")
    print(f"degenerate_sample={b['degenerate'][:20]}")
    print(f"triangulate_result_keys={b['triangulate_result_keys']}")

    if len(a["degenerate"]) and not len(b["degenerate"]):
        print("\nDIAGNOSIS=BMESH_TRIANGULATION_AVOIDS_LOOPTRI_DEGENERATES")
    elif len(a["degenerate"]) and len(b["degenerate"]):
        print("\nDIAGNOSIS=ARTIST_GEOMETRY_CONTAINS_TRUE_DEGENERACY_OR_BOTH_TRIANGULATORS_FAIL")
    elif not len(a["degenerate"]):
        print("\nDIAGNOSIS=MESH_LOOPTRI_ALREADY_VALID")
    else:
        print("\nDIAGNOSIS=INCONCLUSIVE")

    print("artist_triangulation_probe_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"artist_triangulation_probe_check=FAIL: {type(exc).__name__}: {exc}")
        raise

#!/usr/bin/env python3
import bpy, json, pathlib, sys

argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
if len(argv) != 2:
    raise SystemExit("usage: -- <json node list> <out json>")

wanted = set(json.loads(argv[0]))
out = pathlib.Path(argv[1]).expanduser().resolve()

counts = {}
objects_seen = []

for obj in bpy.data.objects:
    if obj.type != "MESH" or obj.name not in wanted:
        continue
    objects_seen.append(obj.name)
    for p in obj.data.polygons:
        k = str(int(p.material_index))
        counts[k] = counts.get(k, 0) + 1

missing = sorted(wanted.difference(objects_seen))
if missing:
    raise RuntimeError(f"selected nodes missing in blend: {missing}")

out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps({
    "objects": sorted(objects_seen),
    "material_face_counts": counts,
}, indent=2), encoding="utf-8")

print("PT_SELECTED_MATERIAL_USAGE_PROBE_V1")
print(f"objects={sorted(objects_seen)}")
print(f"material_face_counts={counts}")
print(f"report={out}")
print("selected_material_usage_check=PASS")

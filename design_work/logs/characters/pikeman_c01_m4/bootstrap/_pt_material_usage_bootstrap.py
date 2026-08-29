
import bpy, json, pathlib, sys

argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
out = pathlib.Path(argv[0]).expanduser().resolve()

usage = {}
for obj in bpy.data.objects:
    if obj.type != "MESH":
        continue
    counts = {}
    for poly in obj.data.polygons:
        idx = int(poly.material_index)
        counts[str(idx)] = counts.get(str(idx), 0) + 1
    usage[obj.name] = counts

out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(usage, indent=2), encoding="utf-8")
print("PT_MATERIAL_USAGE_BOOTSTRAP")
print(f"objects={len(usage)}")
print(f"report={out}")
print("material_usage_bootstrap_check=PASS")

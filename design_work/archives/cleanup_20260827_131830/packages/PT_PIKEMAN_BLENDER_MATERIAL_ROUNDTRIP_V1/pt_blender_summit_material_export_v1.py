#!/usr/bin/env python3
import bpy, pathlib, json, sys, re

def mat_id_from_name(name):
    m = re.match(r"^SMD_MAT_(\d+)_", name)
    return int(m.group(1)) if m else None

def main():
    argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv) != 1:
        raise SystemExit("usage: -- <out.json>")
    out = pathlib.Path(argv[0]).expanduser().resolve()

    rows=[]
    for bm in bpy.data.materials:
        mid = bm.get("pt_material_id", mat_id_from_name(bm.name))
        if mid is None:
            continue
        if bm.get("pt_material_schema") != "PT_SMD_MATERIAL_V1":
            raise RuntimeError(f"{bm.name}: missing/invalid pt_material_schema")
        rows.append({
            "material_id": int(mid),
            "material_name": bm.name,
            "self_illum": float(bm.get("pt_self_illum", 0.0)),
            "editable_fields": ["self_illum"],
        })
    rows.sort(key=lambda x: x["material_id"])
    if not rows:
        raise RuntimeError("no Summit materials found")

    obj={
        "tool":"PT_BLENDER_SUMMIT_MATERIAL_EXPORT_V1",
        "schema":"PT_SMD_MATERIAL_EDIT_MANIFEST_V1",
        "materials":rows,
        "editable_fields":["self_illum"],
        "export_check":"PASS",
    }
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    print("PT_BLENDER_SUMMIT_MATERIAL_EXPORT_V1")
    print(f"material_count={len(rows)}")
    for r in rows:
        print(f"MATERIAL {r['material_id']} self_illum={r['self_illum']}")
    print(f"manifest={out}")
    print("export_check=PASS")

if __name__ == "__main__":
    main()

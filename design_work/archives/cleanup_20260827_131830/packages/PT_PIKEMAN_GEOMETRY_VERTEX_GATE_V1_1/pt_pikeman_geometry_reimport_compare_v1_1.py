#!/usr/bin/env python3
import bpy, pathlib, sys, math, json

TARGET="TbmC03"
EXPECTED_VERTS=74
MAX_ALLOWED_ERROR=0.01

def main():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if len(argv)!=2:
        raise SystemExit("usage: -- <reimport.blend> <report.json>")
    reimport=pathlib.Path(argv[0]).expanduser().resolve()
    report=pathlib.Path(argv[1]).expanduser().resolve()

    artist=bpy.data.objects.get(TARGET)
    if artist is None or artist.type!="MESH":
        raise RuntimeError("artist target missing")
    if len(artist.data.vertices)!=EXPECTED_VERTS:
        raise RuntimeError("artist vertex count mismatch")

    with bpy.data.libraries.load(str(reimport), link=False) as (src,dst):
        if TARGET not in src.objects:
            raise RuntimeError(f"{TARGET} missing from reimport blend")
        dst.objects=[TARGET]
    imported=dst.objects[0]
    if imported is None or imported.type!="MESH":
        raise RuntimeError("failed to append reimported target")
    if len(imported.data.vertices)!=EXPECTED_VERTS:
        raise RuntimeError("reimport vertex count mismatch")

    a=[artist.matrix_world @ v.co for v in artist.data.vertices]
    b=[imported.matrix_world @ v.co for v in imported.data.vertices]
    errs=[(a[i]-b[i]).length for i in range(EXPECTED_VERTS)]
    maxe=max(errs); meane=sum(errs)/len(errs)
    worst=max(range(len(errs)),key=lambda i:errs[i])

    obj={
        "tool":"PT_PIKEMAN_GEOMETRY_REIMPORT_COMPARE_V1_1",
        "target_object":TARGET,
        "vertex_count":EXPECTED_VERTS,
        "max_vertex_position_error":maxe,
        "mean_vertex_position_error":meane,
        "worst_vertex":worst,
        "allowed_max_error":MAX_ALLOWED_ERROR,
        "geometry_reimport_compare_check":"PASS" if maxe<=MAX_ALLOWED_ERROR else "FAIL",
    }
    report.parent.mkdir(parents=True,exist_ok=True)
    report.write_text(json.dumps(obj,indent=2)+"\n",encoding="utf-8")

    print("PT_PIKEMAN_GEOMETRY_REIMPORT_COMPARE_V1_1")
    print(f"object={TARGET}")
    print(f"vertices={EXPECTED_VERTS}")
    print(f"max_vertex_position_error={maxe:.12g}")
    print(f"mean_vertex_position_error={meane:.12g}")
    print(f"worst_vertex={worst}")
    print(f"allowed_max_error={MAX_ALLOWED_ERROR}")
    print(f"report={report}")
    if maxe>MAX_ALLOWED_ERROR:
        print("geometry_reimport_compare_check=FAIL")
        raise RuntimeError(f"reimport does not match artist geometry: max_error={maxe}")
    print("geometry_reimport_compare_check=PASS")

if __name__=="__main__":
    main()

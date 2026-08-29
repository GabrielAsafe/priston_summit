#!/usr/bin/env python3
from __future__ import annotations
import argparse, pathlib, hashlib, json, struct, os, tempfile, re

SOURCE_SHA="9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SOURCE_SIZE=187504
MAT_GROUP_SIZE=88
MATERIAL_SIZE=320

def sha(data): return hashlib.sha256(data).hexdigest()
def i32(d,o): return struct.unpack_from("<i",d,o)[0]
def f32(d,o): return struct.unpack_from("<f",d,o)[0]

def atomic_write(path,data):
    path.parent.mkdir(parents=True,exist_ok=True)
    fd,tmp=tempfile.mkstemp(prefix=path.name+".",suffix=".tmp",dir=path.parent)
    os.close(fd); t=pathlib.Path(tmp)
    try:
        t.write_bytes(data); os.replace(t,path)
    finally:
        if t.exists(): t.unlink()

def atomic_json(path,obj):
    path.parent.mkdir(parents=True,exist_ok=True)
    fd,tmp=tempfile.mkstemp(prefix=path.name+".",suffix=".tmp",dir=path.parent)
    os.close(fd); t=pathlib.Path(tmp)
    try:
        t.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
        os.replace(t,path)
    finally:
        if t.exists(): t.unlink()

def parse_offsets(data):
    if not data.startswith(b"SMD Model data Ver 0.62"):
        raise ValueError("unexpected SMD header")
    mc=i32(data,28); mp=i32(data,32); fp=i32(data,36)
    if i32(data,mp+8)!=mc: raise ValueError("MaterialCount mismatch")
    cursor=mp+MAT_GROUP_SIZE
    rows=[]
    for mid in range(mc):
        base=cursor
        tc=i32(data,base+4); ac=i32(data,base+304); inuse=i32(data,base)
        if not 0<=tc<=8 or not 0<=ac<=32: raise ValueError("bad counters")
        rows.append({"id":mid,"base":base,"selfillum_off":base+148,"selfillum":f32(data,base+148)})
        cursor += MATERIAL_SIZE
        if inuse:
            sl=i32(data,cursor); cursor += 4+sl
    if cursor != fp: raise ValueError("material region end mismatch")
    return rows

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--source",default="TmbC01.smd")
    ap.add_argument("--edit-manifest",required=True)
    ap.add_argument("--out",default="pt_build/characters/pikeman_c01_m4/materials/TmbC01_from_blender_material_v1.smd")
    ap.add_argument("--manifest",default="logs/characters/pikeman_c01_m4/materials/TmbC01_from_blender_material_v1_build_manifest.json")
    args=ap.parse_args()

    src=pathlib.Path(args.source).expanduser().resolve()
    raw=src.read_bytes()
    if sha(raw)!=SOURCE_SHA or len(raw)!=SOURCE_SIZE:
        raise ValueError("registered Pikeman TmbC01.smd source baseline mismatch")

    edit=json.loads(pathlib.Path(args.edit_manifest).expanduser().resolve().read_text(encoding="utf-8"))
    if edit.get("schema")!="PT_SMD_MATERIAL_EDIT_MANIFEST_V1":
        raise ValueError("unexpected edit manifest schema")
    if edit.get("editable_fields")!=["self_illum"]:
        raise ValueError("V1 only permits self_illum")

    rows=parse_offsets(raw)
    byid={r["id"]:r for r in rows}
    out=bytearray(raw)
    requested=[]
    for m in edit["materials"]:
        mid=int(m["material_id"])
        if mid not in byid: raise ValueError(f"unknown material id {mid}")
        val=float(m["self_illum"])
        off=byid[mid]["selfillum_off"]
        old=struct.unpack_from("<f",raw,off)[0]
        struct.pack_into("<f",out,off,val)
        requested.append({"material_id":mid,"old_self_illum":old,"new_self_illum":val,"offset":off})

    changed=[i for i,(a,b) in enumerate(zip(raw,out)) if a!=b]
    allowed=set()
    for r in requested:
        allowed.update(range(r["offset"],r["offset"]+4))
    outside=[i for i in changed if i not in allowed]
    if outside:
        raise RuntimeError(f"bytes changed outside allowed SelfIllum fields: {outside[:20]}")
    if len(out)!=len(raw):
        raise RuntimeError("size changed")

    # Semantic reparse of output.
    after=parse_offsets(bytes(out))
    # No unrelated selfillum changes among materials not in request.
    requested_ids={r["material_id"] for r in requested}
    for a,b in zip(rows,after):
        if a["id"] not in requested_ids and a["selfillum"] != b["selfillum"]:
            raise RuntimeError(f"unrequested material {a['id']} SelfIllum changed")

    outp=pathlib.Path(args.out).expanduser()
    if not outp.is_absolute(): outp=pathlib.Path.cwd()/outp
    outp=outp.resolve()
    atomic_write(outp,bytes(out))
    outsha=sha(bytes(out))

    mp=pathlib.Path(args.manifest).expanduser()
    if not mp.is_absolute(): mp=pathlib.Path.cwd()/mp
    mp=mp.resolve()
    obj={
        "tool":"PT_SMD_MATERIAL_BUILD_FROM_BLENDER_V1",
        "roundtrip_target":"char/tmABCD/TmbC01.smd",
        "source":str(src),
        "source_sha256":SOURCE_SHA,
        "source_size":SOURCE_SIZE,
        "artifact":str(outp),
        "artifact_sha256":outsha,
        "artifact_size":len(out),
        "requested_changes":requested,
        "changed_byte_count":len(changed),
        "changed_offsets":changed,
        "outside_allowed_field_changed_byte_count":len(outside),
        "file_size_preserved":True,
        "runtime_installed":False,
        "build_check":"PASS",
    }
    atomic_json(mp,obj)

    print("PT_SMD_MATERIAL_BUILD_FROM_BLENDER_V1")
    for r in requested:
        print(f"MATERIAL {r['material_id']} SelfIllum {r['old_self_illum']} -> {r['new_self_illum']} offset={r['offset']}")
    print(f"changed_byte_count={len(changed)}")
    print(f"outside_allowed_field_changed_byte_count={len(outside)}")
    print("file_size_preserved=True")
    print(f"artifact={outp}")
    print(f"artifact_sha256={outsha}")
    print(f"manifest={mp}")
    print("runtime_installed=False")
    print("build_check=PASS")

if __name__=="__main__":
    try:
        main()
    except Exception as exc:
        print(f"build_check=FAIL: {type(exc).__name__}: {exc}")
        raise

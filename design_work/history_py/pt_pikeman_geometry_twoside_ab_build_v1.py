#!/usr/bin/env python3
from __future__ import annotations
import argparse, pathlib, struct, hashlib, json, os, tempfile

BASE_SHA = "40b02371a872745d98abc558042b62f75cfb001a297e1bc36582e6c5968495a5"
BASE_SIZE = 187504
TARGET = "char/tmABCD/TmbC01.smd"
MAT_GROUP_SIZE = 88
MATERIAL_SIZE = 320

def sha(data): return hashlib.sha256(data).hexdigest()
def i32(data, off): return struct.unpack_from("<i", data, off)[0]

def atomic_write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd,tmp=tempfile.mkstemp(prefix=path.name+".", suffix=".tmp", dir=path.parent)
    os.close(fd); t=pathlib.Path(tmp)
    try:
        t.write_bytes(data); os.replace(t, path)
    finally:
        if t.exists(): t.unlink()

def atomic_json(path, obj):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd,tmp=tempfile.mkstemp(prefix=path.name+".", suffix=".tmp", dir=path.parent)
    os.close(fd); t=pathlib.Path(tmp)
    try:
        t.write_text(json.dumps(obj, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
        os.replace(t, path)
    finally:
        if t.exists(): t.unlink()

def material_bases(data):
    if not data.startswith(b"SMD Model data Ver 0.62"):
        raise RuntimeError("unexpected SMD header")
    mc=i32(data,28); mp=i32(data,32); fp=i32(data,36)
    if mc != 2:
        raise RuntimeError(f"expected 2 materials, got {mc}")
    if i32(data,mp+8) != mc:
        raise RuntimeError("smMATERIAL_GROUP MaterialCount mismatch")
    cursor=mp+MAT_GROUP_SIZE
    bases=[]
    for mid in range(mc):
        bases.append(cursor)
        inuse=i32(data,cursor)
        tc=i32(data,cursor+4)
        ac=i32(data,cursor+304)
        if not 0 <= tc <= 8 or not 0 <= ac <= 32:
            raise RuntimeError("invalid material counters")
        cursor += MATERIAL_SIZE
        if inuse:
            sl=i32(data,cursor)
            if sl < 0 or cursor+4+sl > fp:
                raise RuntimeError("invalid material string block")
            cursor += 4+sl
    if cursor != fp:
        raise RuntimeError("material region end mismatch")
    return bases

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--base", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--manifest", required=True)
    args=ap.parse_args()

    base=pathlib.Path(args.base).expanduser().resolve()
    raw=base.read_bytes()
    if len(raw)!=BASE_SIZE or sha(raw)!=BASE_SHA:
        raise RuntimeError("base must be the exact approved Pike geometry V1.1 artifact")

    bases=material_bases(raw)
    m0=bases[0]
    m1=bases[1]
    off0=m0+124
    off1=m1+124

    old0=i32(raw,off0)
    old1=i32(raw,off1)
    if old0 != 0 or old1 != 0:
        raise RuntimeError(f"unexpected TwoSide baseline m0={old0} m1={old1}")

    out=bytearray(raw)
    struct.pack_into("<I", out, off0, 1)

    changed=[i for i,(a,b) in enumerate(zip(raw,out)) if a!=b]
    allowed=set(range(off0,off0+4))
    outside=[i for i in changed if i not in allowed]
    if outside:
        raise RuntimeError(f"changed bytes outside material0 TwoSide: {outside[:20]}")
    if len(changed) != 1:
        raise RuntimeError(f"expected exactly one changed byte, got {len(changed)}")
    if len(out)!=len(raw):
        raise RuntimeError("file size changed")

    # Independent post-patch parse.
    bases2=material_bases(bytes(out))
    new0=i32(out,bases2[0]+124)
    new1=i32(out,bases2[1]+124)
    if new0 != 1 or new1 != 0:
        raise RuntimeError("post-patch TwoSide verification failed")

    outp=pathlib.Path(args.out).expanduser()
    if not outp.is_absolute(): outp=(pathlib.Path.cwd()/outp)
    outp=outp.resolve()
    atomic_write(outp,bytes(out))
    outsha=sha(bytes(out))

    mp=pathlib.Path(args.manifest).expanduser()
    if not mp.is_absolute(): mp=(pathlib.Path.cwd()/mp)
    mp=mp.resolve()
    obj={
        "tool":"PT_PIKEMAN_GEOMETRY_TWOSIDE_AB_BUILD_V1",
        "roundtrip_target":TARGET,
        "source":str(base),
        "source_sha256":BASE_SHA,
        "source_size":BASE_SIZE,
        "artifact":str(outp),
        "artifact_sha256":outsha,
        "artifact_size":len(out),
        "material_id":0,
        "field":"TwoSide",
        "field_offset":off0,
        "old_value":old0,
        "new_value":new0,
        "control_material1_value":new1,
        "changed_byte_count":len(changed),
        "changed_offsets":changed,
        "outside_allowed_field_changed_byte_count":len(outside),
        "geometry_bytes_preserved_from_approved_base":True,
        "file_size_preserved":True,
        "runtime_installed":False,
        "build_check":"PASS",
    }
    atomic_json(mp,obj)

    print("PT_PIKEMAN_GEOMETRY_TWOSIDE_AB_BUILD_V1")
    print(f"base_sha256={BASE_SHA}")
    print("material0.TwoSide=0 -> 1")
    print("material1.TwoSide=0 -> 0")
    print(f"changed_byte_count={len(changed)}")
    print(f"outside_allowed_field_changed_byte_count={len(outside)}")
    print("geometry_bytes_preserved_from_approved_base=True")
    print("file_size_preserved=True")
    print(f"artifact={outp}")
    print(f"artifact_sha256={outsha}")
    print(f"manifest={mp}")
    print("runtime_installed=False")
    print("build_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as exc:
        print(f"build_check=FAIL: {type(exc).__name__}: {exc}")
        raise

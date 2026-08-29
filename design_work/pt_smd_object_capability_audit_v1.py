#!/usr/bin/env python3
"""
PT SMD Model 0.62 Object Capability Audit V1

Read-only audit that determines which source object blocks satisfy the current
generic FREE MESH V3 writer assumptions.

No Blender and no client modification required.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import pathlib
import struct
from collections import defaultdict

HEADER = b"SMD Model data Ver 0.62"
SZ_HDR = 556
SZ_INFO = 40
SZ_OBJ = 2236
SZ_VERT = 24
SZ_FACE = 36
SZ_TEX = 32
SZ_ROT = 20
SZ_POS = 16
SZ_SCALE = 16
SZ_MATRIX = 64

O_PHYS_PTR = 16
O_NV = 84
O_NF = 88
O_NT = 92
O_NAME = 172
O_TM = 240
O_NR = 684
O_NP = 688
O_NS = 692
F_TEX_PTR = 32
T_HTEX = 24
T_NEXT = 28


def i32(b, o): return struct.unpack_from("<i", b, o)[0]
def u32(b, o): return struct.unpack_from("<I", b, o)[0]
def cstr(b): return b.split(b"\0", 1)[0].decode("latin-1", errors="replace")
def sha256_bytes(b): return hashlib.sha256(b).hexdigest()


def parse_file(data):
    if len(data) < SZ_HDR or not data.startswith(HEADER):
        raise ValueError("not an SMD Model data Ver 0.62 file")
    n = i32(data, 24)
    if not 1 <= n <= 128:
        raise ValueError(f"implausible object count {n}")
    infos = []
    for idx in range(n):
        off = SZ_HDR + idx * SZ_INFO
        raw = data[off:off + SZ_INFO]
        if len(raw) != SZ_INFO:
            raise ValueError("truncated object info table")
        infos.append({
            "index": idx,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })
    for i, inf in enumerate(infos):
        end = inf["start"] + inf["length"]
        expected = infos[i+1]["start"] if i+1 < len(infos) else len(data)
        if end != expected:
            raise ValueError(f"{inf['name']}: non-contiguous/invalid block")
    return infos


def layout(start, nv, nf, nt, nr, np, ns, phys):
    v = start + SZ_OBJ
    f = v + nv * SZ_VERT
    t = f + nf * SZ_FACE
    r = t + nt * SZ_TEX
    p = r + nr * SZ_ROT
    s = p + np * SZ_POS
    prev = s + ns * SZ_SCALE
    phy = prev + nr * SZ_MATRIX if phys else None
    end = phy + nv * 32 if phys else prev + nr * SZ_MATRIX
    return {"v":v,"f":f,"t":t,"r":r,"p":p,"s":s,"prev":prev,"phy":phy,"end":end}


def parse_skeleton(path):
    data = path.read_bytes()
    infos = parse_file(data)
    names = set()
    for inf in infos:
        raw = data[inf["start"]:inf["start"]+SZ_OBJ]
        name = cstr(raw[O_NAME:O_NAME+32])
        if name:
            names.add(name)
    return names


def audit_object(data, inf, skeleton_names=None):
    raw = data[inf["start"]:inf["start"]+SZ_OBJ]
    node = cstr(raw[O_NAME:O_NAME+32])
    nv, nf, nt = i32(raw,O_NV), i32(raw,O_NF), i32(raw,O_NT)
    nr, np, ns = i32(raw,O_NR), i32(raw,O_NP), i32(raw,O_NS)
    phys = u32(raw,O_PHYS_PTR) != 0
    lay = layout(inf["start"], nv,nf,nt,nr,np,ns,phys)

    reasons = []
    warnings = []
    if node != inf["name"]:
        reasons.append("NODE_NAME_MISMATCH")
    if lay["end"] != inf["start"] + inf["length"]:
        reasons.append("SERIALIZED_LENGTH_MISMATCH")
    if not phys:
        reasons.append("NO_PHYSIQUE")
    if nr or np or ns:
        reasons.append(f"OBJECT_LOCAL_ANIMATION_ARRAYS:{nr}/{np}/{ns}")
    if not (3 <= nv <= 30000):
        reasons.append(f"VERTEX_COUNT_OUTSIDE_SAFE_RANGE:{nv}")
    if not (1 <= nf <= 30000):
        reasons.append(f"FACE_COUNT_OUTSIDE_SAFE_RANGE:{nf}")
    if nv > 65535:
        reasons.append("UINT16_VERTEX_INDEX_OVERFLOW")

    texlinks = []
    for ti in range(max(0, nt)):
        off = lay["t"] + ti * SZ_TEX
        if off + SZ_TEX > len(data):
            reasons.append("TRUNCATED_TEXLINK_ARRAY")
            break
        texlinks.append({
            "hTexture": u32(data, off+T_HTEX),
            "NextTex": u32(data, off+T_NEXT),
        })

    mats = set()
    hsets = defaultdict(set)
    face_tex_indices = []
    bad_faces = []
    texbase = u32(raw, 12)

    for fi in range(max(0, nf)):
        off = lay["f"] + fi * SZ_FACE
        if off + SZ_FACE > len(data):
            reasons.append("TRUNCATED_FACE_ARRAY")
            break
        a,b,c,mat = struct.unpack_from("<4H", data, off)
        mats.add(mat)
        if any(v >= nv for v in (a,b,c)):
            bad_faces.append(fi)
        ptr = u32(data, off+F_TEX_PTR)
        delta = ptr - texbase
        if delta < 0 or delta % SZ_TEX:
            reasons.append(f"BAD_TEXLINK_POINTER_FACE:{fi}")
            continue
        ti = delta // SZ_TEX
        if not 0 <= ti < nt:
            reasons.append(f"TEXLINK_OOB_FACE:{fi}")
            continue
        face_tex_indices.append(ti)
        hsets[mat].add(texlinks[ti]["hTexture"])
        if texlinks[ti]["NextTex"] != 0:
            reasons.append(f"MULTILAYER_TEXLINK_FACE:{fi}")

    if bad_faces:
        reasons.append(f"FACE_VERTEX_OOB_COUNT:{len(bad_faces)}")
    for mat, hs in hsets.items():
        if len(hs) != 1:
            reasons.append(f"MATERIAL_MULTIPLE_HTEXTURE:{mat}")

    bones = []
    missing_bones = []
    if phys and lay["phy"] is not None:
        for vi in range(max(0, nv)):
            off = lay["phy"] + vi*32
            if off + 32 > len(data):
                reasons.append("TRUNCATED_PHYSIQUE_ARRAY")
                break
            bone = cstr(data[off:off+32])
            bones.append(bone)
            if not bone:
                reasons.append(f"EMPTY_BONE_NAME_VERTEX:{vi}")
            elif skeleton_names is not None and bone not in skeleton_names:
                missing_bones.append(bone)
    if missing_bones:
        reasons.append(f"BONES_MISSING_FROM_SKELETON:{len(set(missing_bones))}")

    normal_mags = []
    for vi in range(max(0, nv)):
        off = lay["v"] + vi*SZ_VERT
        if off + SZ_VERT > len(data):
            reasons.append("TRUNCATED_VERTEX_ARRAY")
            break
        _x,_y,_z,nx,ny,nz = struct.unpack_from("<6i", data, off)
        mag = math.sqrt(nx*nx + ny*ny + nz*nz)
        if mag > 0:
            normal_mags.append(mag)
    if not normal_mags:
        reasons.append("NO_NONZERO_SOURCE_NORMALS")

    # De-duplicate while keeping order.
    reasons = list(dict.fromkeys(reasons))
    supported = not reasons

    return {
        "index": inf["index"],
        "name": inf["name"],
        "vertices": nv,
        "faces": nf,
        "texlinks": nt,
        "physique": phys,
        "rot_keys": nr,
        "pos_keys": np,
        "scale_keys": ns,
        "materials": sorted(mats),
        "rigid_bones": sorted(set(bones)),
        "rigid_bone_count": len(set(bones)),
        "referenced_texlinks": len(set(face_tex_indices)),
        "source_normal_samples": len(normal_mags),
        "free_mesh_v3_supported": supported,
        "blockers": reasons,
        "warnings": warnings,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--smd", required=True, type=pathlib.Path)
    ap.add_argument("--skeleton", type=pathlib.Path)
    ap.add_argument("--expected-source-sha256")
    ap.add_argument("--report")
    args = ap.parse_args()

    smd = args.smd.expanduser().resolve()
    data = smd.read_bytes()
    source_sha = sha256_bytes(data)
    if args.expected_source_sha256 and source_sha != args.expected_source_sha256.lower():
        raise ValueError(
            f"source SHA mismatch: {source_sha} != {args.expected_source_sha256.lower()}"
        )

    infos = parse_file(data)
    skeleton_names = None
    if args.skeleton:
        skeleton_names = parse_skeleton(args.skeleton.expanduser().resolve())

    rows = [audit_object(data, inf, skeleton_names) for inf in infos]
    supported = [r["name"] for r in rows if r["free_mesh_v3_supported"]]
    blocked = [r["name"] for r in rows if not r["free_mesh_v3_supported"]]

    report = {
        "tool": "PT_SMD_MODEL_062_OBJECT_CAPABILITY_AUDIT_V1",
        "source_smd": str(smd),
        "source_sha256": source_sha,
        "objects": len(rows),
        "free_mesh_v3_supported_count": len(supported),
        "free_mesh_v3_blocked_count": len(blocked),
        "supported_objects": supported,
        "blocked_objects": blocked,
        "object_rows": rows,
        "audit_check": "PASS",
    }

    if args.report:
        rp = pathlib.Path(args.report).expanduser().resolve()
        rp.parent.mkdir(parents=True, exist_ok=True)
        rp.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    else:
        rp = None

    print("PT_SMD_MODEL_062_OBJECT_CAPABILITY_AUDIT_V1")
    print(f"source_sha256={source_sha}")
    print(f"objects={len(rows)}")
    for r in rows:
        status = "SUPPORTED" if r["free_mesh_v3_supported"] else "BLOCKED"
        reason = ",".join(r["blockers"]) if r["blockers"] else "-"
        print(
            f"OBJECT {r['name']}: "
            f"V={r['vertices']} F={r['faces']} T={r['texlinks']} "
            f"bones={r['rigid_bone_count']} mats={r['materials']} "
            f"status={status} blockers={reason}"
        )
    print(f"supported_count={len(supported)}")
    print(f"blocked_count={len(blocked)}")
    print(f"supported_objects={supported}")
    print(f"blocked_objects={blocked}")
    if rp:
        print(f"report={rp}")
    print("object_capability_audit_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"object_capability_audit_check=FAIL: {type(exc).__name__}: {exc}")
        raise

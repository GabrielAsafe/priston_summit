#!/usr/bin/env python3
import argparse
import json
import struct
import sys
from collections import Counter
from pathlib import Path

SMD_HEADER_TEXT = b"SMD Model data Ver 0.62"

SIZE_DFILE_HEADER = 556
SIZE_DFILE_OBJINFO = 40
SIZE_OBJ3D = 2236
SIZE_VERTEX = 24
SIZE_FACE = 36
SIZE_TEXLINK = 32
SIZE_TM_ROT = 20
SIZE_TM_POS = 16
SIZE_TM_SCALE = 16
SIZE_MATRIX = 64

OFF_HEAD = 0
OFF_VERTEX_PTR = 4
OFF_FACE_PTR = 8
OFF_TEXLINK_PTR = 12
OFF_PHYSIQUE_PTR = 16

OFF_NVERTEX = 84
OFF_NFACE = 88
OFF_NTEXLINK = 92

OFF_NODENAME = 172
OFF_NODEPARENT = 204

OFF_TMROTCNT = 684
OFF_TMPOSCNT = 688
OFF_TMSCALECNT = 692


def u16(data, off):
    return struct.unpack_from("<H", data, off)[0]


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def f32(data, off):
    return struct.unpack_from("<f", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def ptr_to_tex_index(ptr, old_base, count):
    if ptr == 0:
        return None, "NULL"
    if old_base == 0:
        return None, "base_zero"
    delta = (ptr - old_base) & 0xFFFFFFFF
    # Valid pointers in the serialized object must point forward inside
    # the original contiguous smTEXLINK array.
    if ptr < old_base:
        return None, "before_base"
    if delta % SIZE_TEXLINK:
        return None, f"unaligned_delta_{delta}"
    idx = delta // SIZE_TEXLINK
    if idx >= count:
        return None, f"index_oob_{idx}"
    return idx, "OK"


def object_layout(obj_start, nv, nf, nt, nr, np, ns, has_physique):
    vertices_start = obj_start + SIZE_OBJ3D
    faces_start = vertices_start + SIZE_VERTEX * nv
    texlinks_start = faces_start + SIZE_FACE * nf
    tmrot_start = texlinks_start + SIZE_TEXLINK * nt
    tmpos_start = tmrot_start + SIZE_TM_ROT * nr
    tmscale_start = tmpos_start + SIZE_TM_POS * np
    prevrot_start = tmscale_start + SIZE_TM_SCALE * ns
    physique_start = prevrot_start + SIZE_MATRIX * nr if has_physique else None
    end = (
        physique_start + 32 * nv
        if has_physique
        else prevrot_start + SIZE_MATRIX * nr
    )
    return {
        "vertices": vertices_start,
        "faces": faces_start,
        "texlinks": texlinks_start,
        "tmrot": tmrot_start,
        "tmpos": tmpos_start,
        "tmscale": tmscale_start,
        "prevrot": prevrot_start,
        "physique": physique_start,
        "end": end,
    }


def read_texlink(data, off):
    u = struct.unpack_from("<3f", data, off)
    v = struct.unpack_from("<3f", data, off + 12)
    htexture = u32(data, off + 24)
    next_ptr = u32(data, off + 28)
    return {
        "u": u,
        "v": v,
        "hTexture": htexture,
        "NextTex": next_ptr,
    }


def main():
    ap = argparse.ArgumentParser(
        description="Read-only SMD v0.62 topology/UV/rigid-physique probe"
    )
    ap.add_argument("smd", type=Path)
    ap.add_argument("--samples", type=int, default=3)
    ap.add_argument("--json", dest="json_path", type=Path)
    args = ap.parse_args()

    data = args.smd.read_bytes()
    file_size = len(data)

    print("PT_SMD_PROBE_V2")
    print(f"file={args.smd}")
    print(f"file_size={file_size}")

    if file_size < SIZE_DFILE_HEADER:
        print("FAIL file too small")
        return 2

    header = data[:SIZE_DFILE_HEADER]
    if not header.startswith(SMD_HEADER_TEXT):
        print(f"FAIL header={cstr(header[:24])!r}")
        return 3

    obj_counter = i32(header, 24)
    mat_counter = i32(header, 28)
    mat_file_point = i32(header, 32)
    first_obj_point = i32(header, 36)

    print("header_check=OK")
    print(f"ObjCounter={obj_counter}")
    print(f"MatCounter={mat_counter}")
    print(f"MatFilePoint={mat_file_point}")
    print(f"First_ObjInfoPoint={first_obj_point}")

    table_start = SIZE_DFILE_HEADER
    table_end = table_start + obj_counter * SIZE_DFILE_OBJINFO

    infos = []
    for i in range(obj_counter):
        off = table_start + i * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        infos.append({
            "index": i,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })

    report = {
        "file": str(args.smd),
        "file_size": file_size,
        "ObjCounter": obj_counter,
        "MatCounter": mat_counter,
        "objects": [],
    }

    total_vertices = 0
    total_faces = 0
    total_texlinks = 0
    total_physique = 0
    all_ok = True

    print()
    print("OBJECT_EXTRACTION")

    for info in infos:
        i = info["index"]
        obj_start = info["start"]
        blob = data[obj_start:obj_start + SIZE_OBJ3D]

        if len(blob) != SIZE_OBJ3D:
            print(f"[{i:02d}] FAIL object header outside file")
            all_ok = False
            continue

        old_tex_base = u32(blob, OFF_TEXLINK_PTR)
        physique_ptr = u32(blob, OFF_PHYSIQUE_PTR)

        nv = i32(blob, OFF_NVERTEX)
        nf = i32(blob, OFF_NFACE)
        nt = i32(blob, OFF_NTEXLINK)
        nr = i32(blob, OFF_TMROTCNT)
        np = i32(blob, OFF_TMPOSCNT)
        ns = i32(blob, OFF_TMSCALECNT)

        node_name = cstr(blob[OFF_NODENAME:OFF_NODENAME + 32])
        node_parent = cstr(blob[OFF_NODEPARENT:OFF_NODEPARENT + 32])

        has_physique = physique_ptr != 0
        layout = object_layout(obj_start, nv, nf, nt, nr, np, ns, has_physique)

        obj_ok = True
        invalid_vertex_faces = 0
        invalid_material_faces = 0
        invalid_face_tex_ptrs = 0
        invalid_next_ptrs = 0
        looped_chains = 0
        null_face_tex = 0
        max_layers = 0
        chain_hist = Counter()
        material_hist = Counter()
        referenced_texlinks = Counter()

        texlinks = [
            read_texlink(data, layout["texlinks"] + t * SIZE_TEXLINK)
            for t in range(nt)
        ]

        faces = []
        for f in range(nf):
            off = layout["faces"] + f * SIZE_FACE
            v0, v1, v2, mat = struct.unpack_from("<4H", data, off)
            face_ptr = u32(data, off + 32)

            if v0 >= nv or v1 >= nv or v2 >= nv:
                invalid_vertex_faces += 1
                obj_ok = False

            if mat >= mat_counter:
                invalid_material_faces += 1
                obj_ok = False

            material_hist[mat] += 1

            first_idx, status = ptr_to_tex_index(face_ptr, old_tex_base, nt)
            chain = []

            if first_idx is None:
                if status == "NULL":
                    null_face_tex += 1
                else:
                    invalid_face_tex_ptrs += 1
                    obj_ok = False
            else:
                idx = first_idx
                seen = set()
                while idx is not None:
                    if idx in seen:
                        looped_chains += 1
                        obj_ok = False
                        break
                    seen.add(idx)
                    referenced_texlinks[idx] += 1
                    tl = texlinks[idx]
                    chain.append({
                        "index": idx,
                        "u": list(tl["u"]),
                        "v": list(tl["v"]),
                        "hTexture": tl["hTexture"],
                    })
                    next_idx, nstatus = ptr_to_tex_index(
                        tl["NextTex"], old_tex_base, nt
                    )
                    if tl["NextTex"] == 0:
                        idx = None
                    elif next_idx is None:
                        invalid_next_ptrs += 1
                        obj_ok = False
                        break
                    else:
                        idx = next_idx

                    # Source AddTexLink searches up to 8 links. Keep a hard
                    # corruption guard much larger than that.
                    if len(chain) > 64:
                        looped_chains += 1
                        obj_ok = False
                        break

            chain_hist[len(chain)] += 1
            max_layers = max(max_layers, len(chain))

            faces.append({
                "vertices": [v0, v1, v2],
                "material": mat,
                "texlink_chain": chain,
            })

        bones = []
        bone_hist = Counter()
        if has_physique:
            total_physique += nv
            poff = layout["physique"]
            for v in range(nv):
                name = cstr(data[poff + v * 32: poff + (v + 1) * 32])
                bones.append(name)
                bone_hist[name] += 1

        unreferenced_texlinks = nt - len(referenced_texlinks)
        multi_referenced = sum(1 for n in referenced_texlinks.values() if n > 1)

        if layout["end"] != obj_start + info["length"]:
            obj_ok = False

        total_vertices += nv
        total_faces += nf
        total_texlinks += nt
        all_ok &= obj_ok

        print()
        print(f"[{i:02d}] {info['name']!r}")
        print(
            f"  node={node_name!r} parent={node_parent!r} "
            f"V={nv} F={nf} T={nt} physique={int(has_physique)}"
        )
        print(f"  material_hist={dict(sorted(material_hist.items()))}")
        print(f"  texlink_chain_hist={dict(sorted(chain_hist.items()))}")
        print(f"  max_layers={max_layers}")
        print(
            f"  invalid_vertex_faces={invalid_vertex_faces} "
            f"invalid_material_faces={invalid_material_faces}"
        )
        print(
            f"  invalid_face_tex_ptrs={invalid_face_tex_ptrs} "
            f"invalid_next_ptrs={invalid_next_ptrs} "
            f"loops={looped_chains}"
        )
        print(
            f"  null_face_tex={null_face_tex} "
            f"unreferenced_texlinks={unreferenced_texlinks} "
            f"multi_referenced_texlinks={multi_referenced}"
        )
        if has_physique:
            print(
                f"  bone_assignments={len(bones)} "
                f"unique_bones={len(bone_hist)}"
            )
            top_bones = bone_hist.most_common(8)
            print(f"  top_bones={top_bones}")

        for s, face in enumerate(faces[:max(0, args.samples)]):
            print(
                f"  face[{s}] vertices={face['vertices']} "
                f"material={face['material']} "
                f"layers={len(face['texlink_chain'])}"
            )
            for layer_no, layer in enumerate(face["texlink_chain"]):
                uv = list(zip(layer["u"], layer["v"]))
                print(
                    f"    layer[{layer_no}] texlink={layer['index']} "
                    f"hTexture=0x{layer['hTexture']:08X} UV={uv}"
                )
            if has_physique:
                vbones = [bones[v] for v in face["vertices"]]
                print(f"    vertex_bones={vbones}")

        print(f"  object_check={'PASS' if obj_ok else 'FAIL'}")

        report["objects"].append({
            "index": i,
            "name": info["name"],
            "node_name": node_name,
            "node_parent": node_parent,
            "nVertex": nv,
            "nFace": nf,
            "nTexLink": nt,
            "has_physique": has_physique,
            "materials": dict(sorted(material_hist.items())),
            "texlink_chain_hist": dict(sorted(chain_hist.items())),
            "max_layers": max_layers,
            "invalid_vertex_faces": invalid_vertex_faces,
            "invalid_material_faces": invalid_material_faces,
            "invalid_face_tex_ptrs": invalid_face_tex_ptrs,
            "invalid_next_ptrs": invalid_next_ptrs,
            "looped_chains": looped_chains,
            "null_face_tex": null_face_tex,
            "unreferenced_texlinks": unreferenced_texlinks,
            "multi_referenced_texlinks": multi_referenced,
            "bone_hist": dict(sorted(bone_hist.items())),
            "object_check": obj_ok,
        })

    print()
    print("SUMMARY")
    print(f"objects={obj_counter}")
    print(f"vertices={total_vertices}")
    print(f"faces={total_faces}")
    print(f"texlinks={total_texlinks}")
    print(f"physique_assignments={total_physique}")
    print(f"semantic_check={'PASS' if all_ok else 'FAIL'}")

    report["summary"] = {
        "vertices": total_vertices,
        "faces": total_faces,
        "texlinks": total_texlinks,
        "physique_assignments": total_physique,
        "semantic_check": all_ok,
    }

    if args.json_path:
        args.json_path.write_text(
            json.dumps(report, indent=2, ensure_ascii=False),
            encoding="utf-8",
        )
        print(f"json={args.json_path}")

    return 0 if all_ok else 10


if __name__ == "__main__":
    sys.exit(main())

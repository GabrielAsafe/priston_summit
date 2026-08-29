#!/usr/bin/env python3
import argparse
import struct
import sys
from pathlib import Path

SMD_HEADER_TEXT = b"SMD Model data Ver 0.62"

SIZE_FRAME_POS = 16
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

# smOBJ3D offsets confirmed by the MinGW i686 ABI probe.
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
OFF_PARENT_PTR = 236

OFF_TMROT_PTR = 668
OFF_TMPOS_PTR = 672
OFF_TMSCALE_PTR = 676
OFF_TMPREVROT_PTR = 680

OFF_TMROTCNT = 684
OFF_TMPOSCNT = 688
OFF_TMSCALECNT = 692
OFF_TMFRAMECNT = 2232


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def checked_slice(data, start, size, label):
    end = start + size
    if start < 0 or size < 0 or end > len(data):
        raise ValueError(
            f"{label}: faixa invalida start={start} size={size} "
            f"end={end} file_size={len(data)}"
        )
    return data[start:end]


def expected_obj_length(nv, nf, nt, nr, np, ns, has_physique):
    size = SIZE_OBJ3D
    size += SIZE_VERTEX * nv
    size += SIZE_FACE * nf
    size += SIZE_TEXLINK * nt
    size += SIZE_TM_ROT * nr
    size += SIZE_TM_POS * np
    size += SIZE_TM_SCALE * ns
    size += SIZE_MATRIX * nr
    if has_physique:
        size += 32 * nv
    return size


def main():
    ap = argparse.ArgumentParser(
        description="Read-only structural probe for Priston Tale SMD Model data Ver 0.62"
    )
    ap.add_argument("smd", type=Path)
    ap.add_argument(
        "--samples",
        type=int,
        default=2,
        help="number of vertex/face samples to print per object (default: 2)",
    )
    args = ap.parse_args()

    path = args.smd
    data = path.read_bytes()
    size = len(data)

    print("PT_SMD_PROBE_V1")
    print(f"file={path}")
    print(f"file_size={size}")

    if size < SIZE_DFILE_HEADER:
        print("FAIL: arquivo menor que smDFILE_HEADER")
        return 2

    header = data[:SIZE_DFILE_HEADER]
    header_text = cstr(header[:24])

    print(f"header={header_text!r}")
    if not header.startswith(SMD_HEADER_TEXT):
        print("FAIL: header SMD inesperado")
        return 3
    print("header_check=OK")

    obj_counter = i32(header, 24)
    mat_counter = i32(header, 28)
    mat_file_point = i32(header, 32)
    first_obj_point = i32(header, 36)
    tm_frame_counter = i32(header, 40)

    print(f"ObjCounter={obj_counter}")
    print(f"MatCounter={mat_counter}")
    print(f"MatFilePoint={mat_file_point}")
    print(f"First_ObjInfoPoint={first_obj_point}")
    print(f"TmFrameCounter={tm_frame_counter}")

    if obj_counter < 0 or obj_counter > 128:
        print("FAIL: ObjCounter fora do limite esperado 0..128")
        return 4

    obj_table_start = SIZE_DFILE_HEADER
    obj_table_size = SIZE_DFILE_OBJINFO * obj_counter
    obj_table_end = obj_table_start + obj_table_size

    print(f"obj_table_start={obj_table_start}")
    print(f"obj_table_end={obj_table_end}")
    print(f"material_region_size={first_obj_point - obj_table_end}")

    if mat_file_point != obj_table_end:
        print(
            "WARN: MatFilePoint nao coincide com fim da tabela de objetos: "
            f"{mat_file_point} != {obj_table_end}"
        )
    else:
        print("MatFilePoint_check=OK")

    if first_obj_point < obj_table_end or first_obj_point > size:
        print("FAIL: First_ObjInfoPoint fora da faixa plausivel")
        return 5

    checked_slice(data, obj_table_start, obj_table_size, "object table")

    infos = []
    for idx in range(obj_counter):
        off = obj_table_start + idx * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]
        name = cstr(raw[:32])
        length = i32(raw, 32)
        obj_file_point = i32(raw, 36)
        infos.append((name, length, obj_file_point))

    print()
    print("OBJECT_TABLE")
    for idx, (name, length, point) in enumerate(infos):
        print(
            f"[{idx:02d}] name={name!r} "
            f"Length={length} ObjFilePoint={point}"
        )

    all_ok = True
    last_end = 0

    print()
    print("OBJECT_VALIDATION")

    for idx, (table_name, table_length, obj_start) in enumerate(infos):
        print()
        print(f"--- object[{idx:02d}] {table_name!r} ---")

        if obj_start < 0 or obj_start + SIZE_OBJ3D > size:
            print("FAIL object header outside file")
            all_ok = False
            continue

        blob = data[obj_start:obj_start + SIZE_OBJ3D]

        head = u32(blob, OFF_HEAD)
        vertex_ptr = u32(blob, OFF_VERTEX_PTR)
        face_ptr = u32(blob, OFF_FACE_PTR)
        tex_ptr = u32(blob, OFF_TEXLINK_PTR)
        physique_ptr = u32(blob, OFF_PHYSIQUE_PTR)

        nv = i32(blob, OFF_NVERTEX)
        nf = i32(blob, OFF_NFACE)
        nt = i32(blob, OFF_NTEXLINK)

        node_name = cstr(blob[OFF_NODENAME:OFF_NODENAME + 32])
        node_parent = cstr(blob[OFF_NODEPARENT:OFF_NODEPARENT + 32])
        parent_ptr = u32(blob, OFF_PARENT_PTR)

        tmrot_ptr = u32(blob, OFF_TMROT_PTR)
        tmpos_ptr = u32(blob, OFF_TMPOS_PTR)
        tmscale_ptr = u32(blob, OFF_TMSCALE_PTR)
        tmprev_ptr = u32(blob, OFF_TMPREVROT_PTR)

        nr = i32(blob, OFF_TMROTCNT)
        np = i32(blob, OFF_TMPOSCNT)
        ns = i32(blob, OFF_TMSCALECNT)
        frame_cnt = i32(blob, OFF_TMFRAMECNT)

        print(f"Head=0x{head:08X}")
        print(
            "ptrs="
            f"Vertex:0x{vertex_ptr:08X} "
            f"Face:0x{face_ptr:08X} "
            f"TexLink:0x{tex_ptr:08X} "
            f"Physique:0x{physique_ptr:08X}"
        )
        print(f"counts=nVertex:{nv} nFace:{nf} nTexLink:{nt}")
        print(f"NodeName={node_name!r} NodeParent={node_parent!r}")
        print(f"pParent=0x{parent_ptr:08X}")
        print(
            "anim_ptrs="
            f"TmRot:0x{tmrot_ptr:08X} "
            f"TmPos:0x{tmpos_ptr:08X} "
            f"TmScale:0x{tmscale_ptr:08X} "
            f"TmPrevRot:0x{tmprev_ptr:08X}"
        )
        print(
            f"anim_counts=TmRotCnt:{nr} TmPosCnt:{np} "
            f"TmScaleCnt:{ns} TmFrameCnt:{frame_cnt}"
        )

        sane_counts = (
            0 <= nv <= 2_000_000
            and 0 <= nf <= 4_000_000
            and 0 <= nt <= 4_000_000
            and 0 <= nr <= 10_000_000
            and 0 <= np <= 10_000_000
            and 0 <= ns <= 10_000_000
        )
        if not sane_counts:
            print("FAIL implausible counts")
            all_ok = False
            continue

        has_physique = physique_ptr != 0
        calc_length = expected_obj_length(nv, nf, nt, nr, np, ns, has_physique)
        obj_end = obj_start + calc_length

        print(f"has_physique={int(has_physique)}")
        print(f"calculated_Length={calc_length}")
        print(f"table_Length={table_length}")
        print(f"calculated_end={obj_end}")

        if node_name.lower() == table_name.lower():
            print("NodeName_check=OK")
        else:
            print(
                f"WARN NodeName mismatch: table={table_name!r} blob={node_name!r}"
            )

        if calc_length == table_length:
            print("Length_check=OK")
        else:
            print("FAIL Length mismatch")
            all_ok = False

        if obj_end <= size:
            print("bounds_check=OK")
        else:
            print("FAIL object exceeds file")
            all_ok = False
            continue

        if idx + 1 < len(infos):
            next_start = infos[idx + 1][2]
            if obj_end == next_start:
                print("next_object_check=OK")
            else:
                print(
                    f"FAIL next object offset mismatch: "
                    f"calculated_end={obj_end} next_start={next_start}"
                )
                all_ok = False
        else:
            if obj_end == size:
                print("final_file_size_check=OK")
            else:
                print(
                    f"WARN last object ends at {obj_end}, "
                    f"file size is {size}, trailing={size - obj_end}"
                )

        # Array offsets inside object block.
        vertices_start = obj_start + SIZE_OBJ3D
        faces_start = vertices_start + SIZE_VERTEX * nv
        texlinks_start = faces_start + SIZE_FACE * nf
        tmrot_start = texlinks_start + SIZE_TEXLINK * nt
        tmpos_start = tmrot_start + SIZE_TM_ROT * nr
        tmscale_start = tmpos_start + SIZE_TM_POS * np
        tmprev_start = tmscale_start + SIZE_TM_SCALE * ns
        physique_start = tmprev_start + SIZE_MATRIX * nr if has_physique else None

        print(
            "array_offsets="
            f"V:{vertices_start} "
            f"F:{faces_start} "
            f"T:{texlinks_start} "
            f"R:{tmrot_start} "
            f"P:{tmpos_start} "
            f"S:{tmscale_start} "
            f"PrevR:{tmprev_start}"
            + (
                f" Phys:{physique_start}"
                if physique_start is not None
                else ""
            )
        )

        sample_count = max(0, args.samples)

        for s in range(min(sample_count, nv)):
            voff = vertices_start + s * SIZE_VERTEX
            x, y, z, nx, ny, nz = struct.unpack_from("<6i", data, voff)
            print(
                f"vertex[{s}] xyz=({x},{y},{z}) "
                f"normal=({nx},{ny},{nz})"
            )

        for s in range(min(sample_count, nf)):
            foff = faces_start + s * SIZE_FACE
            v0, v1, v2, v3 = struct.unpack_from("<4H", data, foff)
            tvals = struct.unpack_from("<6f", data, foff + 8)
            texlink_ptr = u32(data, foff + 32)
            print(
                f"face[{s}] v=({v0},{v1},{v2},{v3}) "
                f"t=({tvals[0]:.6g},{tvals[1]:.6g}) "
                f"({tvals[2]:.6g},{tvals[3]:.6g}) "
                f"({tvals[4]:.6g},{tvals[5]:.6g}) "
                f"lpTexLink=0x{texlink_ptr:08X}"
            )

        last_end = max(last_end, obj_end)

    print()
    print("SUMMARY")
    print(f"objects={obj_counter}")
    print(f"last_calculated_end={last_end}")
    print(f"file_size={size}")
    print(f"structural_check={'PASS' if all_ok else 'FAIL'}")

    return 0 if all_ok else 10


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
# Priston Tale SMB animation-track inventory probe.
# Read-only. Does NOT evaluate animation or modify any file.

import argparse
import struct
from collections import Counter
from pathlib import Path

HEADER = b"SMD Model data Ver 0.62"

SIZE_DFILE_HEADER = 556
SIZE_DFILE_OBJINFO = 40
SIZE_OBJ3D = 2236

SIZE_VERTEX = 24
SIZE_FACE = 36
SIZE_TEXLINK = 32
SIZE_TM_ROT = 20
SIZE_TM_POS = 16
SIZE_TM_SCALE = 16
SIZE_TM_PREVROT = 64

O_NVERTEX = 84
O_NFACE = 88
O_NTEXLINK = 92

O_NODENAME = 172
O_NODEPARENT = 204

O_TMROTCNT = 684
O_TMPOSCNT = 688
O_TMSCALECNT = 692

O_TMROTFRAME = 696
O_TMPOSFRAME = 1208
O_TMSCALEFRAME = 1720
O_TMFRAMECNT = 2232

OBJ_FRAME_SEARCH_MAX = 32


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def frame_pos(data, off):
    return struct.unpack_from("<4i", data, off)


def parse_frame_table(data, off, count):
    result = []
    for idx in range(count):
        p = off + idx * 16
        start, end, pos_num, pos_cnt = frame_pos(data, p)
        result.append({
            "index": idx,
            "start": start,
            "end": end,
            "pos_num": pos_num,
            "pos_cnt": pos_cnt,
        })
    return result


def parse_header(data):
    if len(data) < SIZE_DFILE_HEADER:
        raise RuntimeError("file too small")

    if not data.startswith(HEADER):
        raise RuntimeError("invalid SMD/SMB header")

    obj_count = i32(data, 24)
    mat_count = i32(data, 28)
    mat_point = i32(data, 32)
    first_obj = i32(data, 36)
    tm_frame_count = i32(data, 40)

    if not 0 <= tm_frame_count <= OBJ_FRAME_SEARCH_MAX:
        raise RuntimeError(
            f"invalid header TmFrameCounter={tm_frame_count}"
        )

    expected_mat_point = (
        SIZE_DFILE_HEADER
        + obj_count * SIZE_DFILE_OBJINFO
    )

    if mat_point != expected_mat_point:
        raise RuntimeError(
            f"MatFilePoint={mat_point} expected={expected_mat_point}"
        )

    header_frames = parse_frame_table(
        data, 44, tm_frame_count
    )

    infos = []
    for idx in range(obj_count):
        off = SIZE_DFILE_HEADER + idx * SIZE_DFILE_OBJINFO
        raw = data[off:off + SIZE_DFILE_OBJINFO]

        infos.append({
            "index": idx,
            "name": cstr(raw[:32]),
            "length": i32(raw, 32),
            "start": i32(raw, 36),
        })

    return {
        "obj_count": obj_count,
        "mat_count": mat_count,
        "mat_point": mat_point,
        "first_obj": first_obj,
        "tm_frame_count": tm_frame_count,
        "header_frames": header_frames,
        "infos": infos,
    }


def track_frames(data, off, count, stride):
    return [i32(data, off + i * stride) for i in range(count)]


def monotonic_non_decreasing(values):
    return all(a <= b for a, b in zip(values, values[1:]))


def validate_segments(segments, track_count):
    errors = []

    for seg in segments:
        start = seg["start"]
        end = seg["end"]
        pos_num = seg["pos_num"]
        pos_cnt = seg["pos_cnt"]

        if end < start:
            errors.append(
                f"segment[{seg['index']}] end<start"
            )

        if pos_num < 0 or pos_cnt < 0:
            errors.append(
                f"segment[{seg['index']}] negative PosNum/PosCnt"
            )
            continue

        if pos_num + pos_cnt > track_count:
            errors.append(
                f"segment[{seg['index']}] "
                f"PosNum+PosCnt={pos_num+pos_cnt} "
                f"> track_count={track_count}"
            )

    return errors


def parse_object(data, info):
    start = info["start"]
    end = start + info["length"]

    fixed = data[start:start + SIZE_OBJ3D]
    if len(fixed) != SIZE_OBJ3D:
        raise RuntimeError(
            f"object {info['index']} fixed struct truncated"
        )

    name = cstr(
        fixed[O_NODENAME:O_NODENAME + 32]
    )
    parent = cstr(
        fixed[O_NODEPARENT:O_NODEPARENT + 32]
    )

    if name != info["name"]:
        raise RuntimeError(
            f"object table/name mismatch "
            f"{info['name']!r} != {name!r}"
        )

    nv = i32(fixed, O_NVERTEX)
    nf = i32(fixed, O_NFACE)
    nt = i32(fixed, O_NTEXLINK)

    nr = i32(fixed, O_TMROTCNT)
    np = i32(fixed, O_TMPOSCNT)
    ns = i32(fixed, O_TMSCALECNT)
    nseg = i32(fixed, O_TMFRAMECNT)

    for label, value in (
        ("nVertex", nv),
        ("nFace", nf),
        ("nTexLink", nt),
        ("TmRotCnt", nr),
        ("TmPosCnt", np),
        ("TmScaleCnt", ns),
    ):
        if value < 0:
            raise RuntimeError(
                f"{name}: negative {label}={value}"
            )

    if not 0 <= nseg <= OBJ_FRAME_SEARCH_MAX:
        raise RuntimeError(
            f"{name}: invalid TmFrameCnt={nseg}"
        )

    cursor = start + SIZE_OBJ3D

    vertex_off = cursor
    cursor += nv * SIZE_VERTEX

    face_off = cursor
    cursor += nf * SIZE_FACE

    tex_off = cursor
    cursor += nt * SIZE_TEXLINK

    rot_off = cursor
    cursor += nr * SIZE_TM_ROT

    pos_off = cursor
    cursor += np * SIZE_TM_POS

    scale_off = cursor
    cursor += ns * SIZE_TM_SCALE

    prevrot_off = cursor
    cursor += nr * SIZE_TM_PREVROT

    # m5.smb skeleton objects normally have no Physique payload because
    # they have no vertices. If a future file differs, any remaining
    # bytes are reported rather than guessed.
    calculated_without_physique = cursor
    remaining = end - cursor

    rot_frames = track_frames(
        data, rot_off, nr, SIZE_TM_ROT
    )
    pos_frames = track_frames(
        data, pos_off, np, SIZE_TM_POS
    )
    scale_frames = track_frames(
        data, scale_off, ns, SIZE_TM_SCALE
    )

    rot_segments = parse_frame_table(
        fixed, O_TMROTFRAME, nseg
    )
    pos_segments = parse_frame_table(
        fixed, O_TMPOSFRAME, nseg
    )
    scale_segments = parse_frame_table(
        fixed, O_TMSCALEFRAME, nseg
    )

    errors = []

    if remaining != 0:
        errors.append(
            f"serialized remainder={remaining} bytes"
        )

    if rot_frames and not monotonic_non_decreasing(rot_frames):
        errors.append("TmRot frames not monotonic")

    if pos_frames and not monotonic_non_decreasing(pos_frames):
        errors.append("TmPos frames not monotonic")

    if scale_frames and not monotonic_non_decreasing(scale_frames):
        errors.append("TmScale frames not monotonic")

    errors.extend(
        "rot:" + x
        for x in validate_segments(rot_segments, nr)
    )
    errors.extend(
        "pos:" + x
        for x in validate_segments(pos_segments, np)
    )
    errors.extend(
        "scale:" + x
        for x in validate_segments(scale_segments, ns)
    )

    return {
        "index": info["index"],
        "name": name,
        "parent": parent,
        "nvertex": nv,
        "nface": nf,
        "ntexlink": nt,
        "rot_count": nr,
        "pos_count": np,
        "scale_count": ns,
        "segment_count": nseg,
        "rot_frames": rot_frames,
        "pos_frames": pos_frames,
        "scale_frames": scale_frames,
        "rot_segments": rot_segments,
        "pos_segments": pos_segments,
        "scale_segments": scale_segments,
        "calculated_end": calculated_without_physique,
        "table_end": end,
        "errors": errors,
    }


def range_text(values):
    if not values:
        return "-"
    return f"{values[0]}..{values[-1]}"


def segment_text(segments):
    if not segments:
        return "-"

    return "; ".join(
        (
            f"{s['index']}:"
            f"{s['start']}..{s['end']}"
            f"@{s['pos_num']}+{s['pos_cnt']}"
        )
        for s in segments
    )


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--skeleton",
        required=True,
        type=Path,
    )
    args = ap.parse_args()

    data = args.skeleton.read_bytes()
    header = parse_header(data)

    objects = [
        parse_object(data, info)
        for info in header["infos"]
    ]

    print("PT_SMB_ANIMATION_TRACK_INVENTORY_V1")
    print(f"skeleton={args.skeleton}")
    print(f"file_size={len(data)}")
    print()

    print("HEADER")
    print(f"objects={header['obj_count']}")
    print(f"materials={header['mat_count']}")
    print(f"TmFrameCounter={header['tm_frame_count']}")
    print()

    print("HEADER_TMFRAME")
    for seg in header["header_frames"]:
        print(
            f"[{seg['index']}] "
            f"StartFrame={seg['start']} "
            f"EndFrame={seg['end']} "
            f"PosNum={seg['pos_num']} "
            f"PosCnt={seg['pos_cnt']}"
        )
    print()

    with_rot = [o for o in objects if o["rot_count"]]
    with_pos = [o for o in objects if o["pos_count"]]
    with_scale = [o for o in objects if o["scale_count"]]
    with_segments = [
        o for o in objects if o["segment_count"]
    ]

    print("TRACK_COUNTS")
    print(f"objects_with_rotation={len(with_rot)}")
    print(f"objects_with_position={len(with_pos)}")
    print(f"objects_with_scale={len(with_scale)}")
    print(f"objects_with_TmFrameCnt={len(with_segments)}")
    print(
        f"total_rotation_keys="
        f"{sum(o['rot_count'] for o in objects)}"
    )
    print(
        f"total_position_keys="
        f"{sum(o['pos_count'] for o in objects)}"
    )
    print(
        f"total_scale_keys="
        f"{sum(o['scale_count'] for o in objects)}"
    )
    print()

    seg_hist = Counter(
        o["segment_count"] for o in objects
    )

    print("TMFRAMECOUNT_HISTOGRAM")
    for nseg in sorted(seg_hist):
        print(
            f"TmFrameCnt={nseg}: objects={seg_hist[nseg]}"
        )
    print()

    all_errors = []

    for obj in objects:
        if obj["errors"]:
            for err in obj["errors"]:
                all_errors.append(
                    f"{obj['name']}: {err}"
                )

    print("KEY_BONES")
    key_names = [
        "Bip01",
        "Bip01 Pelvis",
        "Bip01 Spine",
        "Bip01 Spine1",
        "Bip01 Neck",
        "Bip01 Head",
        "Bip01 L UpperArm",
        "Bip01 L Forearm",
        "Bip01 L Hand",
        "Bip01 R UpperArm",
        "Bip01 R Forearm",
        "Bip01 R Hand",
        "Bip01 L Thigh",
        "Bip01 L Calf",
        "Bip01 L Foot",
        "Bip01 R Thigh",
        "Bip01 R Calf",
        "Bip01 R Foot",
    ]

    by_name = {
        obj["name"]: obj for obj in objects
    }

    for name in key_names:
        obj = by_name.get(name)

        if obj is None:
            print(f"{name!r}: NOT_FOUND")
            continue

        print(
            f"{name!r}: "
            f"R={obj['rot_count']} "
            f"P={obj['pos_count']} "
            f"S={obj['scale_count']} "
            f"segments={obj['segment_count']} "
            f"Rframes={range_text(obj['rot_frames'])} "
            f"Pframes={range_text(obj['pos_frames'])} "
            f"Sframes={range_text(obj['scale_frames'])}"
        )

        if obj["segment_count"]:
            print(
                "  rot_segments="
                + segment_text(obj["rot_segments"])
            )
            print(
                "  pos_segments="
                + segment_text(obj["pos_segments"])
            )
            print(
                "  scale_segments="
                + segment_text(obj["scale_segments"])
            )
    print()

    print("OBJECT_TRACK_TABLE")
    for obj in objects:
        print(
            f"[{obj['index']:02d}] "
            f"{obj['name']!r} "
            f"R={obj['rot_count']} "
            f"P={obj['pos_count']} "
            f"S={obj['scale_count']} "
            f"Seg={obj['segment_count']} "
            f"R={range_text(obj['rot_frames'])} "
            f"P={range_text(obj['pos_frames'])} "
            f"S={range_text(obj['scale_frames'])}"
        )
    print()

    print("VALIDATION")
    print(f"errors={len(all_errors)}")
    for err in all_errors[:100]:
        print(err)

    ok = (
        header["obj_count"] == len(objects)
        and not all_errors
    )

    print(
        "animation_track_inventory_check="
        + ("PASS" if ok else "FAIL")
    )


main()

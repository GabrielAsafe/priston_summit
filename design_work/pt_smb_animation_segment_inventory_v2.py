#!/usr/bin/env python3
# Priston Tale SMB animation segment inventory V2.
# Read-only.
#
# Source semantics reproduced:
#   GetTmFrame*(frame):
#       if PosCnt > 0 and StartFrame <= frame and EndFrame > frame:
#           return PosNum
#       return -1
#
# Therefore:
# - PosNum is the starting index of that segment's key slice.
# - PosNum is irrelevant when PosCnt <= 0.
# - monotonicity must be checked inside each active segment, not globally.

import argparse
import struct
from collections import Counter, defaultdict
from pathlib import Path

HEADER = b"SMD Model data Ver 0.62"

SIZE_HEADER = 556
SIZE_OBJINFO = 40
SIZE_OBJ3D = 2236

SIZE_VERTEX = 24
SIZE_FACE = 36
SIZE_TEXLINK = 32
SIZE_TM_ROT = 20
SIZE_TM_POS = 16
SIZE_TM_SCALE = 16
SIZE_PREVROT = 64

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

MAX_SEGMENTS = 32


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def frame_pos(data, off):
    return struct.unpack_from("<4i", data, off)


def parse_frame_table(data, off, count):
    out = []
    for idx in range(count):
        start, end, pos_num, pos_cnt = frame_pos(data, off + idx * 16)
        out.append({
            "slot": idx,
            "start": start,
            "end": end,
            "pos_num": pos_num,
            "pos_cnt": pos_cnt,
        })
    return out


def active_by_source(seg):
    # Exact condition that matters before frame range testing:
    # GetTmFrame* ignores PosNum entirely unless PosCnt > 0.
    return seg["pos_cnt"] > 0


def parse_header(data):
    if len(data) < SIZE_HEADER:
        raise RuntimeError("file too small")
    if not data.startswith(HEADER):
        raise RuntimeError("invalid SMD/SMB header")

    obj_count = i32(data, 24)
    mat_count = i32(data, 28)
    mat_point = i32(data, 32)
    first_obj = i32(data, 36)
    tm_frame_count = i32(data, 40)

    if not 0 <= tm_frame_count <= MAX_SEGMENTS:
        raise RuntimeError(f"invalid TmFrameCounter={tm_frame_count}")

    expected_mat = SIZE_HEADER + obj_count * SIZE_OBJINFO
    if mat_point != expected_mat:
        raise RuntimeError(
            f"MatFilePoint={mat_point} expected={expected_mat}"
        )

    infos = []
    for idx in range(obj_count):
        off = SIZE_HEADER + idx * SIZE_OBJINFO
        raw = data[off:off + SIZE_OBJINFO]
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
        "frames": parse_frame_table(data, 44, tm_frame_count),
        "infos": infos,
    }


def read_track_frames(data, off, count, stride):
    return [i32(data, off + i * stride) for i in range(count)]


def nondecreasing(values):
    return all(a <= b for a, b in zip(values, values[1:]))


def duplicates(values):
    return sum(1 for a, b in zip(values, values[1:]) if a == b)


def slice_ranges(active_segments):
    return sorted(
        (
            seg["pos_num"],
            seg["pos_num"] + seg["pos_cnt"],
            seg["slot"],
        )
        for seg in active_segments
    )


def validate_track(name, kind, frames, segments, header_frames):
    errors = []
    warnings = []
    active = []

    for seg in segments:
        slot = seg["slot"]

        if not active_by_source(seg):
            # Exact engine behavior: ignored. PosNum may contain 0xCCCCCCCC.
            continue

        active.append(seg)

        if seg["end"] <= seg["start"]:
            errors.append(
                f"{name} {kind} slot={slot}: active PosCnt but EndFrame<=StartFrame"
            )

        if seg["pos_num"] < 0:
            errors.append(
                f"{name} {kind} slot={slot}: active PosNum={seg['pos_num']} < 0"
            )
            continue

        end_index = seg["pos_num"] + seg["pos_cnt"]
        if end_index > len(frames):
            errors.append(
                f"{name} {kind} slot={slot}: "
                f"PosNum+PosCnt={end_index} > track_count={len(frames)}"
            )
            continue

        if seg["pos_cnt"] < 2:
            warnings.append(
                f"{name} {kind} slot={slot}: active segment has only "
                f"{seg['pos_cnt']} key(s)"
            )

        vals = frames[seg["pos_num"]:end_index]

        if vals and not nondecreasing(vals):
            errors.append(
                f"{name} {kind} slot={slot}: key frames not monotonic inside segment"
            )

        dup = duplicates(vals)
        if dup:
            warnings.append(
                f"{name} {kind} slot={slot}: duplicate adjacent key frames={dup}"
            )

        if vals:
            if vals[0] > seg["start"]:
                errors.append(
                    f"{name} {kind} slot={slot}: first_key={vals[0]} "
                    f"> StartFrame={seg['start']}"
                )

            # Get*Frame searches for e > frame while frame < EndFrame.
            # The final key therefore needs to cover the segment's upper edge.
            if vals[-1] < seg["end"]:
                errors.append(
                    f"{name} {kind} slot={slot}: last_key={vals[-1]} "
                    f"< EndFrame={seg['end']}"
                )

        if slot < len(header_frames):
            h = header_frames[slot]
            # Header PosNum/PosCnt are not used for object track lookup,
            # but Start/End should describe the same global slot.
            if (
                seg["start"] != h["start"]
                or seg["end"] != h["end"]
            ):
                errors.append(
                    f"{name} {kind} slot={slot}: object range "
                    f"{seg['start']}..{seg['end']} != header "
                    f"{h['start']}..{h['end']}"
                )

    # Track arrays should be partitioned by active slices. This catches
    # overlap/orphan/gap while allowing inactive slots.
    ranges = slice_ranges(active)

    if not ranges:
        if frames:
            errors.append(
                f"{name} {kind}: {len(frames)} keys but no active segment"
            )
    else:
        cursor = 0
        for start, end, slot in ranges:
            if start < cursor:
                errors.append(
                    f"{name} {kind}: overlapping slice at slot={slot}, "
                    f"start={start} cursor={cursor}"
                )
            elif start > cursor:
                errors.append(
                    f"{name} {kind}: orphan/gap keys [{cursor},{start})"
                )
            cursor = max(cursor, end)

        if cursor != len(frames):
            errors.append(
                f"{name} {kind}: active slices end at {cursor}, "
                f"track_count={len(frames)}"
            )

    return active, errors, warnings


def parse_object(data, info, header_frames):
    start = info["start"]
    end = start + info["length"]

    fixed = data[start:start + SIZE_OBJ3D]
    if len(fixed) != SIZE_OBJ3D:
        raise RuntimeError(f"truncated object {info['index']}")

    name = cstr(fixed[O_NODENAME:O_NODENAME + 32])
    parent = cstr(fixed[O_NODEPARENT:O_NODEPARENT + 32])

    if name != info["name"]:
        raise RuntimeError(
            f"object table/name mismatch {info['name']!r} != {name!r}"
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
            raise RuntimeError(f"{name}: negative {label}={value}")

    if not 0 <= nseg <= MAX_SEGMENTS:
        raise RuntimeError(f"{name}: invalid TmFrameCnt={nseg}")

    cursor = start + SIZE_OBJ3D

    cursor += nv * SIZE_VERTEX
    cursor += nf * SIZE_FACE
    cursor += nt * SIZE_TEXLINK

    rot_off = cursor
    cursor += nr * SIZE_TM_ROT

    pos_off = cursor
    cursor += np * SIZE_TM_POS

    scale_off = cursor
    cursor += ns * SIZE_TM_SCALE

    prevrot_off = cursor
    cursor += nr * SIZE_PREVROT

    remainder = end - cursor

    rot_frames = read_track_frames(data, rot_off, nr, SIZE_TM_ROT)
    pos_frames = read_track_frames(data, pos_off, np, SIZE_TM_POS)
    scale_frames = read_track_frames(data, scale_off, ns, SIZE_TM_SCALE)

    rot_segments = parse_frame_table(fixed, O_TMROTFRAME, nseg)
    pos_segments = parse_frame_table(fixed, O_TMPOSFRAME, nseg)
    scale_segments = parse_frame_table(fixed, O_TMSCALEFRAME, nseg)

    active_rot, rot_errors, rot_warnings = validate_track(
        name, "ROT", rot_frames, rot_segments, header_frames
    )
    active_pos, pos_errors, pos_warnings = validate_track(
        name, "POS", pos_frames, pos_segments, header_frames
    )
    active_scale, scale_errors, scale_warnings = validate_track(
        name, "SCALE", scale_frames, scale_segments, header_frames
    )

    errors = []
    warnings = []

    if remainder != 0:
        errors.append(
            f"{name}: serialized remainder={remainder} bytes"
        )

    errors.extend(rot_errors)
    errors.extend(pos_errors)
    errors.extend(scale_errors)
    warnings.extend(rot_warnings)
    warnings.extend(pos_warnings)
    warnings.extend(scale_warnings)

    return {
        "index": info["index"],
        "name": name,
        "parent": parent,
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
        "active_rot": active_rot,
        "active_pos": active_pos,
        "active_scale": active_scale,
        "errors": errors,
        "warnings": warnings,
    }


def seg_summary(seg, frames):
    if not active_by_source(seg):
        return (
            f"slot{seg['slot']} inactive "
            f"range={seg['start']}..{seg['end']} "
            f"PosNum={seg['pos_num']} PosCnt={seg['pos_cnt']}"
        )

    a = seg["pos_num"]
    b = a + seg["pos_cnt"]
    vals = frames[a:b]

    if vals:
        key_range = f"{vals[0]}..{vals[-1]}"
    else:
        key_range = "-"

    return (
        f"slot{seg['slot']} active "
        f"range={seg['start']}..{seg['end']} "
        f"slice={a}:{b} keys={seg['pos_cnt']} "
        f"keyframes={key_range}"
    )


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--skeleton", required=True, type=Path)
    args = ap.parse_args()

    data = args.skeleton.read_bytes()
    header = parse_header(data)

    objects = [
        parse_object(data, info, header["frames"])
        for info in header["infos"]
    ]

    print("PT_SMB_ANIMATION_SEGMENT_INVENTORY_V2")
    print(f"skeleton={args.skeleton}")
    print(f"file_size={len(data)}")
    print()

    print("SOURCE_SEMANTICS")
    print("active_segment = PosCnt > 0")
    print("frame_match = StartFrame <= frame < EndFrame")
    print("returned_track_index = PosNum")
    print("inactive_PosNum_is_ignored = YES")
    print()

    print("HEADER")
    print(f"objects={header['obj_count']}")
    print(f"materials={header['mat_count']}")
    print(f"TmFrameCounter={header['tm_frame_count']}")
    print()

    print("HEADER_SLOTS")
    global_active_ranges = []
    for seg in header["frames"]:
        # Header slots do not use PosCnt the same way as object tracks;
        # a non-empty Start/End range identifies the global animation slot.
        range_active = seg["end"] > seg["start"]
        state = "ACTIVE_RANGE" if range_active else "EMPTY"
        if range_active:
            global_active_ranges.append(seg["slot"])
        print(
            f"[{seg['slot']}] {state} "
            f"StartFrame={seg['start']} EndFrame={seg['end']} "
            f"raw_PosNum={seg['pos_num']} raw_PosCnt={seg['pos_cnt']}"
        )

    print(
        f"header_active_range_slots={global_active_ranges!r}"
    )
    print(
        f"header_active_range_count={len(global_active_ranges)}"
    )
    print()

    print("TRACK_TOTALS")
    print(
        f"objects_with_rotation="
        f"{sum(1 for o in objects if o['rot_count'] > 0)}"
    )
    print(
        f"objects_with_position="
        f"{sum(1 for o in objects if o['pos_count'] > 0)}"
    )
    print(
        f"objects_with_scale="
        f"{sum(1 for o in objects if o['scale_count'] > 0)}"
    )
    print(
        f"total_rotation_keys={sum(o['rot_count'] for o in objects)}"
    )
    print(
        f"total_position_keys={sum(o['pos_count'] for o in objects)}"
    )
    print(
        f"total_scale_keys={sum(o['scale_count'] for o in objects)}"
    )
    print()

    print("ACTIVE_OBJECTS_BY_SLOT")
    for slot in range(header["tm_frame_count"]):
        row = []
        for kind, field in (
            ("ROT", "active_rot"),
            ("POS", "active_pos"),
            ("SCALE", "active_scale"),
        ):
            active_objs = [
                o for o in objects
                if any(s["slot"] == slot for s in o[field])
            ]
            total_keys = sum(
                next(
                    s["pos_cnt"]
                    for s in o[field]
                    if s["slot"] == slot
                )
                for o in active_objs
            )
            row.append(
                f"{kind}:objects={len(active_objs)},keys={total_keys}"
            )

        h = header["frames"][slot]
        print(
            f"slot={slot} range={h['start']}..{h['end']} "
            + " ".join(row)
        )
    print()

    key_names = [
        "Bip01",
        "Bip01 Pelvis",
        "Bip01 Spine",
        "Bip01 Spine1",
        "Bip01 Neck",
        "Bip01 Head",
        "Bip01 L UpperArm",
        "Bip01 R UpperArm",
        "Bip01 L Thigh",
        "Bip01 R Thigh",
        "Bip weapon01",
        "Bip weapon02",
        "Bip in01",
        "Bip in-bow",
        "Bip in-cro",
    ]
    by_name = {o["name"]: o for o in objects}

    print("KEY_BONE_SEGMENTS")
    for name in key_names:
        o = by_name.get(name)
        if o is None:
            print(f"{name!r}: NOT_FOUND")
            continue

        print(
            f"{name!r}: "
            f"R={o['rot_count']} P={o['pos_count']} S={o['scale_count']}"
        )

        if o["rot_count"]:
            for seg in o["rot_segments"]:
                if active_by_source(seg):
                    print("  ROT " + seg_summary(seg, o["rot_frames"]))

        if o["pos_count"]:
            for seg in o["pos_segments"]:
                if active_by_source(seg):
                    print("  POS " + seg_summary(seg, o["pos_frames"]))

        if o["scale_count"]:
            for seg in o["scale_segments"]:
                if active_by_source(seg):
                    print("  SCL " + seg_summary(seg, o["scale_frames"]))
    print()

    errors = []
    warnings = []
    for o in objects:
        errors.extend(o["errors"])
        warnings.extend(o["warnings"])

    print("VALIDATION")
    print(f"errors={len(errors)}")
    print(f"warnings={len(warnings)}")

    for err in errors[:120]:
        print("ERROR " + err)

    for warning in warnings[:80]:
        print("WARN " + warning)

    print()

    tmframe_hist = Counter(o["segment_count"] for o in objects)
    print("TMFRAMECOUNT_HISTOGRAM")
    for count in sorted(tmframe_hist):
        print(f"TmFrameCnt={count}: objects={tmframe_hist[count]}")
    print()

    ok = (
        len(objects) == header["obj_count"]
        and not errors
    )

    print(
        "animation_segment_inventory_check="
        + ("PASS" if ok else "FAIL")
    )


main()

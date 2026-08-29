#!/usr/bin/env python3
# Priston Tale SMB source-equivalent motion evaluator V1
# Read-only. No Blender. No source asset modification.
#
# Scope:
# - decode one smMOTIONINFO from INX
# - map MotionFrame -> SMB TmFrame
# - evaluate all SMB bones in source parent-first TmSort order
# - reproduce GetTmFrame*, GetRotFrame, GetPosFrame, GetScaleFrame
# - reproduce qmat * parent.TmResult hierarchy
# - report numerical skeleton diagnostics
#
# Floating-point calculations reproduce the source algorithm, but are not
# intended as bit-identical emulation of a particular x86 FPU implementation.

import argparse
import math
import struct
from pathlib import Path


# ---------------------------------------------------------------------------
# Proven i686/cache layout
# ---------------------------------------------------------------------------

SMD_HEADER = b"SMD Model data Ver 0.62"

SIZE_DFILE_HEADER = 556
SIZE_DFILE_OBJINFO = 40
SIZE_OBJ3D = 2236
SIZE_VERTEX = 24
SIZE_FACE = 36
SIZE_TEXLINK = 32
SIZE_TM_ROT = 20
SIZE_TM_POS = 16
SIZE_TM_SCALE = 16
SIZE_FMATRIX = 64

FONE = 256.0
MAX_SEGMENTS = 32

O_NVERTEX = 84
O_NFACE = 88
O_NTEXLINK = 92

O_NODENAME = 172
O_NODEPARENT = 204

O_TM = 240
O_TMINVERT = 304
O_TMROTATE = 432

O_PX = 656

O_TMROTCNT = 684
O_TMPOSCNT = 688
O_TMSCALECNT = 692

O_TMROTFRAME = 696
O_TMPOSFRAME = 1208
O_TMSCALEFRAME = 1720
O_TMFRAMECNT = 2232

MODELINFO_SIZE = 95268
MOTION_INFO_MAX = 512
MOTION_TOOL_MAX = 104
MOTION_SKIL_MAX = 8
CHRMOTION_EXT = 10
MOTIONINFO_SIZE = 172

OFF_MOTIONINFO = 0x18C
OFF_MOTIONCOUNT = OFF_MOTIONINFO + MOTION_INFO_MAX * MOTIONINFO_SIZE
OFF_FILETYPEKEYWORD = OFF_MOTIONCOUNT + 4
OFF_LINKFILEKEYWORD = OFF_FILETYPEKEYWORD + 4
OFF_LINKFILE = OFF_LINKFILEKEYWORD + 4

STATE_NAMES = {
    0x040: "STAND",
    0x050: "WALK",
    0x060: "RUN",
    0x080: "FALLDOWN",
    0x100: "ATTACK",
    0x110: "DAMAGE",
    0x120: "DEAD",
    0x130: "SOMETIME",
    0x140: "EAT",
    0x150: "SKILL",
    0x170: "FALLSTAND",
    0x180: "FALLDAMAGE",
    0x200: "RESTART",
    0x210: "WARP",
    0x220: "YAHOO",
    0x300: "HAMMER",
}


# ---------------------------------------------------------------------------
# Binary helpers
# ---------------------------------------------------------------------------

def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def dword(v):
    return v & 0xFFFFFFFF


def matrix_i32(data, off):
    return list(struct.unpack_from("<16i", data, off))


def matrix_f32(data, off):
    return list(struct.unpack_from("<16f", data, off))


def fixed_matrix_to_float(m):
    return [v / FONE for v in m]


def identity():
    return [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    ]


def matmul(a, b):
    # Exact indexing convention used by smFMatrixMult:
    # pM[4*j+i] += pA[4*j+k] * pB[4*k+i]
    m = [0.0] * 16
    for i in range(4):
        for j in range(4):
            acc = 0.0
            for k in range(4):
                acc += a[4 * j + k] * b[4 * k + i]
            m[4 * j + i] = acc
    return m


def matrix_translation(m):
    # _41, _42, _43
    return (m[12], m[13], m[14])


def det3(m):
    return (
        m[0] * (m[5] * m[10] - m[6] * m[9])
        - m[1] * (m[4] * m[10] - m[6] * m[8])
        + m[2] * (m[4] * m[9] - m[5] * m[8])
    )


def finite_matrix(m):
    return all(math.isfinite(v) for v in m)


def affine_error(m):
    return max(
        abs(m[3]),
        abs(m[7]),
        abs(m[11]),
        abs(m[15] - 1.0),
    )


# ---------------------------------------------------------------------------
# Source GetSpeedSum / INX decode
# ---------------------------------------------------------------------------

def get_speed_sum(text):
    raw = text.encode("latin-1")
    sum2 = 0
    dwsum = 0
    cnt = 0

    for ch in raw:
        if ch == 0:
            break

        if ord("a") <= ch <= ord("z"):
            value = ch - 0x20
        else:
            value = ch

        sum2 = dword(sum2 + value * (cnt + 1))
        dwsum = dword(dwsum + value * (cnt * cnt))
        cnt += 1

    return dword((dwsum << 24) | (cnt << 16) | sum2)


def reconstruct_codes(file_type_kw, link_kw, has_link):
    if has_link:
        file_code = dword(
            (link_kw & 0xFF000000)
            | ((file_type_kw & 0xFF000000) >> 8)
            | (link_kw & 0x0000FF00)
            | ((file_type_kw & 0x0000FF00) >> 8)
        )

        link_code = dword(
            ((file_type_kw & 0x000000FF) << 24)
            | ((link_kw & 0x000000FF) << 16)
            | ((link_kw & 0x00FF0000) >> 8)
            | ((file_type_kw & 0x00FF0000) >> 16)
        )

        return file_code, link_code

    file_code = dword(
        ((file_type_kw & 0xFF000000) >> 16)
        | ((file_type_kw & 0x00FF0000) << 8)
        | ((file_type_kw & 0x0000FF00) >> 8)
        | ((file_type_kw & 0x000000FF) << 16)
    )
    return file_code, None


def decode_start(raw_kw, raw_frame, code):
    if not (raw_kw or raw_frame):
        return 0, True, 0

    keyword = dword(
        (raw_kw & 0xFF000000)
        | ((raw_kw & 0x0000FF00) << 8)
        | (raw_frame & 0x0000FF00)
        | ((raw_frame & 0xFF000000) >> 24)
    )

    frame = dword(
        ((raw_frame & 0x000000FF) << 24)
        | (raw_frame & 0x00FF0000)
        | ((raw_kw & 0x00FF0000) >> 8)
        | (raw_kw & 0x000000FF)
    )

    return frame, keyword == code, keyword


def decode_end(raw_kw, raw_frame, code):
    if not (raw_kw or raw_frame):
        return 0, True, 0

    keyword = dword(
        ((raw_frame & 0x0000FF00) << 16)
        | ((raw_frame & 0xFF000000) >> 8)
        | (raw_kw & 0x0000FF00)
        | ((raw_kw & 0xFF000000) >> 24)
    )

    frame = dword(
        ((raw_kw & 0x00FF0000) << 8)
        | ((raw_kw & 0x000000FF) << 16)
        | ((raw_frame & 0x00FF0000) >> 8)
        | (raw_frame & 0x000000FF)
    )

    return frame, keyword == code, keyword


def parse_motion_info(inx_data, index, decode_code):
    off = OFF_MOTIONINFO + index * MOTIONINFO_SIZE

    state = u32(inx_data, off + 0x00)
    kw1 = u32(inx_data, off + 0x04)
    raw_start = u32(inx_data, off + 0x08)
    kw2 = u32(inx_data, off + 0x0C)
    raw_end = u32(inx_data, off + 0x10)

    repeat = u32(inx_data, off + 0xA0)
    key_code = inx_data[off + 0xA4]
    motion_frame = i32(inx_data, off + 0xA8)

    if index >= CHRMOTION_EXT:
        start, start_ok, start_keyword = decode_start(
            kw1, raw_start, decode_code
        )
        end, end_ok, end_keyword = decode_end(
            kw2, raw_end, decode_code
        )
    else:
        start = raw_start
        end = raw_end
        start_ok = True
        end_ok = True
        start_keyword = 0
        end_keyword = 0

    return {
        "index": index,
        "state": state,
        "state_name": STATE_NAMES.get(state, f"UNKNOWN_0x{state:08X}"),
        "start": start,
        "end": end,
        "start_ok": start_ok,
        "end_ok": end_ok,
        "start_keyword": start_keyword,
        "end_keyword": end_keyword,
        "repeat": repeat,
        "key_code": key_code,
        "motion_frame": motion_frame,
    }


def read_inx_motion(path, logical_path, index, smb_header_frames):
    data = path.read_bytes()

    if len(data) != MODELINFO_SIZE:
        raise RuntimeError(
            f"unexpected INX size={len(data)}, expected={MODELINFO_SIZE}"
        )

    motion_count = u32(data, OFF_MOTIONCOUNT)
    if not 0 <= index < motion_count:
        raise RuntimeError(
            f"motion index {index} outside MotionCount={motion_count}"
        )

    file_kw = u32(data, OFF_FILETYPEKEYWORD)
    link_kw = u32(data, OFF_LINKFILEKEYWORD)
    link_file = cstr(data[OFF_LINKFILE:OFF_LINKFILE + 64])
    embedded_code, _ = reconstruct_codes(file_kw, link_kw, bool(link_file))
    logical_code = get_speed_sum(logical_path)

    if embedded_code != logical_code:
        raise RuntimeError(
            f"logical path code mismatch: embedded=0x{embedded_code:08X} "
            f"calculated=0x{logical_code:08X}"
        )

    motion = parse_motion_info(data, index, embedded_code)

    if not motion["start_ok"] or not motion["end_ok"]:
        raise RuntimeError(
            f"motion[{index}] keyword decode validation failed"
        )

    mf = motion["motion_frame"]
    if mf <= 0:
        raise RuntimeError(
            f"motion[{index}] MotionFrame={mf} has no SMB container"
        )

    slot = mf - 1
    if not 0 <= slot < len(smb_header_frames):
        raise RuntimeError(
            f"motion[{index}] MotionFrame={mf} -> slot={slot} "
            f"outside TmFrameCounter={len(smb_header_frames)}"
        )

    container = smb_header_frames[slot]
    base_logical = container["start"] // 160

    motion["slot"] = slot
    motion["global_start"] = motion["start"] + base_logical
    motion["global_end"] = motion["end"] + base_logical
    motion["engine_start"] = motion["global_start"] * 160
    motion["engine_end"] = motion["global_end"] * 160

    if motion["engine_end"] > container["end"]:
        raise RuntimeError(
            f"motion[{index}] engine_end={motion['engine_end']} "
            f"> container_end={container['end']}"
        )

    return motion, motion_count


# ---------------------------------------------------------------------------
# SMB parsing
# ---------------------------------------------------------------------------

def parse_frame_table(raw, off, count):
    out = []
    for slot in range(count):
        start, end, pos_num, pos_cnt = struct.unpack_from(
            "<4i", raw, off + slot * 16
        )
        out.append({
            "slot": slot,
            "start": start,
            "end": end,
            "pos_num": pos_num,
            "pos_cnt": pos_cnt,
        })
    return out


def parse_smb(path):
    data = path.read_bytes()

    if len(data) < SIZE_DFILE_HEADER:
        raise RuntimeError("SMB too small")
    if not data.startswith(SMD_HEADER):
        raise RuntimeError("invalid SMB header")

    obj_count = i32(data, 24)
    mat_count = i32(data, 28)
    tm_frame_count = i32(data, 40)

    if not 0 <= tm_frame_count <= MAX_SEGMENTS:
        raise RuntimeError(f"invalid TmFrameCounter={tm_frame_count}")

    header_frames = []
    for slot in range(tm_frame_count):
        start, end, pos_num, pos_cnt = struct.unpack_from(
            "<4i", data, 44 + slot * 16
        )
        header_frames.append({
            "slot": slot,
            "start": start,
            "end": end,
            "pos_num": pos_num,
            "pos_cnt": pos_cnt,
        })

    infos = []
    for idx in range(obj_count):
        off = SIZE_DFILE_HEADER + idx * SIZE_DFILE_OBJINFO
        raw_info = data[off:off + SIZE_DFILE_OBJINFO]
        infos.append({
            "index": idx,
            "name": cstr(raw_info[:32]),
            "length": i32(raw_info, 32),
            "start": i32(raw_info, 36),
        })

    bones = []

    for info in infos:
        start = info["start"]
        raw = data[start:start + SIZE_OBJ3D]
        if len(raw) != SIZE_OBJ3D:
            raise RuntimeError(
                f"object[{info['index']}] truncated fixed smOBJ3D"
            )

        name = cstr(raw[O_NODENAME:O_NODENAME + 32])
        parent = cstr(raw[O_NODEPARENT:O_NODEPARENT + 32])

        if name != info["name"]:
            raise RuntimeError(
                f"object table/name mismatch {info['name']!r} != {name!r}"
            )

        nv = i32(raw, O_NVERTEX)
        nf = i32(raw, O_NFACE)
        nt = i32(raw, O_NTEXLINK)

        nr = i32(raw, O_TMROTCNT)
        np = i32(raw, O_TMPOSCNT)
        ns = i32(raw, O_TMSCALECNT)
        nseg = i32(raw, O_TMFRAMECNT)

        if min(nv, nf, nt, nr, np, ns, nseg) < 0:
            raise RuntimeError(f"{name}: negative serialized count")
        if nseg > MAX_SEGMENTS:
            raise RuntimeError(f"{name}: TmFrameCnt={nseg} > {MAX_SEGMENTS}")

        tm_i = matrix_i32(raw, O_TM)
        tminv_i = matrix_i32(raw, O_TMINVERT)
        tmrotate_i = matrix_i32(raw, O_TMROTATE)
        px, py, pz = struct.unpack_from("<3i", raw, O_PX)

        cursor = start + SIZE_OBJ3D
        cursor += nv * SIZE_VERTEX
        cursor += nf * SIZE_FACE
        cursor += nt * SIZE_TEXLINK

        rot = []
        for k in range(nr):
            off = cursor + k * SIZE_TM_ROT
            frame, x, y, z, w = struct.unpack_from("<i4f", data, off)
            rot.append((frame, x, y, z, w))
        cursor += nr * SIZE_TM_ROT

        pos = []
        for k in range(np):
            off = cursor + k * SIZE_TM_POS
            frame, x, y, z = struct.unpack_from("<i3f", data, off)
            pos.append((frame, x, y, z))
        cursor += np * SIZE_TM_POS

        scale = []
        for k in range(ns):
            off = cursor + k * SIZE_TM_SCALE
            frame, x, y, z = struct.unpack_from("<4i", data, off)
            scale.append((frame, x, y, z))
        cursor += ns * SIZE_TM_SCALE

        prevrot = []
        for k in range(nr):
            off = cursor + k * SIZE_FMATRIX
            prevrot.append(matrix_f32(data, off))
        cursor += nr * SIZE_FMATRIX

        expected_end = start + info["length"]
        if cursor != expected_end:
            raise RuntimeError(
                f"{name}: serialized end={cursor}, expected={expected_end}"
            )

        bones.append({
            "index": info["index"],
            "name": name,
            "parent_name": parent,
            "parent_index": None,
            "tm": fixed_matrix_to_float(tm_i),
            "tminvert": fixed_matrix_to_float(tminv_i),
            "tmrotate": fixed_matrix_to_float(tmrotate_i),
            "px": px,
            "py": py,
            "pz": pz,
            "rot": rot,
            "pos": pos,
            "scale": scale,
            "prevrot": prevrot,
            "rot_frames": parse_frame_table(raw, O_TMROTFRAME, nseg),
            "pos_frames": parse_frame_table(raw, O_TMPOSFRAME, nseg),
            "scale_frames": parse_frame_table(raw, O_TMSCALEFRAME, nseg),
            "tm_frame_count": nseg,
        })

    if mat_count != 0:
        raise RuntimeError(
            f"expected skeleton MatCounter=0, got {mat_count}"
        )

    by_name = {bone["name"]: bone["index"] for bone in bones}
    if len(by_name) != len(bones):
        raise RuntimeError("duplicate NodeName in skeleton")

    roots = []
    for bone in bones:
        parent_name = bone["parent_name"]
        if parent_name:
            if parent_name not in by_name:
                raise RuntimeError(
                    f"{bone['name']}: missing parent {parent_name!r}"
                )
            bone["parent_index"] = by_name[parent_name]
        else:
            roots.append(bone["index"])

    # Reproduce LinkObject() TmSort construction:
    # 1) append roots in obj3d serial order
    # 2) walk sorted parents and append their children in serial order
    tm_sort = list(roots)
    cursor = 0
    while cursor < len(tm_sort):
        parent_index = tm_sort[cursor]
        for bone in bones:
            if bone["parent_index"] == parent_index:
                tm_sort.append(bone["index"])
        cursor += 1

    if len(tm_sort) != len(bones):
        raise RuntimeError(
            f"TmSort did not cover skeleton: {len(tm_sort)}/{len(bones)}"
        )
    if len(set(tm_sort)) != len(tm_sort):
        raise RuntimeError("TmSort contains duplicate object indices")

    sort_pos = {bone_index: pos for pos, bone_index in enumerate(tm_sort)}
    parent_order_errors = []
    for bone in bones:
        p = bone["parent_index"]
        if p is not None and sort_pos[p] >= sort_pos[bone["index"]]:
            parent_order_errors.append(
                f"{bone['name']} parent={bones[p]['name']}"
            )

    if parent_order_errors:
        raise RuntimeError(
            "TmSort is not parent-first: "
            + "; ".join(parent_order_errors[:10])
        )

    return {
        "data_size": len(data),
        "obj_count": obj_count,
        "tm_frame_count": tm_frame_count,
        "header_frames": header_frames,
        "bones": bones,
        "roots": roots,
        "tm_sort": tm_sort,
    }


# ---------------------------------------------------------------------------
# Source transform evaluator
# ---------------------------------------------------------------------------

def get_tm_frame(segments, frame):
    for seg in segments:
        if (
            seg["pos_cnt"] > 0
            and seg["start"] <= frame
            and seg["end"] > frame
        ):
            return seg["pos_num"], seg["slot"]
    return -1, None


def quaternion_slerp_source(
    ax, ay, az, aw,
    bx, by, bz, bw,
    alpha,
):
    # Literal D3DMath_QuaternionSlerp, including the legacy Ax=-Az typo.
    cos_theta = ax * bx + ay * by + az * bz + aw * bw

    if cos_theta < 0.0:
        ax = -ax
        ay = -ay
        ax = -az  # literal source behavior
        aw = -aw
        cos_theta = -cos_theta

    if cos_theta + 1.0 > 0.05:
        if 1.0 - cos_theta < 0.05:
            scale1 = 1.0 - alpha
            scale2 = alpha
        else:
            theta = math.acos(cos_theta)
            sin_theta = math.sin(theta)
            scale1 = math.sin(theta * (1.0 - alpha)) / sin_theta
            scale2 = math.sin(theta * alpha) / sin_theta
    else:
        bx = -ay
        by = ax
        bz = -aw
        bw = az
        scale1 = math.sin(math.pi * (0.5 - alpha))
        scale2 = math.sin(math.pi * alpha)

    return (
        scale1 * ax + scale2 * bx,
        scale1 * ay + scale2 * by,
        scale1 * az + scale2 * bz,
        scale1 * aw + scale2 * bw,
    )


def matrix_from_quaternion(x, y, z, w):
    xx = x * x
    yy = y * y
    zz = z * z
    xy = x * y
    xz = x * z
    yz = y * z
    wx = w * x
    wy = w * y
    wz = w * z

    return [
        1.0 - 2.0 * (yy + zz),
        2.0 * (xy - wz),
        2.0 * (xz + wy),
        0.0,

        2.0 * (xy + wz),
        1.0 - 2.0 * (xx + zz),
        2.0 * (yz - wx),
        0.0,

        2.0 * (xz - wy),
        2.0 * (yz + wx),
        1.0 - 2.0 * (xx + yy),
        0.0,

        0.0, 0.0, 0.0, 1.0,
    ]


def get_rot_frame(bone, base, frame, frame_info_end, stats):
    gmat = identity()
    rot = bone["rot"]
    prevrot = bone["prevrot"]

    if not 0 <= base < len(rot):
        raise RuntimeError(
            f"{bone['name']}: rotation base={base} outside {len(rot)}"
        )

    if rot[base][0] > frame:
        stats["rot_before_first"] += 1
        return gmat

    cnt = 0
    while True:
        i = base + cnt
        if i + 1 >= len(rot):
            raise RuntimeError(
                f"{bone['name']}: GetRotFrame search exceeded track "
                f"at frame={frame}, base={base}, i={i}"
            )

        s = rot[i][0]
        e = rot[i + 1][0]

        if s <= frame and e > frame:
            break

        cnt += 1
        stats["rot_search_steps"] += 1

    if frame_info_end is not None and e > frame_info_end:
        stats["rot_end_clamps"] += 1
        return list(prevrot[base + cnt])

    alpha = float(frame - s) / float(e - s)

    # Source reads A=rot[i], but does not use it in the call.
    _, bx, by, bz, bw = rot[i + 1]

    qx, qy, qz, qw = quaternion_slerp_source(
        0.0, 0.0, 0.0, 0.0,
        bx, by, bz, bw,
        alpha,
    )

    delta = matrix_from_quaternion(qx, qy, qz, qw)
    return matmul(prevrot[base + cnt], delta)


def get_pos_frame(bone, base, frame, current_xyz, stats):
    pos = bone["pos"]

    if not 0 <= base < len(pos):
        raise RuntimeError(
            f"{bone['name']}: position base={base} outside {len(pos)}"
        )

    if pos[base][0] > frame:
        stats["pos_before_first"] += 1
        return current_xyz

    cnt = 0
    while True:
        i = base + cnt
        if i + 1 >= len(pos):
            raise RuntimeError(
                f"{bone['name']}: GetPosFrame search exceeded track "
                f"at frame={frame}, base={base}, i={i}"
            )

        s = pos[i][0]
        e = pos[i + 1][0]

        if s <= frame and e > frame:
            break

        cnt += 1
        stats["pos_search_steps"] += 1

    alpha = float(frame - s) / float(e - s)

    _, x0, y0, z0 = pos[i]
    _, x1, y1, z1 = pos[i + 1]

    return (
        x0 + (x1 - x0) * alpha,
        y0 + (y1 - y0) * alpha,
        z0 + (z1 - z0) * alpha,
    )


def get_scale_frame(bone, base, frame, current_xyz, stats):
    scale = bone["scale"]

    if not 0 <= base < len(scale):
        raise RuntimeError(
            f"{bone['name']}: scale base={base} outside {len(scale)}"
        )

    if scale[base][0] > frame:
        stats["scale_before_first"] += 1
        return current_xyz

    cnt = 0
    while True:
        i = base + cnt
        if i + 1 >= len(scale):
            raise RuntimeError(
                f"{bone['name']}: GetScaleFrame search exceeded track "
                f"at frame={frame}, base={base}, i={i}"
            )

        s = scale[i][0]
        e = scale[i + 1][0]

        if s <= frame and e > frame:
            break

        cnt += 1
        stats["scale_search_steps"] += 1

    ch = e - s
    sh = frame - s
    alpha = (sh << 8) // ch

    _, x0, y0, z0 = scale[i]
    _, x1, y1, z1 = scale[i + 1]

    return (
        x0 + (((x1 - x0) * alpha) >> 8),
        y0 + (((y1 - y0) * alpha) >> 8),
        z0 + (((z1 - z0) * alpha) >> 8),
    )


def evaluate_bone_local(bone, parent_bone, frame, frame_info_end, stats):
    qmat = identity()

    num_rot, rot_slot = get_tm_frame(bone["rot_frames"], frame)
    num_pos, pos_slot = get_tm_frame(bone["pos_frames"], frame)
    num_scale, scale_slot = get_tm_frame(bone["scale_frames"], frame)

    # Literal source condition includes NumTmScale > 0 rather than >= 0.
    if bone["tm_frame_count"]:
        has_animation = (
            num_rot >= 0
            or num_pos >= 0
            or num_scale > 0
        )
    else:
        has_animation = (
            len(bone["rot"]) > 0
            or len(bone["pos"]) > 0
            or len(bone["scale"]) > 0
        )

    if has_animation:
        if bone["tm_frame_count"]:
            if bone["rot"] and num_rot >= 0:
                qmat = get_rot_frame(
                    bone, num_rot, frame, frame_info_end, stats
                )
                stats["active_rot"] += 1
            else:
                qmat = list(bone["tmrotate"])

            if bone["scale"] and num_scale >= 0:
                sx, sy, sz = get_scale_frame(
                    bone, num_scale, frame,
                    (256, 256, 256), stats
                )
                fmat = identity()
                fmat[0] = sx / FONE
                fmat[5] = sy / FONE
                fmat[10] = sz / FONE
                qmat = matmul(qmat, fmat)
                stats["active_scale"] += 1

            if bone["pos"] and num_pos >= 0:
                x, y, z = get_pos_frame(
                    bone, num_pos, frame,
                    matrix_translation(qmat), stats
                )
                qmat[12] = x
                qmat[13] = y
                qmat[14] = z
                stats["active_pos"] += 1
            else:
                qmat[12] = bone["px"] / FONE
                qmat[13] = bone["py"] / FONE
                qmat[14] = bone["pz"] / FONE
        else:
            # Included for completeness; m5.smb uses TmFrameCnt=7.
            if bone["rot"] and bone["rot"][-1][0] > frame:
                qmat = get_rot_frame(
                    bone, 0, frame, frame_info_end, stats
                )
            else:
                qmat = list(bone["tmrotate"])

            if bone["scale"] and bone["scale"][-1][0] > frame:
                sx, sy, sz = get_scale_frame(
                    bone, 0, frame,
                    (256, 256, 256), stats
                )
                fmat = identity()
                fmat[0] = sx / FONE
                fmat[5] = sy / FONE
                fmat[10] = sz / FONE
                qmat = matmul(qmat, fmat)

            if bone["pos"] and bone["pos"][-1][0] > frame:
                x, y, z = get_pos_frame(
                    bone, 0, frame,
                    matrix_translation(qmat), stats
                )
                qmat[12] = x
                qmat[13] = y
                qmat[14] = z
            else:
                qmat[12] = bone["px"] / FONE
                qmat[13] = bone["py"] / FONE
                qmat[14] = bone["pz"] / FONE
    else:
        stats["bind_local_fallback"] += 1
        if parent_bone is not None:
            qmat = matmul(bone["tm"], parent_bone["tminvert"])
        else:
            qmat = list(bone["tm"])

    return qmat, {
        "rot_slot": rot_slot,
        "pos_slot": pos_slot,
        "scale_slot": scale_slot,
        "has_animation": has_animation,
    }


def evaluate_pose(smb, frame, frame_info_end):
    bones = smb["bones"]
    results = [None] * len(bones)
    local_results = [None] * len(bones)
    local_meta = [None] * len(bones)

    stats = {
        "active_rot": 0,
        "active_pos": 0,
        "active_scale": 0,
        "bind_local_fallback": 0,
        "rot_before_first": 0,
        "pos_before_first": 0,
        "scale_before_first": 0,
        "rot_end_clamps": 0,
        "rot_search_steps": 0,
        "pos_search_steps": 0,
        "scale_search_steps": 0,
    }

    for idx in smb["tm_sort"]:
        bone = bones[idx]
        pidx = bone["parent_index"]
        parent_bone = bones[pidx] if pidx is not None else None

        local, meta = evaluate_bone_local(
            bone, parent_bone, frame, frame_info_end, stats
        )

        if pidx is not None:
            if results[pidx] is None:
                raise RuntimeError(
                    f"{bone['name']}: parent result not evaluated"
                )
            result = matmul(local, results[pidx])
        else:
            result = list(local)

        local_results[idx] = local
        local_meta[idx] = meta
        results[idx] = result

    return results, local_results, local_meta, stats


# ---------------------------------------------------------------------------
# Diagnostics
# ---------------------------------------------------------------------------

def bbox(points):
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    zs = [p[2] for p in points]
    return (
        min(xs), max(xs),
        min(ys), max(ys),
        min(zs), max(zs),
    )


def dist(a, b):
    return math.sqrt(
        (a[0] - b[0]) ** 2
        + (a[1] - b[1]) ** 2
        + (a[2] - b[2]) ** 2
    )


def fmt_vec(v):
    return f"({v[0]:.5f},{v[1]:.5f},{v[2]:.5f})"


def keycode_text(value):
    if 32 <= value <= 126:
        return repr(chr(value))
    return str(value)
# ---------------------------------------------------------------------------
# Blender bake layer
# ---------------------------------------------------------------------------

try:
    import bpy
    from mathutils import Matrix
except ImportError:
    bpy = None
    Matrix = None


def row_engine_to_blender_matrix(m):
    # Engine:
    #   row vector, translation in _41/_42/_43.
    #
    # Blender:
    #   column vector, translation in final column.
    #
    # Column representation of the same affine transform is transpose(M).
    return Matrix((
        (m[0],  m[4],  m[8],  m[12]),
        (m[1],  m[5],  m[9],  m[13]),
        (m[2],  m[6],  m[10], m[14]),
        (m[3],  m[7],  m[11], m[15]),
    ))


def matrix_max_abs_diff(a, b):
    return max(
        abs(a[r][c] - b[r][c])
        for r in range(4)
        for c in range(4)
    )


def evaluated_mesh_bbox(objects):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    pts = []

    for obj in objects:
        eval_obj = obj.evaluated_get(depsgraph)
        mesh = eval_obj.to_mesh()

        try:
            mw = eval_obj.matrix_world
            for v in mesh.vertices:
                co = mw @ v.co
                pts.append((float(co.x), float(co.y), float(co.z)))
        finally:
            eval_obj.to_mesh_clear()

    if not pts:
        return None

    return bbox(pts)


def blender_main():
    if bpy is None:
        raise RuntimeError("This script must run inside Blender")

    ap = argparse.ArgumentParser()
    ap.add_argument("--skeleton", required=True, type=Path)
    ap.add_argument("--inx", required=True, type=Path)
    ap.add_argument("--logical-path", required=True)
    ap.add_argument("--motion-index", type=int, default=10)
    ap.add_argument("--armature", default="PT_m5_Armature")
    ap.add_argument("--out", required=True, type=Path)
    import sys
    raw_argv = sys.argv
    script_argv = raw_argv[raw_argv.index("--") + 1:] if "--" in raw_argv else []
    args = ap.parse_args(script_argv)

    arm_obj = bpy.data.objects.get(args.armature)
    if arm_obj is None:
        raise RuntimeError(
            f"armature {args.armature!r} not found in loaded .blend"
        )
    if arm_obj.type != 'ARMATURE':
        raise RuntimeError(
            f"{args.armature!r} exists but is not an ARMATURE"
        )

    smb = parse_smb(args.skeleton)
    motion, motion_count = read_inx_motion(
        args.inx,
        args.logical_path,
        args.motion_index,
        smb["header_frames"],
    )

    if len(arm_obj.data.bones) != len(smb["bones"]):
        raise RuntimeError(
            f"Blender armature bones={len(arm_obj.data.bones)} "
            f"SMB bones={len(smb['bones'])}"
        )

    missing_blender_bones = [
        bone["name"]
        for bone in smb["bones"]
        if arm_obj.data.bones.get(bone["name"]) is None
    ]
    if missing_blender_bones:
        raise RuntimeError(
            "SMB bones absent from Blender armature: "
            + repr(missing_blender_bones[:20])
        )

    print("PT_BLENDER_APPLY_MOTION_V1")
    print(f"blend_input={bpy.data.filepath}")
    print(f"skeleton={args.skeleton}")
    print(f"inx={args.inx}")
    print(f"logical_path={args.logical_path!r}")
    print(f"armature={arm_obj.name}")
    print()

    print("MOTION")
    print(f"MotionCount={motion_count}")
    print(f"index={motion['index']}")
    print(
        f"State=0x{motion['state']:08X} "
        f"name={motion['state_name']}"
    )
    print(f"MotionFrame={motion['motion_frame']}")
    print(f"TmFrame_slot={motion['slot']}")
    print(
        f"global_logical={motion['global_start']}.."
        f"{motion['global_end']}"
    )
    print(
        f"engine_half_open=[{motion['engine_start']},"
        f"{motion['engine_end']})"
    )
    print(f"Repeat={motion['repeat']}")
    print()

    logical_frames = list(
        range(motion["global_start"], motion["global_end"])
    )
    if not logical_frames:
        raise RuntimeError("selected motion has empty runtime interval")

    engine_frames = [f * 160 for f in logical_frames]
    blender_frames = list(range(1, len(engine_frames) + 1))

    scene = bpy.context.scene
    scene.frame_start = blender_frames[0]
    scene.frame_end = blender_frames[-1]

    # We are baking exact samples at every logical engine frame.
    old_interp = None
    edit_prefs = getattr(bpy.context.preferences, "edit", None)
    if edit_prefs is not None and hasattr(
        edit_prefs, "keyframe_new_interpolation_type"
    ):
        old_interp = edit_prefs.keyframe_new_interpolation_type
        edit_prefs.keyframe_new_interpolation_type = 'LINEAR'

    arm_obj.animation_data_clear()

    for pb in arm_obj.pose.bones:
        pb.rotation_mode = 'QUATERNION'

    visible_meshes = [
        obj for obj in bpy.data.objects
        if (
            obj.type == 'MESH'
            and int(obj.get("pt_inx_selected", 0)) == 1
        )
    ]

    if not visible_meshes:
        # Fallback for older static files lacking the selection property.
        visible_meshes = [
            obj for obj in bpy.data.objects
            if obj.type == 'MESH' and not obj.hide_render
        ]

    print("BAKE")
    print(f"blender_frames={blender_frames[0]}..{blender_frames[-1]}")
    print(f"engine_samples={engine_frames[0]}..{engine_frames[-1]}")
    print(f"sample_step_engine=160")
    print(f"bones={len(smb['bones'])}")
    print(f"visible_meshes={[obj.name for obj in visible_meshes]!r}")

    max_assignment_error = 0.0
    max_assignment_where = None
    nonfinite_engine_matrices = 0

    sample_blender_frames = sorted(set([
        blender_frames[0],
        blender_frames[(len(blender_frames) - 1) // 2],
        blender_frames[-1],
    ]))

    mesh_bbox_samples = {}

    try:
        for bf, ef in zip(blender_frames, engine_frames):
            scene.frame_set(bf)

            results, local_results, local_meta, stats = evaluate_pose(
                smb, ef, motion["engine_end"]
            )

            targets = {}

            # Source order is parent-first. Keep the same order here.
            for idx in smb["tm_sort"]:
                bone = smb["bones"][idx]
                name = bone["name"]

                engine_rest_col = row_engine_to_blender_matrix(
                    bone["tm"]
                )
                engine_pose_col = row_engine_to_blender_matrix(
                    results[idx]
                )

                if not finite_matrix(results[idx]):
                    nonfinite_engine_matrices += 1

                # Mesh was reconstructed into source bind/world pose:
                #
                #   p_bind = p_local * source_rest
                #   p_pose = p_local * source_pose
                #
                # Therefore row-vector deformation is:
                #
                #   D_row = inverse(source_rest) * source_pose
                #
                # and in Blender column convention:
                #
                #   D_col = source_pose^T * inverse(source_rest^T)
                #
                # The Armature modifier applies:
                #
                #   pose_bone.matrix * bone.matrix_local^-1
                #
                # so choose:
                #
                #   pose_bone.matrix = D_col * blender_rest
                #
                # This makes deformation independent of our display-only
                # bone tail/roll approximation.
                deform_col = (
                    engine_pose_col
                    @ engine_rest_col.inverted()
                )

                blender_rest = arm_obj.data.bones[name].matrix_local.copy()
                target = deform_col @ blender_rest
                targets[name] = target

                arm_obj.pose.bones[name].matrix = target

            bpy.context.view_layer.update()

            for idx in smb["tm_sort"]:
                name = smb["bones"][idx]["name"]
                pb = arm_obj.pose.bones[name]

                err = matrix_max_abs_diff(pb.matrix, targets[name])
                if err > max_assignment_error:
                    max_assignment_error = err
                    max_assignment_where = (bf, ef, name)

                pb.keyframe_insert(data_path="location", frame=bf)
                pb.keyframe_insert(
                    data_path="rotation_quaternion",
                    frame=bf
                )
                pb.keyframe_insert(data_path="scale", frame=bf)

            bpy.context.view_layer.update()

            if bf in sample_blender_frames:
                bb = evaluated_mesh_bbox(visible_meshes)
                mesh_bbox_samples[bf] = bb

                if bb is not None:
                    print(
                        f"frame={bf} engine={ef} "
                        f"visible_mesh_bbox="
                        f"x[{bb[0]:.4f},{bb[1]:.4f}] "
                        f"y[{bb[2]:.4f},{bb[3]:.4f}] "
                        f"z[{bb[4]:.4f},{bb[5]:.4f}]"
                    )

    finally:
        if old_interp is not None:
            edit_prefs.keyframe_new_interpolation_type = old_interp

    action = (
        arm_obj.animation_data.action
        if arm_obj.animation_data is not None
        else None
    )

    if action is None:
        raise RuntimeError("Blender did not create an Action while keyframing")

    action.name = (
        f"PT_m5_{motion['state_name']}_motion{motion['index']}"
    )

    scene.frame_set(blender_frames[0])
    bpy.context.view_layer.update()

    scene["pt_animation_source_smb"] = str(args.skeleton)
    scene["pt_animation_source_inx"] = str(args.inx)
    scene["pt_animation_motion_index"] = motion["index"]
    scene["pt_animation_state"] = motion["state_name"]
    scene["pt_animation_motion_frame"] = motion["motion_frame"]
    scene["pt_animation_engine_start"] = motion["engine_start"]
    scene["pt_animation_engine_end_exclusive"] = motion["engine_end"]
    scene["pt_animation_sample_step"] = 160
    scene["pt_animation_blender_start"] = blender_frames[0]
    scene["pt_animation_blender_end"] = blender_frames[-1]
    scene["pt_animation_rule"] = (
        "D_col=pose_engine_col@inverse(rest_engine_col); "
        "pose_bone.matrix=D_col@bone.matrix_local"
    )

    args.out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.out))

    finite_bboxes = all(
        bb is not None
        and all(math.isfinite(v) for v in bb)
        for bb in mesh_bbox_samples.values()
    )

    ok = (
        nonfinite_engine_matrices == 0
        and max_assignment_error < 1e-4
        and finite_bboxes
        and len(arm_obj.data.bones) == len(smb["bones"])
    )

    print()
    print("VALIDATION")
    print(f"action={action.name}")
    print(f"keyed_bones={len(smb['bones'])}")
    print(f"keyed_frames={len(blender_frames)}")
    print(f"nonfinite_engine_matrices={nonfinite_engine_matrices}")
    print(f"max_pose_matrix_assignment_error={max_assignment_error:.9g}")
    print(f"max_pose_matrix_assignment_where={max_assignment_where!r}")
    print(f"visible_mesh_bbox_samples={len(mesh_bbox_samples)}")
    print(f"visible_mesh_bboxes_finite={'YES' if finite_bboxes else 'NO'}")
    print(f"output={args.out}")
    print(
        "blender_motion_bake_check="
        + ("PASS" if ok else "FAIL")
    )


if __name__ == "__main__":
    blender_main()

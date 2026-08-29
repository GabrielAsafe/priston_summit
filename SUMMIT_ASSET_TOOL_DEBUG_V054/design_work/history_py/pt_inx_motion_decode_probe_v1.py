#!/usr/bin/env python3
# Priston Tale INX logical motion decoder V1.
# Read-only.
#
# Reproduces:
#   GetSpeedSum()
#   ModelKeyWordDecode() code reconstruction
#   MotionKeyWordDecode()
# and maps MotionFrame -> SMB TmFrame[MotionFrame-1].
#
# Does NOT modify INX/SMB and does NOT create Blender keyframes.

import argparse
import struct
from pathlib import Path

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

SMB_HEADER = b"SMD Model data Ver 0.62"
SMB_HEADER_SIZE = 556


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def dword(v):
    return v & 0xFFFFFFFF


def get_speed_sum(text):
    # Exact character.cpp logic, with DWORD truncation.
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


def change_ext_inx(path):
    # The source's ChangeFileExt() behavior needed here is simply the
    # logical path with its last extension replaced by ".inx".
    slash = max(path.rfind("\\"), path.rfind("/"))
    dot = path.rfind(".")
    if dot > slash:
        return path[:dot] + ".inx"
    return path + ".inx"


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


def parse_smb_frames(path):
    data = path.read_bytes()

    if len(data) < SMB_HEADER_SIZE:
        raise RuntimeError("SMB too small")
    if not data.startswith(SMB_HEADER):
        raise RuntimeError("invalid SMB header")

    count = i32(data, 40)
    if not 0 <= count <= 32:
        raise RuntimeError(f"invalid TmFrameCounter={count}")

    frames = []
    for idx in range(count):
        off = 44 + idx * 16
        start, end, pos_num, pos_cnt = struct.unpack_from("<4i", data, off)
        frames.append({
            "slot": idx,
            "start_fixed": start,
            "end_fixed": end,
            "start_logical": start // 160,
            "end_logical": end // 160,
            "raw_pos_num": pos_num,
            "raw_pos_cnt": pos_cnt,
        })

    return frames


def parse_motion(data, index, decode_code):
    off = OFF_MOTIONINFO + index * MOTIONINFO_SIZE

    state = u32(data, off + 0x00)
    kw1 = u32(data, off + 0x04)
    raw_start = u32(data, off + 0x08)
    kw2 = u32(data, off + 0x0C)
    raw_end = u32(data, off + 0x10)

    events = [
        u32(data, off + 0x14 + i * 4)
        for i in range(4)
    ]

    item_count = i32(data, off + 0x24)
    item_codes = bytes(data[off + 0x28:off + 0x28 + MOTION_TOOL_MAX])
    job_bits = u32(data, off + 0x90)
    skills = bytes(data[off + 0x94:off + 0x94 + MOTION_SKIL_MAX])
    map_position = i32(data, off + 0x9C)
    repeat = u32(data, off + 0xA0)
    key_code = data[off + 0xA4]
    motion_frame = i32(data, off + 0xA8)

    if index >= CHRMOTION_EXT:
        start, start_ok, start_keyword = decode_start(
            kw1, raw_start, decode_code
        )
        end, end_ok, end_keyword = decode_end(
            kw2, raw_end, decode_code
        )
    else:
        # MotionKeyWordDecode() explicitly starts at CHRMOTION_EXT.
        start = raw_start
        end = raw_end
        start_ok = True
        end_ok = True
        start_keyword = 0
        end_keyword = 0

    return {
        "index": index,
        "state": state,
        "raw_kw1": kw1,
        "raw_start": raw_start,
        "raw_kw2": kw2,
        "raw_end": raw_end,
        "start": start,
        "end": end,
        "start_ok": start_ok,
        "end_ok": end_ok,
        "start_keyword": start_keyword,
        "end_keyword": end_keyword,
        "events": events,
        "item_count": item_count,
        "item_codes_nonzero": [v for v in item_codes if v],
        "job_bits": job_bits,
        "skills_nonzero": [v for v in skills if v],
        "map_position": map_position,
        "repeat": repeat,
        "key_code": key_code,
        "motion_frame": motion_frame,
    }


def keycode_text(value):
    if 32 <= value <= 126:
        return repr(chr(value))
    return str(value)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--inx", required=True, type=Path)
    ap.add_argument("--skeleton", required=True, type=Path)
    ap.add_argument(
        "--logical-path",
        required=True,
        help=r"Logical Windows asset path, e.g. char\tmABCD\M5Bip.inx",
    )
    args = ap.parse_args()

    data = args.inx.read_bytes()

    print("PT_INX_MOTION_DECODE_PROBE_V1")
    print(f"inx={args.inx}")
    print(f"skeleton={args.skeleton}")
    print(f"logical_path={args.logical_path!r}")
    print(f"file_size={len(data)}")
    print()

    if len(data) != MODELINFO_SIZE:
        raise RuntimeError(
            f"unexpected INX size={len(data)}, expected={MODELINFO_SIZE}"
        )

    motion_count = u32(data, OFF_MOTIONCOUNT)
    file_type_kw = u32(data, OFF_FILETYPEKEYWORD)
    link_kw = u32(data, OFF_LINKFILEKEYWORD)
    link_file = cstr(data[OFF_LINKFILE:OFF_LINKFILE + 64])
    has_link = bool(link_file)

    print("ABI_LAYOUT")
    print(f"smMODELINFO_size={MODELINFO_SIZE}")
    print(f"smMOTIONINFO_size_assumed={MOTIONINFO_SIZE}")
    print(f"MotionInfo_offset=0x{OFF_MOTIONINFO:04X}")
    print(f"MotionCount_offset=0x{OFF_MOTIONCOUNT:05X}")
    print(f"MotionCount={motion_count}")
    print()

    if not CHRMOTION_EXT <= motion_count <= MOTION_INFO_MAX:
        raise RuntimeError(
            f"implausible MotionCount={motion_count}; "
            "smMOTIONINFO layout assumption likely wrong"
        )

    embedded_file_code, embedded_link_code = reconstruct_codes(
        file_type_kw, link_kw, has_link
    )

    logical_code = get_speed_sum(args.logical_path)

    print("MODEL_KEYWORD")
    print(f"FileTypeKeyWord=0x{file_type_kw:08X}")
    print(f"LinkFileKeyWord=0x{link_kw:08X}")
    print(f"szLinkFile={link_file!r}")
    print(f"embedded_file_code=0x{embedded_file_code:08X}")
    print(f"logical_path_code=0x{logical_code:08X}")
    print(
        "logical_path_code_match="
        + ("YES" if logical_code == embedded_file_code else "NO")
    )

    link_match = True
    if has_link:
        link_inx = change_ext_inx(link_file)
        calculated_link_code = get_speed_sum(link_inx)

        print(f"link_inx_path={link_inx!r}")
        print(f"embedded_link_code=0x{embedded_link_code:08X}")
        print(f"calculated_link_code=0x{calculated_link_code:08X}")
        link_match = calculated_link_code == embedded_link_code
        print(
            "link_path_code_match="
            + ("YES" if link_match else "NO")
        )

    print()

    # The embedded code is authoritative for reproducing the stored INX.
    decode_code = embedded_file_code

    smb_frames = parse_smb_frames(args.skeleton)

    print("SMB_TMFRAME")
    print(f"TmFrameCounter={len(smb_frames)}")
    for frame in smb_frames:
        print(
            f"slot={frame['slot']} "
            f"fixed={frame['start_fixed']}..{frame['end_fixed']} "
            f"logical={frame['start_logical']}..{frame['end_logical']}"
        )
    print()

    motions = [
        parse_motion(data, idx, decode_code)
        for idx in range(motion_count)
    ]

    decode_errors = []
    frame_errors = []

    print("MOTIONS")
    for m in motions:
        if not m["start_ok"]:
            decode_errors.append(
                f"motion[{m['index']}] start keyword "
                f"0x{m['start_keyword']:08X} != 0x{decode_code:08X}"
            )
        if not m["end_ok"]:
            decode_errors.append(
                f"motion[{m['index']}] end keyword "
                f"0x{m['end_keyword']:08X} != 0x{decode_code:08X}"
            )

        mf = m["motion_frame"]
        global_start = None
        global_end = None
        slot_text = "-"

        if mf > 0:
            slot = mf - 1
            if 0 <= slot < len(smb_frames):
                base = smb_frames[slot]["start_logical"]
                global_start = m["start"] + base
                global_end = m["end"] + base
                slot_text = str(slot)

                # Motion must fit inside its selected global container
                # after LoadCharactor's StartFrame adjustment.
                container_end = smb_frames[slot]["end_logical"]
                if global_end > container_end:
                    frame_errors.append(
                        f"motion[{m['index']}] global_end={global_end} "
                        f"> slot{slot}_end={container_end}"
                    )
            else:
                frame_errors.append(
                    f"motion[{m['index']}] MotionFrame={mf} "
                    f"outside TmFrameCounter={len(smb_frames)}"
                )

        event_offsets = [
            ev // 160 for ev in m["events"] if ev
        ]

        print(
            f"[{m['index']:03d}] "
            f"State=0x{m['state']:08X} "
            f"MotionFrame={mf} slot={slot_text} "
            f"local={m['start']}..{m['end']} "
            f"global="
            f"{global_start if global_start is not None else '-'}.."
            f"{global_end if global_end is not None else '-'} "
            f"Repeat={m['repeat']} "
            f"KeyCode={keycode_text(m['key_code'])} "
            f"EventOffsets={event_offsets} "
            f"JobBits=0x{m['job_bits']:08X} "
            f"ItemCount={m['item_count']} "
            f"SkillsNonZero={m['skills_nonzero']}"
        )

    print()

    print("VALIDATION")
    print(f"decode_errors={len(decode_errors)}")
    print(f"frame_errors={len(frame_errors)}")

    for err in decode_errors[:100]:
        print("DECODE_ERROR " + err)

    for err in frame_errors[:100]:
        print("FRAME_ERROR " + err)

    abi_consistent = (
        len(data) == MODELINFO_SIZE
        and CHRMOTION_EXT <= motion_count <= MOTION_INFO_MAX
    )

    code_consistent = (
        logical_code == embedded_file_code
        and link_match
    )

    decode_consistent = not decode_errors
    frame_consistent = not frame_errors

    print(f"abi_layout_consistent={'YES' if abi_consistent else 'NO'}")
    print(f"keyword_paths_consistent={'YES' if code_consistent else 'NO'}")
    print(f"motion_decode_consistent={'YES' if decode_consistent else 'NO'}")
    print(f"motion_ranges_consistent={'YES' if frame_consistent else 'NO'}")

    ok = (
        abi_consistent
        and code_consistent
        and decode_consistent
        and frame_consistent
    )

    print(
        "inx_motion_decode_check="
        + ("PASS" if ok else "FAIL")
    )


main()

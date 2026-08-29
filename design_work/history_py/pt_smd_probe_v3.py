#!/usr/bin/env python3
import argparse
import json
import struct
import sys
from pathlib import Path

SMD_HEADER_TEXT = b"SMD Model data Ver 0.62"

SIZE_DFILE_HEADER = 556
SIZE_DFILE_OBJINFO = 40
SIZE_MATERIAL_GROUP = 88
SIZE_MATERIAL = 320

# smMATERIAL_GROUP offsets, MinGW i686 ABI probe.
MG_OFF_SMATERIAL_PTR = 4
MG_OFF_MATERIAL_COUNT = 8
MG_OFF_REFORM_TEXTURE = 12
MG_OFF_MAX_MATERIAL = 16
MG_OFF_LAST_SEARCH_MATERIAL = 20
MG_OFF_LAST_SEARCH_NAME = 24

# smMATERIAL offsets, MinGW i686 ABI probe.
M_OFF_INUSE = 0
M_OFF_TEXTURE_COUNTER = 4
M_OFF_TEXTURE_PTRS = 8
M_OFF_TEXTURE_STAGE_STATE = 40
M_OFF_TEXTURE_FORM_STATE = 72
M_OFF_REFORM_TEXTURE = 104
M_OFF_MAP_OPACITY = 108
M_OFF_TEXTURE_TYPE = 112
M_OFF_BLEND_TYPE = 116
M_OFF_SHADE = 120
M_OFF_TWO_SIDE = 124
M_OFF_SERIAL_NUM = 128
M_OFF_DIFFUSE = 132
M_OFF_TRANSPARENCY = 144
M_OFF_SELF_ILLUM = 148
M_OFF_TEXTURE_SWAP = 152
M_OFF_MAT_FRAME = 156
M_OFF_TEXTURE_CLIP = 160
M_OFF_USE_STATE = 164
M_OFF_MESH_STATE = 168
M_OFF_WIND_MESH_BOTTOM = 172
M_OFF_ANIM_TEXTURE_PTRS = 176
M_OFF_ANIM_TEX_COUNTER = 304
M_OFF_FRAME_MASK = 308
M_OFF_SHIFT_FRAME_SPEED = 312
M_OFF_ANIMATION_FRAME = 316

BLEND_NAMES = {
    0: "SMMAT_BLEND_NONE",
    1: "SMMAT_BLEND_ALPHA",
    2: "SMMAT_BLEND_COLOR",
    3: "SMMAT_BLEND_SHADOW",
    4: "SMMAT_BLEND_LAMP",
    5: "SMMAT_BLEND_ADDCOLOR",
    6: "SMMAT_BLEND_INVSHADOW",
}

TEXTURE_TYPE_NAMES = {
    0: "SMTEX_TYPE_MULTIMIX",
    1: "SMTEX_TYPE_ANIMATION",
}


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def i32(data, off):
    return struct.unpack_from("<i", data, off)[0]


def f32(data, off):
    return struct.unpack_from("<f", data, off)[0]


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1", errors="replace")


def decode_cstrings(block, expected_count):
    values = []
    pos = 0
    for _ in range(expected_count):
        end = block.find(b"\0", pos)
        if end < 0:
            raise ValueError(
                f"string block terminou sem NUL: pos={pos} size={len(block)}"
            )
        raw = block[pos:end]
        values.append(raw.decode("latin-1", errors="replace"))
        pos = end + 1
    return values, pos


def main():
    ap = argparse.ArgumentParser(
        description="Read-only SMD v0.62 material/texture-name probe"
    )
    ap.add_argument("smd", type=Path)
    ap.add_argument("--json", dest="json_path", type=Path)
    args = ap.parse_args()

    data = args.smd.read_bytes()
    file_size = len(data)

    print("PT_SMD_PROBE_V3_MATERIALS")
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
    mat_counter_header = i32(header, 28)
    mat_file_point = i32(header, 32)
    first_obj_point = i32(header, 36)

    print("header_check=OK")
    print(f"ObjCounter={obj_counter}")
    print(f"MatCounter(header)={mat_counter_header}")
    print(f"MatFilePoint={mat_file_point}")
    print(f"First_ObjInfoPoint={first_obj_point}")

    if not (0 <= mat_file_point <= first_obj_point <= file_size):
        print("FAIL material region bounds")
        return 4

    material_region_size = first_obj_point - mat_file_point
    print(f"material_region_size={material_region_size}")

    if material_region_size < SIZE_MATERIAL_GROUP:
        print("FAIL material region smaller than smMATERIAL_GROUP")
        return 5

    mg_start = mat_file_point
    mg = data[mg_start:mg_start + SIZE_MATERIAL_GROUP]

    mg_head = u32(mg, 0)
    old_material_ptr = u32(mg, MG_OFF_SMATERIAL_PTR)
    material_count = u32(mg, MG_OFF_MATERIAL_COUNT)
    reform_texture = i32(mg, MG_OFF_REFORM_TEXTURE)
    max_material = i32(mg, MG_OFF_MAX_MATERIAL)
    last_search_material = i32(mg, MG_OFF_LAST_SEARCH_MATERIAL)
    last_search_name = cstr(
        mg[MG_OFF_LAST_SEARCH_NAME:MG_OFF_LAST_SEARCH_NAME + 64]
    )

    print()
    print("MATERIAL_GROUP")
    print(f"Head=0x{mg_head:08X}")
    print(f"smMaterial(old_ptr)=0x{old_material_ptr:08X}")
    print(f"MaterialCount={material_count}")
    print(f"ReformTexture={reform_texture}")
    print(f"MaxMaterial={max_material}")
    print(f"LastSearchMaterial={last_search_material}")
    print(f"szLastSearchName={last_search_name!r}")

    all_ok = True

    if material_count != mat_counter_header:
        print(
            "FAIL MaterialCount mismatch: "
            f"group={material_count} header={mat_counter_header}"
        )
        all_ok = False
    else:
        print("MaterialCount_check=OK")

    if material_count > 4096:
        print("FAIL implausible MaterialCount")
        return 6

    fixed_min = SIZE_MATERIAL_GROUP + material_count * SIZE_MATERIAL
    print(f"fixed_minimum_size={fixed_min}")
    print(f"variable_bytes_total={material_region_size - fixed_min}")

    cursor = mg_start + SIZE_MATERIAL_GROUP
    materials = []
    used_count = 0
    total_string_payload = 0

    print()
    print("MATERIALS")

    for index in range(material_count):
        struct_start = cursor
        struct_end = struct_start + SIZE_MATERIAL

        if struct_end > first_obj_point:
            print(f"[{index}] FAIL smMATERIAL exceeds material region")
            all_ok = False
            break

        m = data[struct_start:struct_end]
        cursor = struct_end

        in_use = u32(m, M_OFF_INUSE)
        texture_counter = u32(m, M_OFF_TEXTURE_COUNTER)
        texture_ptrs = list(struct.unpack_from("<8I", m, M_OFF_TEXTURE_PTRS))
        stage_states = list(
            struct.unpack_from("<8I", m, M_OFF_TEXTURE_STAGE_STATE)
        )
        form_states = list(
            struct.unpack_from("<8I", m, M_OFF_TEXTURE_FORM_STATE)
        )

        m_reform_texture = i32(m, M_OFF_REFORM_TEXTURE)
        map_opacity = i32(m, M_OFF_MAP_OPACITY)
        texture_type = u32(m, M_OFF_TEXTURE_TYPE)
        blend_type = u32(m, M_OFF_BLEND_TYPE)
        shade = u32(m, M_OFF_SHADE)
        two_side = u32(m, M_OFF_TWO_SIDE)
        serial_num = u32(m, M_OFF_SERIAL_NUM)

        # smFCOLOR is confirmed as 12 bytes but its member definition was not
        # part of the current proof, so preserve both raw bytes and float view.
        diffuse_raw = m[M_OFF_DIFFUSE:M_OFF_DIFFUSE + 12]
        diffuse_f32 = list(struct.unpack_from("<3f", diffuse_raw))

        transparency = f32(m, M_OFF_TRANSPARENCY)
        self_illum = f32(m, M_OFF_SELF_ILLUM)

        texture_swap = i32(m, M_OFF_TEXTURE_SWAP)
        mat_frame = i32(m, M_OFF_MAT_FRAME)
        texture_clip = i32(m, M_OFF_TEXTURE_CLIP)
        use_state = i32(m, M_OFF_USE_STATE)
        mesh_state = i32(m, M_OFF_MESH_STATE)
        wind_mesh_bottom = i32(m, M_OFF_WIND_MESH_BOTTOM)

        anim_texture_ptrs = list(
            struct.unpack_from("<32I", m, M_OFF_ANIM_TEXTURE_PTRS)
        )
        anim_tex_counter = u32(m, M_OFF_ANIM_TEX_COUNTER)
        frame_mask = u32(m, M_OFF_FRAME_MASK)
        shift_frame_speed = u32(m, M_OFF_SHIFT_FRAME_SPEED)
        animation_frame = u32(m, M_OFF_ANIMATION_FRAME)

        texture_names = []
        anim_texture_names = []
        str_len = None
        string_block_consumed = 0

        mat_ok = True

        if texture_counter > 8:
            print(
                f"[{index}] FAIL TextureCounter={texture_counter} > 8"
            )
            mat_ok = False

        if anim_tex_counter > 32:
            print(
                f"[{index}] FAIL AnimTexCounter={anim_tex_counter} > 32"
            )
            mat_ok = False

        if in_use:
            used_count += 1

            if cursor + 4 > first_obj_point:
                print(f"[{index}] FAIL missing StrLen")
                mat_ok = False
                all_ok = False
                break

            str_len = i32(data, cursor)
            cursor += 4

            if str_len < 0 or cursor + str_len > first_obj_point:
                print(
                    f"[{index}] FAIL invalid StrLen={str_len} "
                    f"cursor={cursor} material_end={first_obj_point}"
                )
                mat_ok = False
                all_ok = False
                break

            block = data[cursor:cursor + str_len]
            cursor += str_len
            total_string_payload += str_len

            expected_strings = 2 * (texture_counter + anim_tex_counter)

            try:
                strings, consumed = decode_cstrings(
                    block, expected_strings
                )
            except ValueError as exc:
                print(f"[{index}] FAIL {exc}")
                strings = []
                consumed = 0
                mat_ok = False
            else:
                string_block_consumed = consumed
                if consumed != str_len:
                    print(
                        f"[{index}] FAIL string bytes consumed={consumed} "
                        f"StrLen={str_len}"
                    )
                    mat_ok = False

                p = 0
                for _ in range(texture_counter):
                    texture_names.append({
                        "Name": strings[p],
                        "NameA": strings[p + 1],
                    })
                    p += 2

                for _ in range(anim_tex_counter):
                    anim_texture_names.append({
                        "Name": strings[p],
                        "NameA": strings[p + 1],
                    })
                    p += 2

        all_ok &= mat_ok

        blend_name = BLEND_NAMES.get(blend_type, "UNKNOWN")
        tex_type_base = texture_type & 0xFF
        texture_type_name = TEXTURE_TYPE_NAMES.get(
            tex_type_base, "UNKNOWN"
        )
        auto_animation = bool(texture_type & 0x100)

        print()
        print(f"[{index}]")
        print(
            f"  InUse={in_use} TextureCounter={texture_counter} "
            f"AnimTexCounter={anim_tex_counter}"
        )
        print(
            f"  TextureType=0x{texture_type:08X} "
            f"({texture_type_name}, auto={int(auto_animation)})"
        )
        print(
            f"  BlendType={blend_type} ({blend_name}) "
            f"Shade={shade} TwoSide={two_side}"
        )
        print(
            f"  Transparency={transparency:.9g} "
            f"SelfIllum={self_illum:.9g} "
            f"MapOpacity={map_opacity}"
        )
        print(
            f"  ReformTexture={m_reform_texture} "
            f"TextureSwap={texture_swap} MatFrame={mat_frame} "
            f"TextureClip={texture_clip}"
        )
        print(
            f"  UseState={use_state} MeshState={mesh_state} "
            f"WindMeshBottom={wind_mesh_bottom} SerialNum={serial_num}"
        )
        print(
            f"  DiffuseRaw={diffuse_raw.hex()} "
            f"DiffuseFloatView={diffuse_f32}"
        )
        print(
            "  smTexture(old_ptrs)="
            + str([f"0x{x:08X}" for x in texture_ptrs[:texture_counter]])
        )

        if in_use:
            print(
                f"  StrLen={str_len} "
                f"string_bytes_consumed={string_block_consumed}"
            )
            for t, pair in enumerate(texture_names):
                print(
                    f"  texture[{t}] Name={pair['Name']!r} "
                    f"NameA={pair['NameA']!r}"
                )
            for t, pair in enumerate(anim_texture_names):
                print(
                    f"  anim_texture[{t}] Name={pair['Name']!r} "
                    f"NameA={pair['NameA']!r}"
                )

        print(f"  material_check={'PASS' if mat_ok else 'FAIL'}")

        materials.append({
            "index": index,
            "InUse": in_use,
            "TextureCounter": texture_counter,
            "AnimTexCounter": anim_tex_counter,
            "TextureType": texture_type,
            "TextureTypeName": texture_type_name,
            "AutoAnimation": auto_animation,
            "BlendType": blend_type,
            "BlendName": blend_name,
            "Shade": shade,
            "TwoSide": two_side,
            "Transparency": transparency,
            "SelfIllum": self_illum,
            "MapOpacity": map_opacity,
            "ReformTexture": m_reform_texture,
            "TextureSwap": texture_swap,
            "MatFrame": mat_frame,
            "TextureClip": texture_clip,
            "UseState": use_state,
            "MeshState": mesh_state,
            "WindMeshBottom": wind_mesh_bottom,
            "SerialNum": serial_num,
            "DiffuseRawHex": diffuse_raw.hex(),
            "DiffuseFloatView": diffuse_f32,
            "TextureStageState": stage_states,
            "TextureFormState": form_states,
            "textures": texture_names,
            "anim_textures": anim_texture_names,
            "StrLen": str_len,
            "material_check": mat_ok,
        })

    print()
    print("SUMMARY")
    print(f"material_count={material_count}")
    print(f"used_materials={used_count}")
    print(f"string_payload_bytes={total_string_payload}")
    print(f"material_cursor={cursor}")
    print(f"First_ObjInfoPoint={first_obj_point}")
    print(f"remaining_bytes={first_obj_point - cursor}")

    if cursor == first_obj_point:
        print("material_region_end_check=OK")
    else:
        print("FAIL material region did not close exactly")
        all_ok = False

    print(f"material_semantic_check={'PASS' if all_ok else 'FAIL'}")

    if args.json_path:
        report = {
            "file": str(args.smd),
            "file_size": file_size,
            "MatCounterHeader": mat_counter_header,
            "MatFilePoint": mat_file_point,
            "First_ObjInfoPoint": first_obj_point,
            "material_region_size": material_region_size,
            "material_group": {
                "Head": mg_head,
                "old_smMaterial_ptr": old_material_ptr,
                "MaterialCount": material_count,
                "ReformTexture": reform_texture,
                "MaxMaterial": max_material,
                "LastSearchMaterial": last_search_material,
                "LastSearchName": last_search_name,
            },
            "materials": materials,
            "summary": {
                "used_materials": used_count,
                "string_payload_bytes": total_string_payload,
                "material_cursor": cursor,
                "remaining_bytes": first_obj_point - cursor,
                "material_semantic_check": all_ok,
            },
        }
        args.json_path.write_text(
            json.dumps(report, indent=2, ensure_ascii=False),
            encoding="utf-8",
        )
        print(f"json={args.json_path}")

    return 0 if all_ok else 10


if __name__ == "__main__":
    sys.exit(main())

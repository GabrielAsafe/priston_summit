from __future__ import annotations

import hashlib
import json
import math
import pathlib
import struct

from .stage072_area import (
    AREA_LIST_CAPACITY,
    _choose_area_encoding,
    _fixed_layout,
    _geometry_cells,
    _rebuild_serialized_area,
)
from .stage072_corpus import (
    DFILE_HEADER_SIZE,
    STAGE_HEADER_SIZE,
    STAGE_AREA_PTR_OFFSET,
    STAGE_AREA_PTR_COUNT,
    STAGE_VERTEX_SIZE,
    STAGE_FACE_SIZE,
    TEXLINK_SIZE,
    LIGHT_SIZE,
    S_AREA_BUFF_PTR,
    S_WAREA_SIZE,
    S_STAGE_MAP_RECT,
    parse_stage072,
)
from .stage072_normals import CURRENT_MODEL, analyze_stage072_normals

TOOL = "PT_STAGE072_GEOMETRY_CURRENT_FAMILY_V1"


def _sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _f32(x: float) -> float:
    return struct.unpack("<f", struct.pack("<f", float(x)))[0]


def _i32(data: bytes | bytearray, off: int) -> int:
    return struct.unpack_from("<i", data, off)[0]


def _u32(data: bytes | bytearray, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def _source_setnormal(p0, p1, p2) -> tuple[int, int, int]:
    """Literal strict-float32 evaluation of supplied SetNormal source.

    The current physical family proves this evaluation byte-exactly. A zero
    cross is refused for geometry writing because the supplied C function
    divides by zero; shipped degenerate faces happen to contain zero normals,
    but that is not a safe generation rule for a newly-created degenerate.
    """
    ax, ay, az = p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]
    bx, by, bz = p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]
    cx = ay * bz - az * by
    cy = az * bx - ax * bz
    cz = ax * by - ay * bx
    if cx == 0 and cy == 0 and cz == 0:
        raise ValueError("geometry edit creates/uses degenerate face; SetNormal divide-by-zero is not authorized")
    i, j, k = _f32(cx), _f32(cy), _f32(cz)
    # (i*i)+(j*j)+(k*k) is a float expression in the physically-proven
    # current family, then cast to double for sqrt, then assigned back to float.
    sq = _f32(_f32(i * i) + _f32(j * j))
    sq = _f32(sq + _f32(k * k))
    d = _f32(math.sqrt(float(sq)))
    if not math.isfinite(d) or d == 0.0:
        raise ValueError("non-finite/zero SetNormal length")
    out = []
    for q in (i, j, k):
        v = _f32(q / d)
        v = _f32(v * _f32(32767.0))
        out.append(int(v))  # C float->int truncates toward zero here.
    return tuple(out)


def _points_shifted(data: bytes | bytearray, layout: dict) -> list[tuple[int, int, int]]:
    out = []
    for vi in range(int(layout["nvertex"])):
        o = int(layout["vertex_base"]) + vi * STAGE_VERTEX_SIZE
        x, y, z = struct.unpack_from("<iii", data, o + 8)
        out.append((x >> 8, y >> 8, z >> 8))
    return out


def _incident_faces(data: bytes | bytearray, layout: dict, vertex_index: int) -> list[int]:
    out = []
    for fi in range(int(layout["nface"])):
        o = int(layout["face_base"]) + fi * STAGE_FACE_SIZE
        verts = struct.unpack_from("<3H", data, o + 8)
        if vertex_index in verts:
            out.append(fi)
    return out


def _current_family_guard(path: pathlib.Path | str) -> dict:
    audit = analyze_stage072_normals(path)
    if audit.get("classification") != CURRENT_MODEL:
        raise RuntimeError(
            "Stage0.72 geometry writer V1 is restricted to the 46-file current exact normal family; "
            f"got {audit.get('classification')}"
        )
    if audit.get("check") != "PASS":
        raise RuntimeError("Stage0.72 normal audit is not PASS")
    return audit


def select_stage072_geometry_witness(path: pathlib.Path | str, *, delta_world: float = 64.0) -> dict:
    p = pathlib.Path(path)
    if not math.isfinite(delta_world) or not (1.0 <= abs(float(delta_world)) <= 256.0):
        raise ValueError("delta_world magnitude must be between 1 and 256")
    delta_raw_f = float(delta_world) * 256.0
    delta_raw = int(round(delta_raw_f))
    if abs(delta_raw_f - delta_raw) > 1e-9 or delta_raw == 0:
        raise ValueError("delta_world must map to an exact 1/256 fixed-point increment")

    data = p.read_bytes()
    parsed = parse_stage072(p, include_payload=True)
    if parsed.get("full_parse") != "PASS":
        raise RuntimeError("Stage0.72 source parser is not PASS")
    normal_audit = _current_family_guard(p)
    area = parsed.get("area") or {}
    if area.get("encoding") != "DWORD32":
        raise RuntimeError(f"geometry writer V1 requires physical DWORD32 StageArea, got {area.get('encoding')}")
    if int(area.get("opaque_trailing_bytes") or 0) != 0:
        raise RuntimeError("geometry writer V1 keeps opaque-suffix Stage0.72 files fail-closed")

    layout = _fixed_layout(data)
    original_geom = _geometry_cells(data, layout)
    degree = [0] * int(layout["nvertex"])
    for fi in range(int(layout["nface"])):
        o = int(layout["face_base"]) + fi * STAGE_FACE_SIZE
        for vi in struct.unpack_from("<3H", data, o + 8):
            degree[int(vi)] += 1

    # Prefer a low-degree vertex so the witness changes a small, auditable
    # normal set. Require StageArea membership to actually change.
    candidates = sorted(range(len(degree)), key=lambda vi: (degree[vi] if degree[vi] else 10**9, vi))
    for vi in candidates:
        if degree[vi] <= 0:
            continue
        vo = int(layout["vertex_base"]) + vi * STAGE_VERTEX_SIZE
        before = list(struct.unpack_from("<iii", data, vo + 8))
        nx = before[0] + delta_raw
        if not -(2**31) <= nx < 2**31:
            continue
        trial = bytearray(data)
        struct.pack_into("<i", trial, vo + 8, nx)
        after = [nx, before[1], before[2]]
        # The normal input must actually change after >>8.
        if (before[0] >> 8) == (after[0] >> 8):
            continue
        incident = _incident_faces(trial, layout, vi)
        points = _points_shifted(trial, layout)
        safe = True
        try:
            for fi in incident:
                fo = int(layout["face_base"]) + fi * STAGE_FACE_SIZE
                a, b, c = struct.unpack_from("<3H", trial, fo + 8)
                _source_setnormal(points[a], points[b], points[c])
        except ValueError:
            safe = False
        if not safe:
            continue
        geom = _geometry_cells(trial, layout)
        if geom["cells"] == original_geom["cells"]:
            continue
        if geom["faces_exceeding_area_list_capacity"]:
            continue
        return {
            "vertex_index": vi,
            "component": "x",
            "delta_world": float(delta_world),
            "delta_raw": delta_raw,
            "before_raw": before,
            "after_raw": after,
            "before_world": [q / 256.0 for q in before],
            "after_world": [q / 256.0 for q in after],
            "incident_faces": incident,
            "incident_face_count": len(incident),
            "stagearea_changed": True,
            "stage_map_rect_before": list(original_geom["stage_map_rect"]),
            "stage_map_rect_after": list(geom["stage_map_rect"]),
            "active_cells_before": len(original_geom["cells"]),
            "active_cells_after": len(geom["cells"]),
            "max_face_area_cell_span_after": int(geom["max_face_area_cell_span"]),
            "normal_family": normal_audit.get("classification"),
        }
    raise RuntimeError("no safe current-family Stage0.72 geometry witness found that changes StageArea")


def strict_write_stage072_geometry_witness(
    source: pathlib.Path | str,
    output: pathlib.Path | str,
    witness_report: pathlib.Path | str,
    *,
    expected_source_sha256: str | None = None,
    report_path: pathlib.Path | str | None = None,
) -> dict:
    source = pathlib.Path(source)
    output = pathlib.Path(output)
    data = source.read_bytes()
    source_sha = _sha(data)
    if expected_source_sha256 and source_sha.lower() != str(expected_source_sha256).lower():
        raise RuntimeError(f"source sha256 drift: expected={expected_source_sha256} actual={source_sha}")
    parsed = parse_stage072(source, include_payload=True)
    if parsed.get("full_parse") != "PASS":
        raise RuntimeError("Stage0.72 source parser is not PASS")
    normal_audit = _current_family_guard(source)
    area = parsed.get("area") or {}
    if area.get("encoding") != "DWORD32" or int(area.get("opaque_trailing_bytes") or 0) != 0:
        raise RuntimeError("geometry writer V1 accepts only DWORD32 StageArea with no opaque suffix")

    witness = json.loads(pathlib.Path(witness_report).read_text(encoding="utf-8"))
    if str(witness.get("source_sha256") or "").lower() != source_sha.lower():
        raise RuntimeError("witness source_sha256 does not match source")
    edits = list(witness.get("geometry_edits") or [])
    if len(edits) != 1:
        raise RuntimeError("geometry writer V1 requires exactly one witness vertex edit")
    edit = edits[0]
    vi = int(edit["vertex_index"])
    if str(edit.get("component") or "").lower() != "x":
        raise RuntimeError("geometry writer V1 witness must edit x only")

    layout = _fixed_layout(data)
    if not 0 <= vi < int(layout["nvertex"]):
        raise ValueError("witness vertex index out of range")
    vo = int(layout["vertex_base"]) + vi * STAGE_VERTEX_SIZE
    source_raw = list(struct.unpack_from("<iii", data, vo + 8))
    before_raw = [int(x) for x in edit["before_raw"]]
    after_raw = [int(x) for x in edit["after_raw"]]
    if source_raw != before_raw:
        raise RuntimeError(f"witness/source vertex mismatch: source={source_raw} witness={before_raw}")
    if after_raw[1:] != before_raw[1:] or after_raw[0] == before_raw[0]:
        raise RuntimeError("geometry writer V1 expects an x-only nonzero edit")
    if any(not -(2**31) <= int(q) < 2**31 for q in after_raw):
        raise ValueError("edited fixed-point coordinate outside int32")

    out = bytearray(data)
    struct.pack_into("<iii", out, vo + 8, *after_raw)
    incident = _incident_faces(out, layout, vi)
    if sorted(int(x) for x in edit.get("incident_faces") or []) != incident:
        raise RuntimeError("witness incident-face set mismatch")

    points = _points_shifted(out, layout)
    normal_changes = []
    for fi in incident:
        fo = int(layout["face_base"]) + fi * STAGE_FACE_SIZE
        a, b, c = struct.unpack_from("<3H", out, fo + 8)
        normal = _source_setnormal(points[a], points[b], points[c])
        before4 = struct.unpack_from("<4h", data, fo + 20)
        struct.pack_into("<3h", out, fo + 20, *normal)
        after4 = struct.unpack_from("<4h", out, fo + 20)
        if after4[3] != before4[3]:
            raise AssertionError("VectNormal[3] must remain byte-identical")
        normal_changes.append({
            "face_index": fi,
            "before": list(before4[:3]),
            "after": list(after4[:3]),
            "vectnormal3_preserved": int(after4[3]),
        })

    # Rebuild StageArea and StageMapRect from the edited geometry using the
    # physical DWORD pointer-counter family proven in V0.6.4.
    stage_start = DFILE_HEADER_SIZE
    stage = bytes(out[stage_start:stage_start + STAGE_HEADER_SIZE])
    physical = _choose_area_encoding(data, data[stage_start:stage_start + STAGE_HEADER_SIZE], layout)
    if int(physical["width"]) != 4 or int(physical["trailing_bytes"]) != 0:
        raise RuntimeError("unexpected non-DWORD/opaque physical StageArea in geometry writer V1")
    geometry = _geometry_cells(out, layout)
    if geometry["faces_exceeding_area_list_capacity"]:
        raise RuntimeError("edited geometry exceeds AreaList[4096] face-cell capacity")
    rebuilt_area = _rebuild_serialized_area(
        geometry,
        width=4,
        area_buffer_base=_u32(stage, S_AREA_BUFF_PTR),
    )
    ptr_abs = stage_start + STAGE_AREA_PTR_OFFSET
    out[ptr_abs:ptr_abs + STAGE_AREA_PTR_COUNT * 4] = rebuilt_area["pointer_bytes"]
    struct.pack_into("<i", out, stage_start + S_WAREA_SIZE, int(rebuilt_area["wAreaSize"]))
    struct.pack_into("<4i", out, stage_start + S_STAGE_MAP_RECT, *geometry["stage_map_rect"])

    area_base = int(layout["area_base"])
    final = bytes(out[:area_base]) + rebuilt_area["payload_bytes"]
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(final)

    rebuilt = parse_stage072(output, include_payload=True)
    if rebuilt.get("full_parse") != "PASS":
        raise RuntimeError("rebuilt geometry Stage0.72 parser is not PASS")
    rebuilt_normals = analyze_stage072_normals(output)
    if rebuilt_normals.get("classification") != CURRENT_MODEL or rebuilt_normals.get("check") != "PASS":
        raise RuntimeError(f"rebuilt geometry left current exact normal family: {rebuilt_normals.get('classification')}")

    result = {
        "tool": TOOL,
        "source": str(source),
        "output": str(output),
        "source_sha256": source_sha,
        "output_sha256": _sha(final),
        "normal_family": normal_audit.get("classification"),
        "vertex_edit": {
            "vertex_index": vi,
            "component": "x",
            "before_raw": before_raw,
            "after_raw": after_raw,
            "delta_raw": after_raw[0] - before_raw[0],
            "delta_world": (after_raw[0] - before_raw[0]) / 256.0,
        },
        "incident_faces": incident,
        "normal_changes": normal_changes,
        "normal_change_face_count": len(normal_changes),
        "stagearea": {
            "encoding": "DWORD32",
            "capacity_model": rebuilt_area["capacity_model"],
            "active_cells": int(rebuilt_area["active_cells"]),
            "wAreaSize": int(rebuilt_area["wAreaSize"]),
            "payload_bytes": len(rebuilt_area["payload_bytes"]),
            "stage_map_rect": list(geometry["stage_map_rect"]),
            "max_face_area_cell_span": int(geometry["max_face_area_cell_span"]),
        },
        "source_parse": parsed.get("full_parse"),
        "rebuilt_parse": rebuilt.get("full_parse"),
        "rebuilt_normal_family": rebuilt_normals.get("classification"),
        "client_modified": False,
        "check": "PASS",
    }
    if report_path:
        rp = pathlib.Path(report_path)
        rp.parent.mkdir(parents=True, exist_ok=True)
        rp.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return result


def compare_stage072_geometry_roundtrip(
    source: pathlib.Path | str,
    output: pathlib.Path | str,
    witness_report: pathlib.Path | str,
    *,
    report_path: pathlib.Path | str | None = None,
) -> dict:
    source = pathlib.Path(source)
    output = pathlib.Path(output)
    sb = source.read_bytes()
    ob = output.read_bytes()
    s = parse_stage072(source, include_payload=True)
    o = parse_stage072(output, include_payload=True)
    witness = json.loads(pathlib.Path(witness_report).read_text(encoding="utf-8"))
    errors = []
    if s.get("full_parse") != "PASS": errors.append("SOURCE_PARSE")
    if o.get("full_parse") != "PASS": errors.append("OUTPUT_PARSE")
    if analyze_stage072_normals(source).get("classification") != CURRENT_MODEL: errors.append("SOURCE_NORMAL_FAMILY")
    if analyze_stage072_normals(output).get("classification") != CURRENT_MODEL: errors.append("OUTPUT_NORMAL_FAMILY")
    edits = list(witness.get("geometry_edits") or [])
    if len(edits) != 1: errors.append("WITNESS_EDIT_COUNT")
    edit = edits[0] if edits else {}
    vi = int(edit.get("vertex_index", -1))

    ls = _fixed_layout(sb)
    lo = _fixed_layout(ob)
    # Counts and all fixed-section boundaries before StageArea must remain equal.
    for k in ("nvertex", "nface", "ntexlink", "nlight", "material_start", "vertex_base", "face_base", "texlink_base", "light_base", "area_base"):
        if int(ls[k]) != int(lo[k]): errors.append(f"LAYOUT_{k}")

    # DFILE header + material bytes are immutable.
    if sb[:int(ls["vertex_base"])] != ob[:int(lo["vertex_base"])]:
        # Stage header is inside this range and has authorized fields; check it separately.
        if sb[:DFILE_HEADER_SIZE] != ob[:DFILE_HEADER_SIZE]: errors.append("DFILE_HEADER")
        ms = DFILE_HEADER_SIZE + STAGE_HEADER_SIZE
        if sb[ms:int(ls["vertex_base"])] != ob[ms:int(lo["vertex_base"])]: errors.append("MATERIALS")

    # Stage header: only StageArea pointer table, wAreaSize and StageMapRect may change.
    shs = bytearray(sb[DFILE_HEADER_SIZE:DFILE_HEADER_SIZE + STAGE_HEADER_SIZE])
    sho = bytearray(ob[DFILE_HEADER_SIZE:DFILE_HEADER_SIZE + STAGE_HEADER_SIZE])
    allowed_header = set(range(STAGE_AREA_PTR_OFFSET, STAGE_AREA_PTR_OFFSET + STAGE_AREA_PTR_COUNT * 4))
    allowed_header.update(range(S_WAREA_SIZE, S_WAREA_SIZE + 4))
    allowed_header.update(range(S_STAGE_MAP_RECT, S_STAGE_MAP_RECT + 16))
    header_unexpected = [i for i, (a,b) in enumerate(zip(shs, sho)) if a != b and i not in allowed_header]
    if header_unexpected: errors.append("STAGE_HEADER_UNEXPECTED")

    # Vertex bytes: exactly one xyz triplet may differ, and V1 permits x only.
    vertex_unexpected = []
    for vj in range(int(ls["nvertex"])):
        a0 = int(ls["vertex_base"]) + vj * STAGE_VERTEX_SIZE
        b0 = int(lo["vertex_base"]) + vj * STAGE_VERTEX_SIZE
        for j in range(STAGE_VERTEX_SIZE):
            if sb[a0+j] == ob[b0+j]:
                continue
            allowed = (vj == vi and 8 <= j < 12)
            if not allowed:
                vertex_unexpected.append((vj,j))
                if len(vertex_unexpected) >= 20: break
        if len(vertex_unexpected) >= 20: break
    if vertex_unexpected: errors.append("VERTEX_UNEXPECTED")

    # Topology/material/face housekeeping are immutable; only VectNormal[0..2]
    # of incident faces may change. VectNormal[3] is always opaque/exact.
    incident = set(int(x) for x in edit.get("incident_faces") or [])
    face_unexpected = []
    vectnormal3_exact = True
    for fi in range(int(ls["nface"])):
        a0 = int(ls["face_base"]) + fi * STAGE_FACE_SIZE
        b0 = int(lo["face_base"]) + fi * STAGE_FACE_SIZE
        if sb[a0+26:a0+28] != ob[b0+26:b0+28]:
            vectnormal3_exact = False
        for j in range(STAGE_FACE_SIZE):
            if sb[a0+j] == ob[b0+j]:
                continue
            allowed = (fi in incident and 20 <= j < 26)
            if not allowed:
                face_unexpected.append((fi,j))
                if len(face_unexpected) >= 20: break
        if len(face_unexpected) >= 20: break
    if face_unexpected: errors.append("FACE_UNEXPECTED")
    if not vectnormal3_exact: errors.append("VECTNORMAL3")

    # TexLinks and lights are immutable byte-for-byte.
    tex_exact = sb[int(ls["texlink_base"]):int(ls["light_base"])] == ob[int(lo["texlink_base"]):int(lo["light_base"])]
    lights_exact = sb[int(ls["light_base"]):int(ls["area_base"])] == ob[int(lo["light_base"]):int(lo["area_base"])]
    if not tex_exact: errors.append("TEXLINKS")
    if not lights_exact: errors.append("LIGHTS")

    # Re-run the physical StageArea proof against the rebuilt output. It must
    # be exact for the edited geometry, not merely parseable.
    from .stage072_area import analyze_stage072_stagearea
    area_check = analyze_stage072_stagearea(output)
    if area_check.get("check") != "PASS": errors.append("STAGEAREA_REBUILD")
    source_area = analyze_stage072_stagearea(source)
    stagearea_changed = sb[int(ls["area_base"]):] != ob[int(lo["area_base"]):]
    if not stagearea_changed: errors.append("STAGEAREA_NOT_EXERCISED")

    # Semantic invariants unrelated to geometry.
    if s.get("counts", {}).get("materials") != o.get("counts", {}).get("materials"): errors.append("MATERIAL_COUNT")
    if s.get("materials") != o.get("materials"): errors.append("MATERIAL_SEMANTICS")
    if s.get("texlink_depth_face_counts") != o.get("texlink_depth_face_counts"): errors.append("TEXLINK_DEPTHS")
    if s.get("max_texlink_depth") != o.get("max_texlink_depth"): errors.append("MAX_DEPTH")
    if s.get("lighting") != o.get("lighting"): errors.append("LIGHTING_CONTROLS")
    if (s.get("payload") or {}).get("lights") != (o.get("payload") or {}).get("lights"): errors.append("LIGHT_PAYLOAD")

    result = {
        "tool": "PT_STAGE072_GEOMETRY_ROUNDTRIP_COMPARE_V1",
        "source": str(source),
        "output": str(output),
        "source_sha256": _sha(sb),
        "output_sha256": _sha(ob),
        "source_parse": s.get("full_parse"),
        "output_parse": o.get("full_parse"),
        "normal_family": CURRENT_MODEL,
        "vertex_index": vi,
        "incident_face_count": len(incident),
        "file_size_before": len(sb),
        "file_size_after": len(ob),
        "stagearea_changed": stagearea_changed,
        "stagearea_source": source_area,
        "stagearea_output": area_check,
        "texlinks_byte_identical": tex_exact,
        "lights_byte_identical": lights_exact,
        "vectnormal3_byte_identical": vectnormal3_exact,
        "header_unexpected_diff_count": len(header_unexpected),
        "vertex_unexpected_diff_count": len(vertex_unexpected),
        "face_unexpected_diff_count": len(face_unexpected),
        "errors": errors,
        "check": "PASS" if not errors else "FAIL",
        "client_modified": False,
    }
    if report_path:
        rp = pathlib.Path(report_path)
        rp.parent.mkdir(parents=True, exist_ok=True)
        rp.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    if errors:
        raise RuntimeError(f"Stage0.72 strict geometry roundtrip compare failed: {errors[:8]}")
    return result

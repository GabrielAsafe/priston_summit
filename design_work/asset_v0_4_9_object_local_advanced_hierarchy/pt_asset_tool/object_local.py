from __future__ import annotations

"""SMD Model0.62 static object-local transform parser/evaluator.

V1 intentionally targets the flat/no-Physique layout proven by the client corpus.
Rotation keys in the source are incremental local quaternions; the serialized
TmPrevRot matrix beside each key is the accumulated absolute rotation and is
used as source truth for preview/evaluation.
"""

import bisect
import json
import math
import pathlib
import struct
from typing import Any

SMD_HEADER = b"SMD Model data Ver 0.62"
SMD_HEADER_SIZE = 556
SMD_INFO_SIZE = 40
SMD_OBJECT_SIZE = 2236
SMD_VERTEX_SIZE = 24
SMD_FACE_SIZE = 36
SMD_TEXLINK_SIZE = 32
ROT_KEY_SIZE = 20
POS_KEY_SIZE = 16
SCALE_KEY_SIZE = 16
PREVROT_SIZE = 64
ENGINE_UNITS_PER_PREVIEW_FRAME = 160

O_PHYSIQUE_PTR = 16
O_NVERTEX = 84
O_NFACE = 88
O_NTEXLINK = 92
O_NODE_NAME = 172
O_NODE_PARENT = 204
O_TM = 240
O_TM_INVERT = 304
O_Q = 628
O_SCALE = 644
O_POS = 656
O_NROT = 684
O_NPOS = 688
O_NSCALE = 692
O_TM_FRAME_CNT = 2232


def _cstring(data: bytes) -> str:
    return data.split(b"\0", 1)[0].decode("latin1", "replace")


def _finite(values) -> bool:
    return all(math.isfinite(float(x)) for x in values)


def _quat_normalize(q: tuple[float, float, float, float]) -> tuple[float, float, float, float]:
    x, y, z, w = (float(v) for v in q)
    n = math.sqrt(x*x + y*y + z*z + w*w)
    if n < 1e-20:
        raise ValueError("zero quaternion")
    return (x/n, y/n, z/n, w/n)


def _quat_conjugate(q):
    x, y, z, w = q
    return (-x, -y, -z, w)


def _quat_mul(a, b):
    ax, ay, az, aw = a
    bx, by, bz, bw = b
    return (
        aw*bx + ax*bw + ay*bz - az*by,
        aw*by - ax*bz + ay*bw + az*bx,
        aw*bz + ax*by - ay*bx + az*bw,
        aw*bw - ax*bx - ay*by - az*bz,
    )


def quat_slerp(a, b, t: float):
    a = _quat_normalize(a)
    b = _quat_normalize(b)
    dot = sum(x*y for x, y in zip(a, b))
    if dot < 0.0:
        b = tuple(-x for x in b)
        dot = -dot
    dot = max(-1.0, min(1.0, dot))
    if dot > 0.9995:
        out = tuple(x + (y-x)*t for x, y in zip(a, b))
        return _quat_normalize(out)
    theta0 = math.acos(dot)
    sin0 = math.sin(theta0)
    s0 = math.sin((1.0-t)*theta0) / sin0
    s1 = math.sin(t*theta0) / sin0
    return tuple(s0*x + s1*y for x, y in zip(a, b))


def quat_to_matrix4(q):
    x, y, z, w = _quat_normalize(q)
    return (
        (1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w), 0.0),
        (2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w), 0.0),
        (2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y), 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )


def matrix4_mul(a, b):
    return tuple(tuple(sum(a[r][k]*b[k][c] for k in range(4)) for c in range(4)) for r in range(4))


def matrix4_to_quat(m):
    # Standard column-vector rotation matrix -> (x,y,z,w), branch-stable.
    r00, r01, r02 = m[0][:3]
    r10, r11, r12 = m[1][:3]
    r20, r21, r22 = m[2][:3]
    trace = r00 + r11 + r22
    if trace > 0.0:
        s = math.sqrt(trace + 1.0) * 2.0
        w = 0.25 * s
        x = (r21-r12) / s
        y = (r02-r20) / s
        z = (r10-r01) / s
    elif r00 > r11 and r00 > r22:
        s = math.sqrt(max(0.0, 1.0+r00-r11-r22)) * 2.0
        w = (r21-r12) / s
        x = 0.25 * s
        y = (r01+r10) / s
        z = (r02+r20) / s
    elif r11 > r22:
        s = math.sqrt(max(0.0, 1.0+r11-r00-r22)) * 2.0
        w = (r02-r20) / s
        x = (r01+r10) / s
        y = 0.25 * s
        z = (r12+r21) / s
    else:
        s = math.sqrt(max(0.0, 1.0+r22-r00-r11)) * 2.0
        w = (r10-r01) / s
        x = (r02+r20) / s
        y = (r12+r21) / s
        z = 0.25 * s
    return _quat_normalize((x, y, z, w))


def _matrix_from_flat(values):
    return tuple(tuple(float(values[r*4+c]) for c in range(4)) for r in range(4))


def _matrix_max_error(a, b) -> float:
    return max(abs(float(a[r][c])-float(b[r][c])) for r in range(4) for c in range(4))


def _validate_frames(keys: list[dict[str, Any]], label: str) -> None:
    frames = [int(k["frame"]) for k in keys]
    if frames != sorted(frames):
        raise ValueError(f"{label} frames are not monotonic: {frames[:12]}")
    if len(set(frames)) != len(frames):
        raise ValueError(f"{label} contains duplicate frames")



def _fixed_matrix4(raw):
    return tuple(tuple(float(raw[r*4+c]) / 256.0 for c in range(4)) for r in range(4))


def _identity_error(m):
    ident = tuple(tuple(1.0 if r == c else 0.0 for c in range(4)) for r in range(4))
    return _matrix_max_error(m, ident)


def validate_hierarchy(parsed: dict[str, Any]) -> dict[str, Any]:
    """Resolve NodeParent with the runtime's LinkObject semantics.

    The client scans objects in file order and assigns the first case-insensitive
    NodeName match.  A missing NodeParent is *not* fatal: pParent remains null and
    the object is later inserted in TmSort as a root.  Duplicate NodeNames are
    therefore legal (though diagnostically interesting).
    """
    objects = parsed["objects"]
    by_name: dict[str, list[dict[str, Any]]] = {}
    for obj in objects:
        by_name.setdefault(str(obj["name"]).casefold(), []).append(obj)

    duplicate_groups = {
        vals[0]["name"]: [int(x["index"]) for x in vals]
        for vals in by_name.values() if len(vals) > 1
    }
    roots: list[str] = []
    edges: list[dict[str, Any]] = []
    unresolved: list[dict[str, Any]] = []

    for obj in objects:
        obj["parent_index"] = None
        obj["runtime_parent_resolution"] = "ROOT_NO_NODEPARENT"
        parent_name = str(obj.get("parent") or "")
        if not parent_name:
            roots.append(obj["name"])
            continue
        hits = by_name.get(parent_name.casefold()) or []
        if not hits:
            # Exact client behavior: LinkObject leaves pParent == 0.
            obj["runtime_parent_resolution"] = "UNRESOLVED_RUNTIME_ROOT"
            roots.append(obj["name"])
            unresolved.append({"object": obj["name"], "object_index": int(obj["index"]), "parent": parent_name})
            continue
        parent = hits[0]
        if parent is obj:
            raise ValueError(f"NodeParent self-cycle under runtime first-match semantics: object={obj['name']!r}")
        obj["parent_index"] = int(parent["index"])
        obj["runtime_parent_resolution"] = "FIRST_CASE_INSENSITIVE_MATCH"
        rel = matrix4_mul(tuple(tuple(x for x in row) for row in obj["tm_matrix"]),
                          tuple(tuple(x for x in row) for row in parent["tm_invert_matrix"]))
        obj["bind_local_matrix"] = [list(r) for r in rel]
        obj["bind_local_identity_error"] = _identity_error(rel)
        edges.append({"parent": parent["name"], "parent_index": int(parent["index"]),
                      "child": obj["name"], "child_index": int(obj["index"]),
                      "child_motion_class": obj["motion_class"],
                      "bind_local_identity_error": obj["bind_local_identity_error"]})

    # Cycle check on the *effective* runtime parent graph.
    visiting: set[int] = set()
    done: set[int] = set()
    def visit(obj: dict[str, Any]) -> None:
        idx = int(obj["index"])
        if idx in done:
            return
        if idx in visiting:
            raise ValueError(f"NodeParent cycle detected at object index={idx} name={obj['name']!r}")
        visiting.add(idx)
        pi = obj.get("parent_index")
        if pi is not None:
            visit(objects[int(pi)])
        visiting.remove(idx)
        done.add(idx)
    for obj in objects:
        visit(obj)

    return {
        "root_objects": roots,
        "parent_edges": edges,
        "parent_edge_count": len(edges),
        "unresolved_parent_count": len(unresolved),
        "unresolved_parents": unresolved,
        "duplicate_node_name_group_count": len(duplicate_groups),
        "duplicate_node_name_groups": duplicate_groups,
        "parent_resolution_semantics": "CLIENT_LINKOBJECT_FIRST_MATCH_UNRESOLVED_AS_ROOT",
    }


def parse_object_local_smd(path: str | pathlib.Path, *, strict_v1: bool = False) -> dict[str, Any]:
    path = pathlib.Path(path).expanduser().resolve()
    data = path.read_bytes()
    if len(data) < SMD_HEADER_SIZE or not data.startswith(SMD_HEADER):
        raise ValueError("not SMD Model data Ver 0.62")
    object_count = struct.unpack_from("<i", data, 24)[0]
    if object_count <= 0 or SMD_HEADER_SIZE + object_count*SMD_INFO_SIZE > len(data):
        raise ValueError(f"invalid object_count={object_count}")

    objects = []
    for idx in range(object_count):
        info_off = SMD_HEADER_SIZE + idx*SMD_INFO_SIZE
        info_name = _cstring(data[info_off:info_off+32])
        obj_len, obj_off = struct.unpack_from("<2i", data, info_off+32)
        if obj_off < 0 or obj_len < SMD_OBJECT_SIZE or obj_off+obj_len > len(data):
            raise ValueError(f"object[{idx}] invalid bounds start={obj_off} length={obj_len}")
        physique_ptr = struct.unpack_from("<I", data, obj_off+O_PHYSIQUE_PTR)[0]
        if physique_ptr:
            raise ValueError(f"object[{idx}] {info_name}: Physique is not object-local static V1")
        nv = struct.unpack_from("<i", data, obj_off+O_NVERTEX)[0]
        nf = struct.unpack_from("<i", data, obj_off+O_NFACE)[0]
        nt = struct.unpack_from("<i", data, obj_off+O_NTEXLINK)[0]
        nr = struct.unpack_from("<i", data, obj_off+O_NROT)[0]
        np = struct.unpack_from("<i", data, obj_off+O_NPOS)[0]
        ns = struct.unpack_from("<i", data, obj_off+O_NSCALE)[0]
        tm_frame_cnt = struct.unpack_from("<i", data, obj_off+O_TM_FRAME_CNT)[0]
        if min(nv, nf, nt, nr, np, ns, tm_frame_cnt) < 0:
            raise ValueError(f"object[{idx}] negative count")
        if strict_v1 and tm_frame_cnt:
            raise ValueError(f"SEGMENTED_OBJECT_LOCAL_UNSUPPORTED_V1 object={info_name} TmFrameCnt={tm_frame_cnt}")

        arr = obj_off + SMD_OBJECT_SIZE + nv*SMD_VERTEX_SIZE + nf*SMD_FACE_SIZE + nt*SMD_TEXLINK_SIZE
        arrays_end = arr + nr*ROT_KEY_SIZE + np*POS_KEY_SIZE + ns*SCALE_KEY_SIZE + nr*PREVROT_SIZE
        if arrays_end != obj_off + obj_len:
            raise ValueError(
                f"object[{idx}] serialized length mismatch expected_end={obj_off+obj_len} parsed_end={arrays_end}"
            )

        rot_keys = []
        for i in range(nr):
            frame, x, y, z, w = struct.unpack_from("<i4f", data, arr+i*ROT_KEY_SIZE)
            if not _finite((x, y, z, w)):
                raise ValueError(f"object[{idx}] non-finite rotation key {i}")
            norm = math.sqrt(x*x+y*y+z*z+w*w)
            if not 0.98 <= norm <= 1.02:
                raise ValueError(
                    f"LEGACY_AFFINE_ROTATION_UNSUPPORTED object[{idx}] quaternion_norm={norm:.8f} key={i}; "
                    "runtime may preserve non-unit quaternion scale/shear and must not be normalized silently"
                )
            rot_keys.append({"frame": frame, "q_incremental_xyzw": [x, y, z, w], "q_norm": norm})
        _validate_frames(rot_keys, f"object[{idx}].rot")

        pos_off = arr + nr*ROT_KEY_SIZE
        pos_keys = []
        for i in range(np):
            frame, x, y, z = struct.unpack_from("<i3f", data, pos_off+i*POS_KEY_SIZE)
            if not _finite((x, y, z)):
                raise ValueError(f"object[{idx}] non-finite position key {i}")
            pos_keys.append({"frame": frame, "xyz": [x, y, z]})
        _validate_frames(pos_keys, f"object[{idx}].pos")

        scale_off = pos_off + np*POS_KEY_SIZE
        scale_keys = []
        for i in range(ns):
            frame, sx, sy, sz = struct.unpack_from("<4i", data, scale_off+i*SCALE_KEY_SIZE)
            scale_keys.append({"frame": frame, "raw": [sx, sy, sz], "xyz": [sx/256.0, sy/256.0, sz/256.0]})
        _validate_frames(scale_keys, f"object[{idx}].scale")

        prev_off = scale_off + ns*SCALE_KEY_SIZE
        prevrot = []
        cumulative_max_error = 0.0
        for i in range(nr):
            flat = struct.unpack_from("<16f", data, prev_off+i*PREVROT_SIZE)
            if not _finite(flat):
                raise ValueError(f"object[{idx}] non-finite TmPrevRot matrix {i}")
            matrix = _matrix_from_flat(flat)
            q_abs = matrix4_to_quat(matrix)
            prevrot.append({"frame": rot_keys[i]["frame"], "matrix": [list(r) for r in matrix], "q_absolute_xyzw": list(q_abs)})
            q_inc = tuple(rot_keys[i]["q_incremental_xyzw"])
            previous_matrix = tuple(tuple(float(v) for v in row) for row in prevrot[i-1]["matrix"]) if i else None
            predicted = quat_to_matrix4(q_inc) if i == 0 else matrix4_mul(previous_matrix, quat_to_matrix4(q_inc))
            err = _matrix_max_error(predicted, matrix)
            cumulative_max_error = max(cumulative_max_error, err)
        if nr and cumulative_max_error > 2e-4:
            raise ValueError(
                f"LEGACY_AFFINE_ROTATION_UNSUPPORTED object[{idx}] TmPrevRot cumulative mismatch "
                f"max_error={cumulative_max_error:.8g}; source matrix contains transform drift that a pure quaternion preview would erase"
            )

        tm_raw = struct.unpack_from("<16i", data, obj_off+O_TM)
        tm_invert_raw = struct.unpack_from("<16i", data, obj_off+O_TM_INVERT)
        base_q = struct.unpack_from("<4f", data, obj_off+O_Q)
        base_scale_raw = struct.unpack_from("<3i", data, obj_off+O_SCALE)
        base_pos_raw = struct.unpack_from("<3i", data, obj_off+O_POS)
        channels = [name for name, count in (("ROT", nr), ("POS", np), ("SCALE", ns)) if count]
        obj = {
            "index": idx,
            "info_name": info_name,
            "name": _cstring(data[obj_off+O_NODE_NAME:obj_off+O_NODE_NAME+32]) or info_name,
            "parent": _cstring(data[obj_off+O_NODE_PARENT:obj_off+O_NODE_PARENT+32]),
            "object_offset": obj_off,
            "object_length": obj_len,
            "nVertex": nv, "nFace": nf, "nTexLink": nt,
            "nRot": nr, "nPos": np, "nScale": ns, "TmFrameCnt": tm_frame_cnt,
            "channels_serialized": channels,
            "tm_raw": list(tm_raw),
            "tm_invert_raw": list(tm_invert_raw),
            "tm_matrix": [list(r) for r in _fixed_matrix4(tm_raw)],
            "tm_invert_matrix": [list(r) for r in _fixed_matrix4(tm_invert_raw)],
            "base_q_xyzw": list(base_q),
            "base_scale_raw": list(base_scale_raw),
            "base_scale_xyz": [v/256.0 for v in base_scale_raw],
            "base_pos_raw": list(base_pos_raw),
            "base_pos_xyz": [v/256.0 for v in base_pos_raw],
            "rot_keys": rot_keys,
            "pos_keys": pos_keys,
            "scale_keys": scale_keys,
            "tm_prev_rot": prevrot,
            "rotation_semantics_check": "PASS" if nr else "NOT_APPLICABLE",
            "rotation_cumulative_max_error": cumulative_max_error,
        }
        objects.append(obj)

    result = {
        "format": "PT_OBJECT_LOCAL_SOURCE_V1",
        "source": str(path),
        "file_size": len(data),
        "object_count": object_count,
        "objects": objects,
    }
    for obj in objects:
        _annotate_motion(obj)
    hierarchy = validate_hierarchy(result)
    result.update(hierarchy)
    result["animated_child_count"] = sum(1 for o in objects if o.get("parent_index") is not None and o.get("motion_class") == "ANIMATED")
    result["animated_object_count"] = sum(1 for o in objects if o["motion_class"] == "ANIMATED")
    result["hold_object_count"] = sum(1 for o in objects if o["motion_class"] == "HOLD")
    result["object_local_object_count"] = sum(1 for o in objects if o["channels_serialized"])
    result["max_engine_frame"] = max((o["max_engine_frame"] for o in objects), default=0)
    return result


def _interp_vec(keys: list[dict[str, Any]], frame: float, field: str, fallback):
    if not keys:
        return tuple(float(x) for x in fallback)
    frames = [k["frame"] for k in keys]
    if frame <= frames[0]:
        return tuple(float(x) for x in keys[0][field])
    if frame >= frames[-1]:
        return tuple(float(x) for x in keys[-1][field])
    hi = bisect.bisect_right(frames, frame)
    lo = hi-1
    f0, f1 = frames[lo], frames[hi]
    t = (float(frame)-f0)/(f1-f0)
    a, b = keys[lo][field], keys[hi][field]
    return tuple(float(x)+(float(y)-float(x))*t for x, y in zip(a, b))


def _interp_rot(obj: dict[str, Any], frame: float):
    rows = obj["tm_prev_rot"]
    if not rows:
        return _quat_normalize(tuple(obj["base_q_xyzw"]))
    frames = [r["frame"] for r in rows]
    if frame <= frames[0]:
        return tuple(rows[0]["q_absolute_xyzw"])
    if frame >= frames[-1]:
        return tuple(rows[-1]["q_absolute_xyzw"])
    hi = bisect.bisect_right(frames, frame)
    lo = hi-1
    t = (float(frame)-frames[lo])/(frames[hi]-frames[lo])
    return quat_slerp(tuple(rows[lo]["q_absolute_xyzw"]), tuple(rows[hi]["q_absolute_xyzw"]), t)


def absolute_sample(obj: dict[str, Any], engine_frame: float) -> dict[str, Any]:
    pos = _interp_vec(obj["pos_keys"], engine_frame, "xyz", obj["base_pos_xyz"])
    scale = _interp_vec(obj["scale_keys"], engine_frame, "xyz", obj["base_scale_xyz"])
    rot = _interp_rot(obj, engine_frame)
    return {"engine_frame": float(engine_frame), "location": list(pos), "scale": list(scale), "rotation_xyzw": list(rot)}


def normalized_sample(obj: dict[str, Any], engine_frame: float, reference_frame: float | None = None) -> dict[str, Any]:
    if reference_frame is None:
        reference_frame = float(obj.get("reference_engine_frame", 0))
    ref = absolute_sample(obj, reference_frame)
    cur = absolute_sample(obj, engine_frame)
    loc = [cur["location"][i]-ref["location"][i] for i in range(3)]
    scale = []
    for a, b in zip(cur["scale"], ref["scale"]):
        if abs(b) < 1e-12:
            if abs(a-b) > 1e-8:
                raise ValueError(f"cannot normalize scale through zero reference: ref={ref['scale']} cur={cur['scale']}")
            scale.append(1.0)
        else:
            scale.append(a/b)
    q_ref = tuple(ref["rotation_xyzw"])
    q_cur = tuple(cur["rotation_xyzw"])
    q_delta = _quat_normalize(_quat_mul(_quat_conjugate(q_ref), q_cur))
    return {
        "engine_frame": float(engine_frame),
        "location": loc,
        "scale": scale,
        "rotation_xyzw": list(q_delta),
    }


def _annotate_motion(obj: dict[str, Any]) -> None:
    frames = sorted({int(k["frame"]) for group in (obj["rot_keys"], obj["pos_keys"], obj["scale_keys"]) for k in group})
    ref = frames[0] if frames else 0
    maxf = frames[-1] if frames else 0
    obj["reference_engine_frame"] = ref
    obj["max_engine_frame"] = maxf
    obj["source_key_frames"] = frames
    if not frames:
        obj["motion_class"] = "NONE"
        obj["channels_animated"] = []
        return
    changed = {"ROT": False, "POS": False, "SCALE": False}
    for f in frames:
        s = normalized_sample(obj, f, ref)
        if max(abs(v) for v in s["location"]) > 1e-5:
            changed["POS"] = True
        if max(abs(v-1.0) for v in s["scale"]) > 1e-5:
            changed["SCALE"] = True
        q = s["rotation_xyzw"]
        # q and -q represent the same rotation.
        if abs(abs(q[3])-1.0) > 1e-6 or max(abs(q[i]) for i in range(3)) > 1e-5:
            changed["ROT"] = True
    obj["channels_animated"] = [c for c in ("ROT", "POS", "SCALE") if changed[c]]
    obj["motion_class"] = "ANIMATED" if obj["channels_animated"] else "HOLD"


def preview_engine_frames(obj: dict[str, Any], step: int = ENGINE_UNITS_PER_PREVIEW_FRAME) -> list[int]:
    ref = int(obj.get("reference_engine_frame", 0))
    end = int(obj.get("max_engine_frame", ref))
    if end < ref:
        raise ValueError("max frame precedes reference frame")
    frames = set(int(x) for x in obj.get("source_key_frames") or [])
    if step <= 0:
        raise ValueError("preview step must be positive")
    f = ref
    while f <= end:
        frames.add(f)
        f += step
    frames.add(end)
    return sorted(x for x in frames if ref <= x <= end)


def compact_report(parsed: dict[str, Any], *, include_key_samples: bool = True) -> dict[str, Any]:
    rows = []
    for obj in parsed["objects"]:
        row = {
            "index": obj["index"], "name": obj["name"], "parent": obj["parent"],
            "nVertex": obj["nVertex"], "nFace": obj["nFace"], "nTexLink": obj["nTexLink"],
            "nRot": obj["nRot"], "nPos": obj["nPos"], "nScale": obj["nScale"], "TmFrameCnt": obj["TmFrameCnt"],
            "channels_serialized": obj["channels_serialized"], "channels_animated": obj["channels_animated"],
            "motion_class": obj["motion_class"], "reference_engine_frame": obj["reference_engine_frame"],
            "max_engine_frame": obj["max_engine_frame"], "rotation_semantics_check": obj["rotation_semantics_check"],
            "rotation_cumulative_max_error": obj["rotation_cumulative_max_error"],
            "parent_index": obj.get("parent_index"),
            "bind_local_identity_error": obj.get("bind_local_identity_error"),
        }
        if include_key_samples and obj["source_key_frames"]:
            frames = obj["source_key_frames"]
            pick = sorted(set([frames[0], frames[len(frames)//2], frames[-1]]))
            row["normalized_samples"] = [normalized_sample(obj, f, obj["reference_engine_frame"]) for f in pick]
        rows.append(row)
    return {
        "format": parsed["format"], "source": parsed["source"], "file_size": parsed["file_size"],
        "object_count": parsed["object_count"], "object_local_object_count": parsed["object_local_object_count"],
        "animated_object_count": parsed["animated_object_count"], "hold_object_count": parsed["hold_object_count"],
        "max_engine_frame": parsed["max_engine_frame"], "objects": rows,
        "root_objects": parsed.get("root_objects", []),
        "parent_edges": parsed.get("parent_edges", []),
        "parent_edge_count": parsed.get("parent_edge_count", 0),
        "unresolved_parent_count": parsed.get("unresolved_parent_count", 0),
        "unresolved_parents": parsed.get("unresolved_parents", []),
        "duplicate_node_name_group_count": parsed.get("duplicate_node_name_group_count", 0),
        "duplicate_node_name_groups": parsed.get("duplicate_node_name_groups", {}),
        "parent_resolution_semantics": parsed.get("parent_resolution_semantics"),
        "animated_child_count": parsed.get("animated_child_count", 0),
        "hierarchy_semantics": "LOCAL_QMAT_TIMES_PARENT_TMRESULT",
        "rotation_semantics": "ACCUMULATED_TM_PREV_ROT",
        "source_rot_key_semantics": "INCREMENTAL_LOCAL_QUATERNION",
        "preview_reference": "FIRST_KEY_NORMALIZED_OBJECT_LOCAL",
    }


def write_compact_report(path: str | pathlib.Path, parsed: dict[str, Any]) -> pathlib.Path:
    path = pathlib.Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(compact_report(parsed), ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
    return path

from __future__ import annotations
import importlib.util, json, os, pathlib, shutil, struct, subprocess
from .utils import norm_path

_MODEL062_HEADER = b"SMD Model data Ver 0.62"
_SMD_HEADER_SIZE = 556
_SMD_INFO_SIZE = 40
_SMD_OBJECT_SIZE = 2236
_SMD_VERTEX_SIZE = 24
_SMD_FACE_SIZE = 36
_O_NV = 84
_O_NF = 88


def _low(path: str) -> str:
    return norm_path(path).lower()


def _used_material_ids_model062(path: pathlib.Path) -> list[int]:
    """Return material ids actually referenced by Model0.62 faces.

    This intentionally ignores serialized-but-unreferenced material slots.
    Failures propagate to the caller so readiness can fall back to the old,
    stricter ALL_SERIALIZED semantics rather than silently weakening a gate.
    """
    data = path.read_bytes()
    if len(data) < _SMD_HEADER_SIZE or not data.startswith(_MODEL062_HEADER):
        raise ValueError("not SMD Model data Ver 0.62")
    obj_count = struct.unpack_from("<i", data, 24)[0]
    if not 0 <= obj_count <= 4096:
        raise ValueError(f"invalid object count: {obj_count}")
    used: set[int] = set()
    for i in range(obj_count):
        info = _SMD_HEADER_SIZE + i * _SMD_INFO_SIZE
        if info + _SMD_INFO_SIZE > len(data):
            raise ValueError("object info table truncated")
        start = struct.unpack_from("<i", data, info + 36)[0]
        if start < 0 or start + _SMD_OBJECT_SIZE > len(data):
            raise ValueError(f"object {i} start out of range: {start}")
        nv = struct.unpack_from("<i", data, start + _O_NV)[0]
        nf = struct.unpack_from("<i", data, start + _O_NF)[0]
        if nv < 0 or nf < 0:
            raise ValueError(f"object {i} has negative counts")
        face_off = start + _SMD_OBJECT_SIZE + nv * _SMD_VERTEX_SIZE
        face_end = face_off + nf * _SMD_FACE_SIZE
        if face_end > len(data):
            raise ValueError(f"object {i} face array truncated")
        for fi in range(nf):
            used.add(struct.unpack_from("<H", data, face_off + fi * _SMD_FACE_SIZE + 6)[0])
    return sorted(used)




def _load_bmp_core(root: pathlib.Path):
    p = root / "pt_character_texture_v1_4.py"
    if not p.is_file():
        raise FileNotFoundError(p)
    spec = importlib.util.spec_from_file_location("pt_texture_overlay_bmp_core", p)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load BMP core: {p}")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod

def _load_material_parser(root: pathlib.Path):
    p = root / "pt_pikeman_smd_material_audit_v1.py"
    if not p.is_file():
        raise FileNotFoundError(p)
    spec = importlib.util.spec_from_file_location("pt_texture_overlay_material_decoder", p)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load material parser: {p}")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def _face_texture_scope(*, asset, resolver, root: pathlib.Path) -> dict:
    """Classify texture references by whether a face-referenced material uses them.

    The inventory contains every serialized material texture. Model0.62 files can
    retain historical materials that no face references. Those references remain
    preserved and are reported, but they must not make a visual texture gate fail.
    """
    fallback = {
        "semantics": "ALL_SERIALIZED_FALLBACK",
        "used_material_ids": [],
        "required_names": [],
        "error": None,
    }
    if str(getattr(asset, "kind", "")) != "SMD_MODEL_062":
        return fallback
    try:
        smd = resolver.resolve(norm_path(asset.primary_path))
        used_ids = _used_material_ids_model062(smd)
        parser = _load_material_parser(root)
        decoded = parser.parse_smd(smd)
        by_id = {int(m["id"]): m for m in decoded.get("materials", [])}
        missing_ids = [mid for mid in used_ids if mid not in by_id]
        if missing_ids:
            raise RuntimeError(f"face material ids absent from material table: {missing_ids}")
        required: list[str] = []
        seen: set[str] = set()
        for mid in used_ids:
            mat = by_id[mid]
            for row in list(mat.get("textures") or []) + list(mat.get("anim_textures") or []):
                name = norm_path(row.get("Name") or "")
                if not name:
                    continue
                k = _low(name)
                if k not in seen:
                    seen.add(k)
                    required.append(name)
        return {
            "semantics": "FACE_REFERENCED_MATERIALS",
            "used_material_ids": used_ids,
            "required_names": required,
            "error": None,
        }
    except Exception as exc:
        fallback["error"] = f"{type(exc).__name__}: {exc}"
        return fallback


def prepare_texture_overlay(*, asset, resolver, root: pathlib.Path, work: pathlib.Path) -> tuple[pathlib.Path, dict]:
    """Create a read-only client texture overlay for Blender import.

    - TGA is decoded through pt_protected_tga_v1.py (standard TGA remains byte-equivalent).
    - Other texture types are symlinked to the real client file.
    - Model0.62 readiness is based on textures of face-referenced materials.
      Serialized textures belonging only to unused/historical material slots are
      preserved as diagnostic entries but cannot alone fail texture_prepare.
    - If face-use classification cannot be proven, behavior fails closed to the
      historical ALL_SERIALIZED readiness semantics.
    - The real client is never modified.
    """
    overlay = work / "texture_overlay_client"
    if overlay.exists():
        shutil.rmtree(overlay)
    overlay.mkdir(parents=True, exist_ok=True)

    scope = _face_texture_scope(asset=asset, resolver=resolver, root=root)
    required_names = list(scope.get("required_names") or [])
    required_keys = {_low(x) for x in required_names}
    face_scoped = scope.get("semantics") == "FACE_REFERENCED_MATERIALS"

    # Required names are derived from the source material table and may include
    # animated texture names that an older inventory did not enumerate. Include
    # them in the overlay rather than trusting inventory completeness.
    logicals: list[str] = []
    seen: set[str] = set()
    for raw in list(getattr(asset, "textures", []) or []) + required_names:
        n = norm_path(raw)
        if not n:
            continue
        k = _low(n)
        if k in seen:
            continue
        seen.add(k)
        logicals.append(n)

    rows = []
    required_failures = 0
    historical_unused_failures = 0
    for logical_n in logicals:
        required = (not face_scoped) or (_low(logical_n) in required_keys)
        dst = overlay / pathlib.PurePosixPath(logical_n)
        dst.parent.mkdir(parents=True, exist_ok=True)
        try:
            src = resolver.resolve(logical_n)
        except Exception as exc:
            if required:
                required_failures += 1
            else:
                historical_unused_failures += 1
            rows.append({
                "logical": logical_n,
                "status": "MISSING",
                "required_by_faces": required,
                "affects_ready": required,
                "error": f"{type(exc).__name__}: {exc}",
            })
            continue

        ext = src.suffix.lower()
        if ext == ".bmp" and src.read_bytes()[:2] != b"BM":
            try:
                bmp = _load_bmp_core(root)
                raw = src.read_bytes()
                info = bmp.inspect_bmp_bytes(raw)
                editable = bmp.decode_editable(raw, info)
                # Strong preview invariant: decoding changes only the 14-byte
                # proprietary file header; DIB/palette/pixel payload stays exact.
                if editable[14:] != raw[14:] or editable[:2] != b"BM":
                    raise RuntimeError("protected BMP preview decode changed non-header bytes")
                dst.write_bytes(editable)
                rows.append({
                    "logical": logical_n, "physical": str(src), "prepared": str(dst),
                    "status": "BMP_PROTECTED_DECODED", "required_by_faces": required,
                    "affects_ready": required, "source_format": info.get("format"),
                    "preview_format": "BMP_STANDARD_EQUIVALENT",
                })
            except Exception as exc:
                if required:
                    required_failures += 1
                else:
                    historical_unused_failures += 1
                rows.append({
                    "logical": logical_n, "physical": str(src),
                    "status": "BMP_PROTECTED_DECODE_FAIL",
                    "required_by_faces": required, "affects_ready": required,
                    "error": f"{type(exc).__name__}: {exc}",
                })
            continue
        if ext == ".tga":
            log = work / "texture_overlay_logs" / (dst.name + ".decode.log")
            log.parent.mkdir(parents=True, exist_ok=True)
            cmd = [
                "python3", str(root / "pt_protected_tga_v1.py"), "decode", str(src), "--out", str(dst)
            ]
            proc = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode != 0 or not dst.is_file():
                if required:
                    required_failures += 1
                else:
                    historical_unused_failures += 1
                rows.append({
                    "logical": logical_n, "physical": str(src), "status": "TGA_DECODE_FAIL",
                    "required_by_faces": required, "affects_ready": required,
                    "log": str(log), "returncode": proc.returncode,
                })
                # Keep the original available for diagnostic import, but do not
                # call the failed decode prepared.
                try:
                    if dst.exists() or dst.is_symlink():
                        dst.unlink()
                    os.symlink(src, dst)
                except Exception:
                    pass
            else:
                rows.append({
                    "logical": logical_n, "physical": str(src), "prepared": str(dst),
                    "status": "TGA_READY", "required_by_faces": required,
                    "affects_ready": required, "log": str(log),
                })
        else:
            try:
                os.symlink(src, dst)
            except FileExistsError:
                pass
            rows.append({
                "logical": logical_n, "physical": str(src), "prepared": str(dst),
                "status": "LINK_READY", "required_by_faces": required,
                "affects_ready": required,
            })

    if face_scoped:
        historical_count = sum(1 for x in logicals if _low(x) not in required_keys)
    else:
        historical_count = 0
    report = {
        "asset_id": asset.asset_id,
        "asset": asset.primary_path,
        "texture_count": len(logicals),
        "serialized_texture_count": len(list(getattr(asset, "textures", []) or [])),
        "usage_semantics": scope.get("semantics"),
        "usage_error": scope.get("error"),
        "used_material_ids": scope.get("used_material_ids") or [],
        "required_texture_count": len(required_keys) if face_scoped else len(logicals),
        "historical_unused_texture_count": historical_count,
        "required_failures": required_failures,
        "historical_unused_failures": historical_unused_failures,
        "ready": required_failures == 0,
        "overlay": str(overlay),
        "entries": rows,
        "client_modified": False,
    }
    rp = work / "texture_overlay_report.json"
    rp.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return overlay, report


def prepare_stage072_texture_overlay(*, asset, parsed: dict, resolver, root: pathlib.Path, work: pathlib.Path) -> tuple[pathlib.Path, dict]:
    """Prepare every serialized texture used by a face-referenced Stage0.72 material.

    Stage0.72 is not covered by the historical inventory's Model0.62 face-scope
    helper, so derive the exact material set from the physically parsed faces.
    The real client remains read-only; this reuses the same protected BMP/TGA
    overlay machinery as Model0.62.
    """
    import types

    used_ids = {
        int(mid) for mid, count in (parsed.get("material_face_counts") or {}).items()
        if int(count or 0) > 0
    }
    names: list[str] = []
    seen: set[str] = set()
    for mat in parsed.get("materials") or []:
        mid = int(mat.get("id") or 0)
        if mid not in used_ids:
            continue
        for row in list(mat.get("textures") or []) + list(mat.get("anim_textures") or []):
            logical = norm_path(row.get("Name") or "")
            if not logical:
                continue
            key = _low(logical)
            if key in seen:
                continue
            seen.add(key)
            names.append(logical)

    proxy = types.SimpleNamespace(
        asset_id=asset.asset_id,
        primary_path=asset.primary_path,
        kind="SMD_STAGE_072",
        textures=names,
    )
    overlay, report = prepare_texture_overlay(
        asset=proxy, resolver=resolver, root=root, work=work
    )
    report["usage_semantics"] = "STAGE072_FACE_REFERENCED_MATERIALS"
    report["used_material_ids"] = sorted(used_ids)
    report["required_texture_count"] = len(names)
    report["serialized_texture_count"] = len(names)
    report["historical_unused_texture_count"] = 0
    rp = work / "texture_overlay_report.json"
    rp.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return overlay, report

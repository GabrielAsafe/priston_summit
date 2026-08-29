from __future__ import annotations

import collections
import json
import pathlib
import zipfile

from .stage072_corpus import parse_stage072
from .utils import norm_path


def _candidate_rows(records: list[dict]) -> list[dict]:
    rows = []
    seen = set()
    for r in records:
        if str(r.get("kind") or "") != "SMD_STAGE_072":
            continue
        rel = norm_path(r.get("relative_path") or r.get("path") or "")
        if not rel:
            continue
        key = rel.lower()
        if key in seen:
            continue
        seen.add(key)
        rows.append({"relative_path": rel, "record": r})
    rows.sort(key=lambda x: x["relative_path"].lower())
    return rows


def _used_material_texture_refs(parsed: dict) -> list[dict]:
    """Return serialized textures belonging to face-referenced materials.

    This deliberately mirrors the V0.6.0 Stage0.72 overlay scope. It is a
    material-use gate, not a filename inventory gate: historical materials that
    no face references are excluded. Within a used material, every serialized
    base/animated texture name remains preserved and checked.
    """
    used_ids = {
        int(mid) for mid, count in (parsed.get("material_face_counts") or {}).items()
        if int(count or 0) > 0
    }
    refs = []
    for mat in parsed.get("materials") or []:
        mid = int(mat.get("id") or 0)
        if mid not in used_ids:
            continue
        face_uses = int((parsed.get("material_face_counts") or {}).get(str(mid)) or 0)
        for kind, rows in (
            ("BASE", list(mat.get("textures") or [])),
            ("ANIM", list(mat.get("anim_textures") or [])),
        ):
            for slot, row in enumerate(rows):
                logical = norm_path(row.get("Name") or "")
                if not logical:
                    continue
                refs.append({
                    "material_id": mid,
                    "face_uses": face_uses,
                    "kind": kind,
                    "slot": slot,
                    "logical": logical,
                })
    return refs


def audit_stage072_texture_presence(records: list[dict], resolver, *, bundle: str | None = None) -> dict:
    candidates = _candidate_rows(records)
    rows = []
    scan_errors = []
    unique_refs: dict[str, dict] = {}

    # Build once so absence/collision semantics are exactly the same as normal
    # client resolution, including exact-case preference and collision guards.
    _ = resolver.index

    for item in candidates:
        rel = item["relative_path"]
        try:
            source = resolver.resolve(rel)
            parsed = parse_stage072(source)
            if parsed.get("full_parse") != "PASS":
                raise RuntimeError("Stage0.72 source parser did not pass")
            refs = _used_material_texture_refs(parsed)
            missing = []
            present = 0
            for ref in refs:
                logical = ref["logical"]
                key = logical.lower()
                agg = unique_refs.setdefault(key, {
                    "logical": logical,
                    "present": False,
                    "files": set(),
                    "materials": set(),
                    "slots": set(),
                    "face_uses_sum": 0,
                    "resolved_paths": set(),
                })
                agg["files"].add(rel)
                agg["materials"].add((rel, int(ref["material_id"])))
                agg["slots"].add((ref["kind"], int(ref["slot"])))
                agg["face_uses_sum"] += int(ref["face_uses"])
                try:
                    physical = resolver.resolve(logical)
                except FileNotFoundError:
                    missing.append(ref)
                except Exception as exc:
                    # Case collisions and other resolver failures are not the
                    # same as an absent shipped file; keep them hard-fail.
                    raise RuntimeError(f"texture resolver error for {logical}: {type(exc).__name__}: {exc}") from exc
                else:
                    present += 1
                    agg["present"] = True
                    agg["resolved_paths"].add(str(physical))
            rows.append({
                "relative_path": rel,
                "texture_reference_occurrences": len(refs),
                "present_reference_occurrences": present,
                "missing_reference_occurrences": len(missing),
                "missing_references": missing,
            })
        except Exception as exc:
            scan_errors.append({"relative_path": rel, "error": f"{type(exc).__name__}: {exc}"})
            rows.append({"relative_path": rel, "scan": "FAIL", "error": f"{type(exc).__name__}: {exc}"})

    unique_rows = []
    for _key, agg in sorted(unique_refs.items()):
        unique_rows.append({
            "logical": agg["logical"],
            "present": bool(agg["present"]),
            "files": sorted(agg["files"], key=str.lower),
            "materials": [
                {"file": f, "material_id": mid}
                for f, mid in sorted(agg["materials"], key=lambda x: (x[0].lower(), x[1]))
            ],
            "slots": [
                {"kind": kind, "slot": slot}
                for kind, slot in sorted(agg["slots"], key=lambda x: (x[0], x[1]))
            ],
            "face_uses_sum": int(agg["face_uses_sum"]),
            "resolved_paths": sorted(agg["resolved_paths"]),
        })

    missing_unique = [x for x in unique_rows if not x["present"]]
    files_with_missing = [r for r in rows if int(r.get("missing_reference_occurrences") or 0) > 0]
    expected = len(candidates)
    audited = len(rows)
    status = "PASS" if expected > 0 and audited == expected and not scan_errors else "FAIL"

    bundle_path = None
    if bundle:
        bp = pathlib.Path(bundle).expanduser()
        if not bp.is_absolute():
            bp = pathlib.Path.cwd() / bp
        bp = bp.resolve()
        bp.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(bp, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
            for row in files_with_missing:
                rel = row["relative_path"]
                physical = resolver.resolve(rel, required=False)
                if physical is not None:
                    zf.write(physical, arcname=f"client/{rel}")
            zf.writestr(
                "STAGE072_MISSING_TEXTURE_REFERENCES.json",
                json.dumps(missing_unique, ensure_ascii=False, indent=2) + "\n",
            )
        bundle_path = str(bp)

    return {
        "format": "PT_STAGE072_TEXTURE_PRESENCE_AUDIT_V1",
        "status": status,
        "candidate_files_expected": expected,
        "physical_files_audited": audited,
        "scan_errors": scan_errors,
        "face_material_scoped_texture_reference_occurrences": sum(int(r.get("texture_reference_occurrences") or 0) for r in rows),
        "unique_face_material_scoped_texture_references": len(unique_rows),
        "present_unique_texture_references": sum(bool(x["present"]) for x in unique_rows),
        "missing_unique_texture_references": len(missing_unique),
        "files_with_missing_client_textures": len(files_with_missing),
        "missing_references": missing_unique,
        "texture_presence_complete_candidate": bool(status == "PASS" and expected == 63 and not missing_unique),
        "transport_policy": {
            "missing_client_file": "preserve logical texture reference and allow structural Blender transport; preview material remains incomplete",
            "decode_or_load_error_for_existing_file": "fail closed",
            "substitution_by_filename_guess": "forbidden",
            "client_modified": False,
        },
        "sourcecode_runtime_note": "material loading keeps serialized texture handles; texture creation can return -1 without aborting material-group load, so a missing shipped image is a client/reference condition rather than a Stage0.72 structural parse failure",
        "bundle": bundle_path,
        "rows": rows,
    }

#!/usr/bin/env python3
from __future__ import annotations
import argparse, csv, hashlib, importlib.util, json, re, shutil, struct
from collections import defaultdict
from pathlib import Path

TOOL = "PT_AUTO_PROFILE_PRODUCTION_V1"
HEADER = b"SMD Model data Ver 0.62"
SZ_HDR, SZ_INFO, SZ_OBJ = 556, 40, 2236
O_PHYS, O_NV, O_NF, O_NT = 16, 84, 88, 92
O_NAME, O_NR, O_NP, O_NS = 172, 684, 688, 692

def sha256_file(p: Path):
    h = hashlib.sha256()
    with p.open("rb") as f:
        for b in iter(lambda: f.read(1024 * 1024), b""):
            h.update(b)
    return h.hexdigest()

def cstr(b):
    return b.split(b"\0", 1)[0].decode("latin1", "replace")

def norm(s):
    return str(s or "").replace("\\", "/").lstrip("./")

def low(s):
    return norm(s).lower()

def slug(s):
    return re.sub(r"[^a-z0-9]+", "_", str(s).lower()).strip("_") or "asset"

def truth(v):
    return str(v).lower() in ("1", "true", "yes")

def read_csv(p: Path):
    with p.open("r", encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))

def parse_inx(p: Path):
    b = p.read_bytes()
    if len(b) < 0x18C:
        raise RuntimeError("INX too short")
    groups = {}
    for lod, off in (("high", 0xC0), ("default", 0x104), ("low", 0x148)):
        n = struct.unpack_from("<i", b, off)[0]
        if not 0 <= n <= 4:
            raise RuntimeError(f"{lod} group count={n} invalid")
        groups[lod] = [
            cstr(b[off + 4 + i * 16 : off + 4 + (i + 1) * 16])
            for i in range(n)
        ]
    return {
        "model_ref": cstr(b[:0x40]),
        "motion_ref": cstr(b[0x40:0x80]),
        "submodel_ref": cstr(b[0x80:0xC0]),
        "groups": groups,
    }

def parse_smd_objects(p: Path):
    b = p.read_bytes()
    if len(b) < SZ_HDR or not b.startswith(HEADER):
        raise RuntimeError("not SMD Model data Ver 0.62")
    n, mat_count = struct.unpack_from("<ii", b, 24)
    if not 1 <= n <= 128:
        raise RuntimeError(f"object count={n} outside runtime range")
    rows = []
    for i in range(n):
        off = SZ_HDR + i * SZ_INFO
        name = cstr(b[off : off + 32])
        length, start = struct.unpack_from("<ii", b, off + 32)
        if start < 0 or length <= 0 or start + length > len(b):
            raise RuntimeError(f"{name}: invalid object block")
        raw = b[start : start + SZ_OBJ]
        node = cstr(raw[O_NAME : O_NAME + 32])
        if node != name:
            raise RuntimeError(f"{name}: NodeName mismatch {node!r}")
        nv, nf, nt = struct.unpack_from("<iii", raw, O_NV)
        nr, np, ns = struct.unpack_from("<iii", raw, O_NR)
        rows.append({
            "name": name,
            "vertices": nv,
            "faces": nf,
            "texlinks": nt,
            "physique": struct.unpack_from("<I", raw, O_PHYS)[0] != 0,
            "rot_keys": nr,
            "pos_keys": np,
            "scale_keys": ns,
        })
    return {"object_count": n, "material_count": mat_count, "objects": rows}

def load_material_parser(root: Path):
    p = root / "pt_pikeman_smd_material_audit_v1.py"
    if not p.is_file():
        raise FileNotFoundError(p)
    spec = importlib.util.spec_from_file_location("pt_generic_material_decoder", p)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)
    return mod

def exact_path(record, client: Path, rel: str):
    if record:
        p = Path(str(record.get("path", "")))
        if p.is_file():
            return p
    return client / rel

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--client-root", required=True)
    ap.add_argument("--inventory", required=True)
    ap.add_argument("--inx-pairing", required=True)
    sel = ap.add_mutually_exclusive_group(required=True)
    sel.add_argument("--inx")
    sel.add_argument("--model")
    ap.add_argument("--profile-id")
    ap.add_argument("--owner-name")
    ap.add_argument("--class-code", default="")
    ap.add_argument("--out-profile", required=True)
    ap.add_argument("--source-dir")
    a = ap.parse_args()

    root = Path(a.root).expanduser().resolve()
    client = Path(a.client_root).expanduser().resolve()
    inventory = json.loads(
        Path(a.inventory).expanduser().resolve().read_text(encoding="utf-8")
    )
    records = {
        low(r["relative_path"]): r
        for r in inventory["records"]
        if str(r.get("path", "")).startswith(str(client))
    }
    pairs = read_csv(Path(a.inx_pairing).expanduser().resolve())

    hits = []
    for row in pairs:
        if not truth(row.get("model_resolved")):
            continue
        if a.inx and low(row.get("inx")) == low(a.inx):
            hits.append(row)
        elif a.model and low(row.get("model_smd")) == low(a.model):
            hits.append(row)
    if not hits:
        raise RuntimeError("selection did not resolve to an INX/model pair")

    # V1 intentionally writes one profile per INX/config.
    # A model with multiple configs must be selected explicitly by --inx.
    if a.inx:
        if len(hits) != 1:
            raise RuntimeError(f"selected INX produced {len(hits)} rows")
        row = hits[0]
    else:
        configs = {low(x.get("inx")) for x in hits}
        if len(configs) != 1:
            raise RuntimeError(
                f"model has {len(configs)} INX configs; select --inx explicitly"
            )
        row = hits[0]

    model_rel = norm(row["model_smd"])
    smb_rel = norm(row.get("motion_smb"))
    inx_rel = norm(row["inx"])
    if not smb_rel:
        raise RuntimeError(
            "V1 production generator requires a self-contained INX with one "
            "resolved SMB. Config-only assets must not guess a skeleton."
        )

    mr, sr, ir = (
        records.get(low(model_rel)),
        records.get(low(smb_rel)),
        records.get(low(inx_rel)),
    )
    if not all((mr, sr, ir)):
        raise RuntimeError("inventory is missing selected model/skeleton/INX records")

    model = exact_path(mr, client, model_rel)
    smb = exact_path(sr, client, smb_rel)
    inx = exact_path(ir, client, inx_rel)
    for p in (model, smb, inx):
        if not p.is_file():
            raise FileNotFoundError(p)

    ii = parse_inx(inx)
    sm = parse_smd_objects(model)
    names = {x["name"] for x in sm["objects"]}

    unresolved = {
        lod: [x for x in vals if x and x not in names]
        for lod, vals in ii["groups"].items()
    }

    mat_decoder = load_material_parser(root)
    materials = mat_decoder.parse_smd(model)
    texslots = defaultdict(set)
    for m in materials["materials"]:
        for t in m["textures"] + m["anim_textures"]:
            if t.get("Name"):
                texslots[norm(t["Name"])].add(m["id"])

    owner = a.owner_name or Path(inx_rel).stem
    pid = a.profile_id or "auto_" + slug(Path(inx_rel).with_suffix("").as_posix())

    source_dir = Path(a.source_dir).expanduser().resolve() if a.source_dir else None
    if source_dir:
        source_dir.mkdir(parents=True, exist_ok=True)
        for p in (model, smb, inx):
            dst = source_dir / p.name
            if dst.exists() and sha256_file(dst) != sha256_file(p):
                raise RuntimeError(f"source collision: {dst}")
            if not dst.exists():
                shutil.copy2(p, dst)

    components = [
        {
            "role": "model",
            "required": True,
            "editable": True,
            "local_source": model.name,
            "roundtrip_target": model_rel,
            "workspace_path": f"Characters/{owner}/Model/{model.name}",
            "category": "Characters",
            "subcategory": "Character Model",
            "representation": "SMD Model0.62",
            "verified_client_size": model.stat().st_size,
            "verified_client_sha256": sha256_file(model),
        },
        {
            "role": "skeleton",
            "required": True,
            "editable": False,
            "local_source": smb.name,
            "roundtrip_target": smb_rel,
            "workspace_path": f"Characters/{owner}/Skeleton/{smb.name}",
            "category": "Characters",
            "subcategory": "Character Skeleton",
            "representation": "SMB",
            "verified_client_size": smb.stat().st_size,
            "verified_client_sha256": sha256_file(smb),
        },
        {
            "role": "model_info",
            "required": True,
            "editable": False,
            "local_source": inx.name,
            "roundtrip_target": inx_rel,
            "workspace_path": f"Characters/{owner}/Config/{inx.name}",
            "category": "Characters",
            "subcategory": "Character Model Info",
            "representation": "INX",
            "verified_client_size": inx.stat().st_size,
            "verified_client_sha256": sha256_file(inx),
        },
    ]

    for idx, (tex, slots) in enumerate(sorted(texslots.items())):
        role = "texture" if idx == 0 else f"texture_{idx:02d}"
        tr = records.get(low(tex))
        tp = exact_path(tr, client, tex)
        comp = {
            "role": role,
            "required": True,
            "editable": True,
            "roundtrip_target": tex,
            "workspace_path": f"Characters/{owner}/Textures/{Path(tex).name}",
            "category": "Textures",
            "subcategory": "Character Texture",
            "representation": "Serialized SMD Texture",
            "material_slots": sorted(slots),
        }
        if tp.is_file():
            comp["verified_client_size"] = tp.stat().st_size
            comp["verified_client_sha256"] = sha256_file(tp)
        components.append(comp)

    all_phys = all(x["physique"] for x in sm["objects"])
    no_phys = all(not x["physique"] for x in sm["objects"])
    physique_mode = "RIGID" if all_phys else ("STATIC" if no_phys else "MIXED")

    profile = {
        "profile_version": 4,
        "profile_id": pid,
        "asset_set_id": "AUTOSET-" + hashlib.sha1(
            (model_rel + "|" + inx_rel).encode("utf-8")
        ).hexdigest()[:12].upper(),
        "display_name": f"{owner} / {Path(inx_rel).stem}",
        "class_code": a.class_code,
        "owner_name": owner,
        "description": (
            "Auto-generated production-schema profile from canonical physical "
            "INX→SMD→SMB chain. Structural candidate only; runtime certification "
            "is not implied."
        ),
        "model_smd": model.name,
        "model_smd_sha256": sha256_file(model),
        "skeleton_smb": smb.name,
        "skeleton_smb_sha256": sha256_file(smb),
        "model_info_inx": inx.name,
        "model_info_inx_sha256": sha256_file(inx),
        "roundtrip_target": model_rel,
        "runtime_object_limit": 128,
        "runtime_vertex_limit_per_object": 30000,
        "runtime_face_limit_per_object": 30000,
        "lod_groups": ii["groups"],
        "known_mesh_objects": [x["name"] for x in sm["objects"]],
        "baseline_counts": {
            x["name"]: {"vertices": x["vertices"], "faces": x["faces"]}
            for x in sm["objects"]
        },
        "components": components,
        "geometry_status": {
            "single_object_runtime_proven": False,
            "high_lod_runtime_proven": False,
            "all_lods_runtime_proven": False,
            "artist_free_topology_runtime_proven": False,
            "production_force_test_allowed": False,
            "status_note": (
                "AUTO PROFILE: structural candidate; representative runtime "
                "certification pending"
            ),
        },
        "identity_evidence": {
            "status": "AUTO_PHYSICAL_CHAIN_STRUCTURALLY_RESOLVED",
            "inx": inx_rel,
            "szModelFile": ii["model_ref"],
            "szMotionFile": ii["motion_ref"],
            "szSubModelFile": ii["submodel_ref"],
            "model_smd_sha256": sha256_file(model),
            "skeleton_smb_sha256": sha256_file(smb),
            "model_info_inx_sha256": sha256_file(inx),
            "legacy_unresolved_lod_nodes": unresolved,
        },
        "auto_profile": {
            "generator": TOOL,
            "production_schema_compatible_intent": True,
            "self_contained_motion": True,
            "runtime_certified": False,
            "physique_mode": physique_mode,
            "object_local_animation_present": any(
                x["rot_keys"] or x["pos_keys"] or x["scale_keys"]
                for x in sm["objects"]
            ),
        },
    }

    out = Path(a.out_profile).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(
        json.dumps(profile, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print(TOOL)
    print(f"profile_id={pid}")
    print(f"model={model_rel}")
    print(f"skeleton={smb_rel}")
    print(f"inx={inx_rel}")
    print(f"objects={sm['object_count']}")
    print(f"materials={sm['material_count']}")
    print(f"texture_components={len(texslots)}")
    print(f"physique_mode={physique_mode}")
    print(f"profile={out}")
    print("runtime_certified=False")
    print("client_modified=False")
    print("auto_profile_production_check=PASS")

if __name__ == "__main__":
    main()
